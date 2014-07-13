/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Driver de framebuffer 
 *
 * Marcos Medeiros
 */
#include <asm/barrier.h>
#include <asm/asm.h>
#include <asm/io.h>
#include <kernel/fb.h>
#include <driver/bcm2835.h>
#include <types.h>
#include <errno.h>

static const struct fbdev_mode bcm2835_fb_modes[] = {
#ifdef __EMUVERSION__
	{ 800, 600, 16 },
#endif
	{ 1024, 720, 16 },
	{ 1024, 720, 24 },
};

struct mbox_fb_message {
	uint width;
	uint height;
	uint virtual_width;
	uint virtual_height;
	uint pitch;
	uint bpp;
	uint xoffset;
	uint yoffset;
	uint baseptr;
	uint size;
} __attribute__((packed));


volatile struct mbox_fb_message msgfb __attribute__((aligned(16)));
/*
 * Faz o modeset do framebuffer
 */
static uint bcm2835_fb_modeset(struct fbdev *dev, const struct fbdev_mode *mode)
{
	/* Se não for nos dado um modo, pegamos nosso modo padrão */
	if (!mode)
		mode = &bcm2835_fb_modes[0];

	msgfb.width = mode->width;
	msgfb.height = mode->height;
	msgfb.virtual_width = mode->width;
	msgfb.virtual_height = mode->height;
	msgfb.bpp = mode->bpp;
	msgfb.pitch = 0;
	msgfb.xoffset = 0;
	msgfb.yoffset = 0;
	msgfb.baseptr = 0;
	msgfb.size = 0;


	/* É necessário tentar algumas vezes até que nossa solicitação
	 * seja atendida pela videocore.
	 */
	do {
		do_dsb();
		/* Envia uma mensagem para a Videocore informando sobre o nosso modo */
		bcm2835_mbox_write(MBOX_CHANNEL_FB, __pa(&msgfb));
		/* Espera a resposta */
		bcm2835_mbox_read(MBOX_CHANNEL_FB);
		do_dmb();
	} while (msgfb.baseptr == 0);

	dev->width = msgfb.width;
	dev->height = msgfb.height;
	dev->bpp = msgfb.bpp;
	dev->pitch = msgfb.pitch;
	dev->size = msgfb.size;
	dev->base = ioremap(msgfb.baseptr, dev->size);

	return -EOK;
}

static struct fbdev bcm2835_fbdev = {
	.name = "bcm2835_rpi_fb",
	.modeset = bcm2835_fb_modeset,
	.maprgb = fb_generic_maprgb,
};

void bcm2835_fb_init()
{
	fb_register_device(&bcm2835_fbdev);
}