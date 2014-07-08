/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Marcos Medeiros
 */
#include <kernel/printk.h>
#include <kernel/block.h>
#include <kernel/sched.h>
#include <kernel/mm.h>
#include <types.h>
#include <errno.h>
#include <memory.h>

struct ramdisk_info {
	unsigned char *start;
	unsigned long size;
	unsigned long nr_blocks;
};

static int ramdisk_init(struct block_dev *dev)
{
	if (!dev)
		return -EINVPARAM;
	return -EOK;
}

static int ramdisk_exit(struct block_dev *dev)
{
	return -ENOIMPL;
}

static int ramdisk_ioctl(struct block_dev *dev, int cmd, ...)
{
	return -ENOIMPL;
}

static int ramdisk_read(struct block_dev *dev, unsigned block,
						char *buf, unsigned nblocks)
{
	struct ramdisk_info *rd = dev->private;
	unsigned long count = nblocks * BLOCK_SIZE;
	unsigned long src = block * BLOCK_SIZE;

	/* Tentando acessar um bloco inexistente */
	if (block >= rd->nr_blocks)
		return -EINVPARAM;

	if ((src + count) >= rd->size) {
		count -= (src + count) - rd->size; 
	}

	memcpy_s(buf, rd->start + src, count);
	return -EOK;
}

static int ramdisk_write(struct block_dev *dev, unsigned block,
						 const char *buf, unsigned nblocks)
{
	return -ENOIMPL;
}

int ramdisk_setup(unsigned char *start, unsigned long size)
{
	struct block_dev *dev = NULL;
	struct ramdisk_info *rd = NULL;

	dev = kmalloc(sizeof(struct block_dev));
	if (!dev)
		return -ENOMEM;

	rd =  kmalloc(sizeof(struct ramdisk_info));
	if (!rd) {
		kfree(dev);
		return -ENOMEM;
	}

	/* Configura alguns parâmetros */
	rd->start = start;
	rd->size = size;
	rd->nr_blocks = size / BLOCK_SIZE;
	dev->private = rd;

	dev->name = "ramdisk";
	/* Configura as operações */
	dev->init = ramdisk_init;
	dev->exit = ramdisk_exit;
	dev->ioctl = ramdisk_ioctl;
	dev->read = ramdisk_read;
	dev->write = ramdisk_write;

	return -EOK;
}