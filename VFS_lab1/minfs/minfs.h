#ifndef _MINFS_H
#define _MINFS_H

#include <linux/types.h>

#define MINFS_MAGIC 0x4d494e46 /* "MINF" */
#define MINFS_BLOCK_SIZE 4096

/* Cau truc Superblock tren dia (Block 0) */
struct minfs_super_block {
    uint32_t version;
    uint32_t magic;
};

/* Cau truc Inode tren dia (Root Inode tai Block 1) */
struct minfs_inode {
    uint16_t i_mode;
    uint16_t i_uid;
    uint16_t i_gid;
    uint32_t i_size;
};

#ifdef __KERNEL__
/* Cau truc Superblock mo rong tren RAM VFS */
struct minfs_sb_info {
    uint32_t version;
};

/* Cau truc Inode mo rong tren RAM VFS */
struct minfs_inode_info {
    struct inode vfs_inode;
};
#endif

#endif
