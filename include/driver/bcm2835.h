#ifndef __BCM2835_H__
#define __BCM2835_H__

#include <types.h>

#define RPI_GPIO_MAX	54
#define MBOX_CHANNEL_FB	1
#define GPIO_FUNC_OUTPUT	1
#define GPIO_FUNC_INPUT	0

/* 256MB */
#define RPI_TOTAL_MEMORY	(1024 * 1024 * 256)


/* mailbox */
int bcm2835_mbox_init();
uint bcm2835_mbox_read(uint channel);
void bcm2835_mbox_write(uint channel, uint data);

/* framebuffer */
void bcm2835_fb_init();

/* gpio */
void bcm2835_gpio_init();
void bcm2835_gpio_setfunction(uint pin, uint function);
void bcm2835_gpio_output_clearset(uint pin, uint clear);
void bcm2835_turn_led(int state);

/* timer */
int bcm2835_timer_init();
void bcm2835_timer_start();
uint bcm2835_timer_read();

/* emmc */
void bcm2835_emmc_init();

#endif/*__BCM2835_H__*/