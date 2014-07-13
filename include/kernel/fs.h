/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __FS_H__
#define __FS_H__

struct inode;
struct inode_ops;
struct superblock;
struct superblock_ops;

struct superblock {
	struct block_dev *bdev;
	void *private;
	struct inode *root;
};

struct inode {
	unsigned long type;
	unsigned long flags;
	unsigned long id;
	unsigned short uid;
	unsigned short gid;
	unsigned long size;
	struct superblock *sb;
	struct inode_ops *ops;
	void *private;
};

struct inode_ops {

};



#endif
