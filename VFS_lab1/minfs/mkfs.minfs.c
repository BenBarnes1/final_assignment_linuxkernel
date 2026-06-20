#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "minfs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <device_or_file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Cannot open file/device");
        return 1;
    }

    struct minfs_super_block sb = { .version = 1, .magic = MINFS_MAGIC };
    struct minfs_inode root_ino = {
        .i_mode = S_IFDIR | 0755,
        .i_uid = 0,
        .i_gid = 0,
        .i_size = MINFS_BLOCK_SIZE
    };

    char block[MINFS_BLOCK_SIZE];

    /* Write Superblock to Block 0 */
    memset(block, 0, MINFS_BLOCK_SIZE);
    memcpy(block, &sb, sizeof(sb));
    write(fd, block, MINFS_BLOCK_SIZE);

    /* Write Root Inode to Block 1 */
    memset(block, 0, MINFS_BLOCK_SIZE);
    memcpy(block, &root_ino, sizeof(root_ino));
    write(fd, block, MINFS_BLOCK_SIZE);

    close(fd);
    printf("Formatted %s successfully!\n", argv[1]);
    return 0;
}
