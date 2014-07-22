/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __BLOCK_DEV_H__
#define __BLOCK_DEV_H__

#include <kernel/list.h>

#define BLOCK_SIZE	512

struct block_dev {
	struct list_head list;
	const char *name;
	void *private;
	int (*init)(struct block_dev *dev);
	int (*exit)(struct block_dev *dev);
	int (*ioctl)(struct block_dev *dev, int cmd, ...);
	int (*read)(struct block_dev *dev, unsigned block, char *buf, unsigned nblocks);
	int (*write)(struct block_dev *dev, unsigned block, const char *buf, unsigned nblocks);
};

#endif