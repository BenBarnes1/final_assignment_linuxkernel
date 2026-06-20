#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <net/tcp.h>
#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/wait.h>

#define TARGET_PORT 5001
#define BUF_SIZE 2048

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced System Programmer");
MODULE_DESCRIPTION("Flawless Ring-Buffer Network Steganography with Logging");

static char srv_tx_buf[BUF_SIZE], srv_rx_buf[BUF_SIZE];
static int srv_tx_head = 0, srv_tx_tail = 0, srv_rx_head = 0, srv_rx_tail = 0;
static DEFINE_SPINLOCK(srv_tx_lock);
static DEFINE_SPINLOCK(srv_rx_lock);
static DECLARE_WAIT_QUEUE_HEAD(srv_rx_wait);

static char cli_tx_buf[BUF_SIZE], cli_rx_buf[BUF_SIZE];
static int cli_tx_head = 0, cli_tx_tail = 0, cli_rx_head = 0, cli_rx_tail = 0;
static DEFINE_SPINLOCK(cli_tx_lock);
static DEFINE_SPINLOCK(cli_rx_lock);
static DECLARE_WAIT_QUEUE_HEAD(cli_rx_wait);

static struct nf_hook_ops nfho_out, nfho_in;

static ssize_t srv_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char tmp[256];
    int i;
    if (len > 255)
        len = 255;
    if (copy_from_user(tmp, buf, len))
        return -EFAULT;
    spin_lock_bh(&srv_tx_lock);
    for (i = 0; i < len; i++)
    {
        srv_tx_buf[srv_tx_head] = tmp[i];
        srv_tx_head = (srv_tx_head + 1) % BUF_SIZE;
    }
    spin_unlock_bh(&srv_tx_lock);
    return len;
}

static ssize_t srv_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    int bytes = 0;
    char tmp[256];
    if (srv_rx_head == srv_rx_tail)
    {
        if (f->f_flags & O_NONBLOCK)
            return -EAGAIN;
        if (wait_event_interruptible(srv_rx_wait, srv_rx_head != srv_rx_tail))
            return -ERESTARTSYS;
    }
    spin_lock_bh(&srv_rx_lock);
    while (srv_rx_tail != srv_rx_head && bytes < len && bytes < 255)
    {
        tmp[bytes++] = srv_rx_buf[srv_rx_tail];
        srv_rx_tail = (srv_rx_tail + 1) % BUF_SIZE;
    }
    spin_unlock_bh(&srv_rx_lock);
    if (bytes > 0 && copy_to_user(buf, tmp, bytes))
        return -EFAULT;
    return bytes;
}

static ssize_t cli_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
    char tmp[256];
    int i;
    if (len > 255)
        len = 255;
    if (copy_from_user(tmp, buf, len))
        return -EFAULT;
    spin_lock_bh(&cli_tx_lock);
    for (i = 0; i < len; i++)
    {
        cli_tx_buf[cli_tx_head] = tmp[i];
        cli_tx_head = (cli_tx_head + 1) % BUF_SIZE;
    }
    spin_unlock_bh(&cli_tx_lock);
    return len;
}

static ssize_t cli_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
    int bytes = 0;
    char tmp[256];
    if (cli_rx_head == cli_rx_tail)
    {
        if (f->f_flags & O_NONBLOCK)
            return -EAGAIN;
        if (wait_event_interruptible(cli_rx_wait, cli_rx_head != cli_rx_tail))
            return -ERESTARTSYS;
    }
    spin_lock_bh(&cli_rx_lock);
    while (cli_rx_tail != cli_rx_head && bytes < len && bytes < 255)
    {
        tmp[bytes++] = cli_rx_buf[cli_rx_tail];
        cli_rx_tail = (cli_rx_tail + 1) % BUF_SIZE;
    }
    spin_unlock_bh(&cli_rx_lock);
    if (bytes > 0 && copy_to_user(buf, tmp, bytes))
        return -EFAULT;
    return bytes;
}

static const struct file_operations srv_fops = {.read = srv_read, .write = srv_write};
static const struct file_operations cli_fops = {.read = cli_read, .write = cli_write};
static struct miscdevice srv_dev = {.minor = MISC_DYNAMIC_MINOR, .name = "stego_server", .fops = &srv_fops, .mode = 0666};
static struct miscdevice cli_dev = {.minor = MISC_DYNAMIC_MINOR, .name = "stego_client", .fops = &cli_fops, .mode = 0666};

/* --- HOOK GUI DI (Inject) --- */
static unsigned int hook_func_out(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    int tcplen;
    u16 sport, dport;
    char secret_char = 0;
    int has_data = 0;

    if (!skb)
        return NF_ACCEPT;
    iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_TCP)
        return NF_ACCEPT;
    tcph = tcp_hdr(skb);
    if (!tcph || tcph->syn || tcph->fin || tcph->rst)
        return NF_ACCEPT;

    sport = ntohs(tcph->source);
    dport = ntohs(tcph->dest);

    if (sport == TARGET_PORT)
    {
        spin_lock_bh(&srv_tx_lock);
        has_data = (srv_tx_head != srv_tx_tail);
        spin_unlock_bh(&srv_tx_lock);
        if (!has_data)
            return NF_ACCEPT;

        if (skb_is_nonlinear(skb) && skb_linearize(skb) != 0)
            return NF_ACCEPT;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        if (skb_try_make_writable(skb, skb->len))
            return NF_ACCEPT;
#else
        if (skb_make_writable(skb, skb->len))
            return NF_ACCEPT;
#endif

        spin_lock_bh(&srv_tx_lock);
        if (srv_tx_head != srv_tx_tail)
        {
            secret_char = srv_tx_buf[srv_tx_tail];
            srv_tx_tail = (srv_tx_tail + 1) % BUF_SIZE;
        }
        else
        {
            has_data = 0;
        }
        spin_unlock_bh(&srv_tx_lock);
    }
    else if (dport == TARGET_PORT)
    {
        spin_lock_bh(&cli_tx_lock);
        has_data = (cli_tx_head != cli_tx_tail);
        spin_unlock_bh(&cli_tx_lock);
        if (!has_data)
            return NF_ACCEPT;

        if (skb_is_nonlinear(skb) && skb_linearize(skb) != 0)
            return NF_ACCEPT;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        if (skb_try_make_writable(skb, skb->len))
            return NF_ACCEPT;
#else
        if (skb_make_writable(skb, skb->len))
            return NF_ACCEPT;
#endif

        spin_lock_bh(&cli_tx_lock);
        if (cli_tx_head != cli_tx_tail)
        {
            secret_char = cli_tx_buf[cli_tx_tail];
            cli_tx_tail = (cli_tx_tail + 1) % BUF_SIZE;
        }
        else
        {
            has_data = 0;
        }
        spin_unlock_bh(&cli_tx_lock);
    }

    if (has_data)
    {
        iph = ip_hdr(skb);
        tcph = tcp_hdr(skb);
        tcph->urg = 1;
        tcph->urg_ptr = htons((uint16_t)secret_char);
        tcplen = ntohs(iph->tot_len) - (iph->ihl * 4);
        tcph->check = 0;
        tcph->check = csum_tcpudp_magic(iph->saddr, iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)tcph, tcplen, 0));
        skb->ip_summed = CHECKSUM_NONE;

        /* IN RA LOG KERNEL LÀM BẰNG CHỨNG GIẤU TIN */
        if (secret_char == '\n')
        {
            pr_info("[Stego_TCP] [HOOK OUT] Da giau ky tu: <Xuong Dong> vao TCP URG_PTR\n");
        }
        else
        {
            pr_info("[Stego_TCP] [HOOK OUT] Da giau ky tu: '%c' vao TCP URG_PTR\n", secret_char);
        }
    }
    return NF_ACCEPT;
}

/* --- HOOK NHAN VAO (Extract) --- */
static unsigned int hook_func_in(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *iph;
    struct tcphdr *tcph;
    u16 sport, dport;
    uint16_t extracted;
    int nxt;
    if (!skb)
        return NF_ACCEPT;
    iph = ip_hdr(skb);
    if (!iph || iph->protocol != IPPROTO_TCP)
        return NF_ACCEPT;
    tcph = tcp_hdr(skb);
    if (!tcph || tcph->syn || tcph->fin || tcph->rst || !tcph->urg)
        return NF_ACCEPT;

    sport = ntohs(tcph->source);
    dport = ntohs(tcph->dest);
    extracted = ntohs(tcph->urg_ptr);
    if (!((extracted >= 32 && extracted <= 126) || extracted == '\n'))
        return NF_ACCEPT;

    /* IN RA LOG KERNEL LÀM BẰNG CHỨNG BÓC TIN */
    if (extracted == '\n')
    {
        pr_info("[Stego_TCP] [HOOK IN] Da boc tach ky tu: <Xuong Dong> tu TCP URG_PTR\n");
    }
    else
    {
        pr_info("[Stego_TCP] [HOOK IN] Da boc tach ky tu: '%c' tu TCP URG_PTR\n", (char)extracted);
    }

    if (dport == TARGET_PORT)
    {
        spin_lock_bh(&srv_rx_lock);
        nxt = (srv_rx_head + 1) % BUF_SIZE;
        if (nxt != srv_rx_tail)
        {
            srv_rx_buf[srv_rx_head] = (char)extracted;
            srv_rx_head = nxt;
        }
        spin_unlock_bh(&srv_rx_lock);
        wake_up_interruptible(&srv_rx_wait);
    }
    else if (sport == TARGET_PORT)
    {
        spin_lock_bh(&cli_rx_lock);
        nxt = (cli_rx_head + 1) % BUF_SIZE;
        if (nxt != cli_rx_tail)
        {
            cli_rx_buf[cli_rx_head] = (char)extracted;
            cli_rx_head = nxt;
        }
        spin_unlock_bh(&cli_rx_lock);
        wake_up_interruptible(&cli_rx_wait);
    }
    return NF_ACCEPT;
}

static int __init stego_init(void)
{
    misc_register(&srv_dev);
    misc_register(&cli_dev);
    nfho_out.hook = hook_func_out;
    nfho_out.hooknum = NF_INET_POST_ROUTING;
    nfho_out.pf = PF_INET;
    nfho_out.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho_out);
    nfho_in.hook = hook_func_in;
    nfho_in.hooknum = NF_INET_PRE_ROUTING;
    nfho_in.pf = PF_INET;
    nfho_in.priority = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &nfho_in);
    pr_info("[Stego_TCP] Da nap Module. San sang ghi nhan log giao tiep.\n");
    return 0;
}

static void __exit stego_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho_out);
    nf_unregister_net_hook(&init_net, &nfho_in);
    misc_deregister(&srv_dev);
    misc_deregister(&cli_dev);
}
module_init(stego_init);
module_exit(stego_exit);
