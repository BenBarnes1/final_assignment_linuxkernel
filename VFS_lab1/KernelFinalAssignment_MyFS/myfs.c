#include <linux/version.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/uaccess.h> 

#define MYFS_MAGIC 0x4D594653 /* 'MYFS' theo yeu cau file Word */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced System Programmer");
MODULE_DESCRIPTION("Virtual File System (myfs) - Hybrid Version");

static const char hello_content[] = "Hello from myfs kernel module!\n";
static const size_t hello_size = sizeof(hello_content) - 1;

static const struct file_operations myfs_dir_ops;
static const struct inode_operations myfs_dir_inode_ops;
static const struct file_operations myfs_file_ops;

/* =========================================================================
 * TODO 3 (Kernel Lab): Khởi tạo Inode
 * ========================================================================= */
static struct inode *myfs_make_inode(struct super_block *sb, int mode, const struct file_operations *fops) {
    struct inode *ret = new_inode(sb);
    if (ret) {
        ret->i_ino = get_next_ino();
        ret->i_mode = mode;
        
        /* Gán quyền truy cập thủ công để tương thích tốt với Ubuntu 22.04/24.04 */
        ret->i_uid.val = 0;
        ret->i_gid.val = 0;
        ret->i_blocks = 0;
        
        /* Initialize i_atime, i_ctime, and i_mtime (Kernel Lab TODO 3) */
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
        inode_set_atime_to_ts(ret, current_time(ret));
        inode_set_mtime_to_ts(ret, current_time(ret));
        inode_set_ctime_to_ts(ret, current_time(ret));
#else
        ret->i_atime = ret->i_mtime = ret->i_ctime = current_time(ret);
#endif
        
        if (fops) {
            ret->i_fop = fops;
        }
    }
    return ret;
}

/* =========================================================================
 * File Word: File Operations (Xu ly lenh cat / doc file)
 * ========================================================================= */
static ssize_t myfs_read(struct file *filp, char __user *buf, size_t count, loff_t *ppos) {
    if (*ppos >= hello_size) return 0;
    if (*ppos + count > hello_size) {
        count = hello_size - *ppos;
    }
    if (copy_to_user(buf, hello_content + *ppos, count)) {
        return -EFAULT;
    }
    *ppos += count;
    return count;
}

static const struct file_operations myfs_file_ops = {
    .read = myfs_read,
};

/* =========================================================================
 * File Word: Directory Operations (Xu ly lenh ls va tra cuu file)
 * ========================================================================= */
static struct dentry *myfs_lookup(struct inode *dir, struct dentry *child, unsigned int flags) {
    if (strcmp(child->d_name.name, "hello.txt") == 0) {
        struct inode *inode = myfs_make_inode(dir->i_sb, S_IFREG | 0444, &myfs_file_ops);
        if (!inode) return ERR_PTR(-ENOMEM);
        
        inode->i_size = hello_size;
        d_add(child, inode);
        return NULL;
    }
    return NULL;
}

static int myfs_readdir(struct file *file, struct dir_context *ctx) {
    if (!dir_emit_dots(file, ctx)) return 0;
    if (ctx->pos == 2) {
        if (!dir_emit(ctx, "hello.txt", 9, get_next_ino(), DT_REG)) return 0;
        ctx->pos++;
    }
    return 0;
}

static const struct inode_operations myfs_dir_inode_ops = {
    .lookup = myfs_lookup,
};

static const struct file_operations myfs_dir_ops = {
    .iterate_shared = myfs_readdir,
};

/* =========================================================================
 * TODO 2 (Kernel Lab): Cấu trúc super_operations
 * ========================================================================= */
static const struct super_operations myfs_ops = {
    .drop_inode = generic_delete_inode,
    .statfs     = simple_statfs,
};

/* =========================================================================
 * TODO 2 & File Word: Khởi tạo Superblock
 * ========================================================================= */
static int myfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct inode *root_inode;

    sb->s_magic = MYFS_MAGIC;
    sb->s_blocksize = 4096;
    
    /* Gán s_op bằng cấu trúc tự định nghĩa theo chuẩn Kernel Lab */
    sb->s_op = &myfs_ops;

    /* Tạo root inode duoi dang Directory voi quyen 0755 */
    root_inode = myfs_make_inode(sb, S_IFDIR | 0755, &myfs_dir_ops);
    if (!root_inode) {
        pr_err("myfs: Khong the cap phat root inode.\n");
        return -ENOMEM;
    }
    
    /* File Word: Gán hàm lookup vào root inode */
    root_inode->i_op = &myfs_dir_inode_ops;
    
    /* TODO 3 (Kernel Lab): Tăng nlink cho thư mục */
    inc_nlink(root_inode);

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        pr_err("myfs: Khong the tao root dentry.\n");
        return -ENOMEM;
    }

    return 0;
}

/* =========================================================================
 * TODO 1 (Kernel Lab) & File Word: Đăng ký File System
 * ========================================================================= */
static struct dentry *myfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
    return mount_nodev(fs_type, flags, data, myfs_fill_super);
}

static struct file_system_type myfs_type = {
    .owner = THIS_MODULE,
    .name = "myfs",
    .mount = myfs_mount,
    /* Sử dụng kill_litter_super chuẩn đét theo Kernel Lab TODO 1 */
    .kill_sb = kill_litter_super, 
};

static int __init myfs_init(void) {
    int ret = register_filesystem(&myfs_type);
    if (ret == 0) pr_info("myfs: Da dang ky file system.\n");
    return ret;
}

static void __exit myfs_exit(void) {
    unregister_filesystem(&myfs_type);
    pr_info("myfs: Da huy dang ky file system.\n");
}

module_init(myfs_init);
module_exit(myfs_exit);
