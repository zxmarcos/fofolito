/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções responsáveis por fazer a comunicação com a Mailbox0 do RPi
 *
 * Marcos Medeiros
 */
#ifndef __FB_CONSOLE_H__
#define __FB_CONSOLE_H__

#include <types.h>
#include <kernel/fb.h>

int fb_console_init();
void fb_console_putc(char chr);

#endif/*__FB_CONSOLE_H__*/
