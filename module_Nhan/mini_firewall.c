#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <net/net_namespace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced System Programmer");
MODULE_DESCRIPTION("A mini kernel firewall that intercepts and drops ICMP (Ping) packets");
MODULE_VERSION("1.0");

/* Cau truc chua thong tin ve diem neo (hook) cua chung ta */
static struct nf_hook_ops netfilter_ops;

/* Ham xu ly chinh: Duoc goi moi khi co mot goi tin di qua Kernel */
unsigned int block_icmp_hook(void *priv, struct sk_buff *skb, const struct nf_hook_state *state) {
    struct iphdr *ip_header;

    /* Kiem tra tinh hop le cua socket buffer (sk_buff) */
    if (!skb) {
        return NF_ACCEPT;
    }

    /* Trich xuat Header cua giao thuc IP */
    ip_header = ip_hdr(skb);
    if (!ip_header) {
        return NF_ACCEPT;
    }

    /* Neu goi tin dung giao thuc ICMP (Ping) -> Tieu diet (Drop) */
    if (ip_header->protocol == IPPROTO_ICMP) {
        pr_info("[MINI-FIREWALL] Phat hien goi tin ICMP (Ping) tu IP: %pI4. Da huy (DROP)!\n", &ip_header->saddr);
        return NF_DROP;
    }

    /* Cac loai goi tin khac (TCP, UDP...) duoc phep di qua */
    return NF_ACCEPT;
}

static int __init firewall_init(void) {
    pr_info("[MINI-FIREWALL] Dang nap mo-dun vao he thong...\n");

    /* Cau hinh thong so cho Hook */
    netfilter_ops.hook = block_icmp_hook;                 /* Ham xu ly goi tin */
    netfilter_ops.hooknum = NF_INET_PRE_ROUTING;          /* Chan ngay tai cuu vao (Incoming) */
    netfilter_ops.pf = PF_INET;                           /* Ap dung cho IPv4 */
    netfilter_ops.priority = NF_IP_PRI_FIRST;             /* Uu tien cao nhat */

    /* Dang ky hook voi Kernel */
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
        nf_register_net_hook(&init_net, &netfilter_ops);
    #else
        nf_register_hook(&netfilter_ops);
    #endif

    pr_info("[MINI-FIREWALL] Kich hoat thanh cong! He thong hien tai dang tàng hinh truoc Ping.\n");
    return 0;
}

static void __exit firewall_exit(void) {
    pr_info("[MINI-FIREWALL] Dang go bo mo-dun...\n");

    /* Huy dang ky hook */
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
        nf_unregister_net_hook(&init_net, &netfilter_ops);
    #else
        nf_unregister_hook(&netfilter_ops);
    #endif

    pr_info("[MINI-FIREWALL] Da go bo. He thong nhan Ping binh thuong.\n");
}

module_init(firewall_init);
module_exit(firewall_exit);
