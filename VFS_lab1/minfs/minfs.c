#include <linux/module.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/version.h>
#include "minfs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Lab Student");
MODULE_DESCRIPTION("minfs - Minimal File System with disk support");

/* =========================================================================
 * TODO 3: Creating and destroying minfs inodes
 * ========================================================================= */
static struct inode *minfs_alloc_inode(struct super_block *sb) {
    struct minfs_inode_info *mii;
    mii = kzalloc(sizeof(struct minfs_inode_info), GFP_KERNEL);
    if (!mii) return NULL;
    
    inode_init_once(&mii->vfs_inode);
    return &mii->vfs_inode;
}

static void minfs_destroy_inode(struct inode *inode) {
    struct minfs_inode_info *mii = container_of(inode, struct minfs_inode_info, vfs_inode);
    kfree(mii);
}

/* =========================================================================
 * TODO 4: Initialize minfs root inode
 * ========================================================================= */
static struct inode *minfs_iget(struct super_block *sb, unsigned long ino) {
    struct inode *inode;
    struct buffer_head *bh;
    struct minfs_inode *di;

    inode = iget_locked(sb, ino);
    if (!inode) return ERR_PTR(-ENOMEM);
    if (!(inode->i_state & I_NEW)) return inode;

    bh = sb_bread(sb, ino);
    if (!bh) {
        iget_failed(inode);
        return ERR_PTR(-EIO);
    }

    di = (struct minfs_inode *)bh->b_data;
    
    inode->i_mode = di->i_mode;
    i_uid_write(inode, di->i_uid);
    i_gid_write(inode, di->i_gid);
    inode->i_size = di->i_size;
    
    /* [PATCH] Xu ly loi TimeStamps tren Kernel 6.6+ */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
    {
        struct timespec64 now = current_time(inode);
        inode_set_atime_to_ts(inode, now);
        inode_set_mtime_to_ts(inode, now);
        inode_set_ctime_to_ts(inode, now);
    }
#else
    inode->i_atime = inode->i_mtime = inode->i_ctime = current_time(inode);
#endif

    if (S_ISDIR(inode->i_mode)) {
        inode->i_op = &simple_dir_inode_operations;
        inode->i_fop = &simple_dir_operations;
        inc_nlink(inode);
    }

    brelse(bh);
    unlock_new_inode(inode);
    return inode;
}

static const struct super_operations minfs_ops = {
    .alloc_inode   = minfs_alloc_inode,
    .destroy_inode = minfs_destroy_inode,
    .drop_inode    = generic_delete_inode,
    .statfs        = simple_statfs,
};

/* =========================================================================
 * TODO 2: Completing minfs superblock
 * ========================================================================= */
static int minfs_fill_super(struct super_block *sb, void *data, int silent) {
    struct buffer_head *bh;
    struct minfs_super_block *msb;
    struct minfs_sb_info *sbi;
    struct inode *root_inode;

    sbi = kzalloc(sizeof(struct minfs_sb_info), GFP_KERNEL);
    if (!sbi) return -ENOMEM;
    sb->s_fs_info = sbi;

    sb_set_blocksize(sb, MINFS_BLOCK_SIZE);

    bh = sb_bread(sb, 0);
    if (!bh) {
        kfree(sbi);
        return -EIO;
    }

    msb = (struct minfs_super_block *)bh->b_data;
    sb->s_magic = msb->magic;
    sbi->version = msb->version;
    brelse(bh);

    if (sb->s_magic != MINFS_MAGIC) {
        pr_err("minfs: Invalid magic number\n");
        kfree(sbi);
        return -EINVAL;
    }

    sb->s_op = &minfs_ops;

    root_inode = minfs_iget(sb, 1);
    if (IS_ERR(root_inode)) {
        kfree(sbi);
        return PTR_ERR(root_inode);
    }

    sb->s_root = d_make_root(root_inode);
    if (!sb->s_root) {
        kfree(sbi);
        return -ENOMEM;
    }

    return 0;
}

/* =========================================================================
 * TODO 1: Registering and unregistering the minfs file system
 * ========================================================================= */
static struct dentry *minfs_mount(struct file_system_type *fs_type, int flags, const char *dev_name, void *data) {
    return mount_bdev(fs_type, flags, dev_name, data, minfs_fill_super);
}

static struct file_system_type minfs_fs_type = {
    .owner    = THIS_MODULE,
    .name     = "minfs",
    .mount    = minfs_mount,
    .kill_sb  = kill_block_super,
    .fs_flags = FS_REQUIRES_DEV,
};

static int __init minfs_init(void) {
    return register_filesystem(&minfs_fs_type);
}

static void __exit minfs_exit(void) {
    unregister_filesystem(&minfs_fs_type);
}

module_init(minfs_init);
module_exit(minfs_exit);
