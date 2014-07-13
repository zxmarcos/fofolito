/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Emulação de um console sobre um driver de framebuffer
 * Marcos Medeiros
 */

#include <kernel/fb.h>
#include <driver/fb_console.h>
#include <memory.h>
#include <errno.h>
#include <types.h>

/* Nosso dispositivo de framebuffer */
struct fbdev *dev = NULL;
static const struct fbdev_font *font = &font_vga_8x16;

uint cursor_x = 0;
uint cursor_y = 0;
uint console_columns = 0;
uint console_rows = 0;
uint color_fg = 0;
uint color_bg = 0;
uint char_width = 0;
uint char_height = 0;

/* 
 * Inicia um console sobre o framebuffer.
 * É importante notar que é necessário que o fb já esteja iniciado
 */
int fb_console_init()
{
	dev = fb_get_device();
	if (!dev)
		return -ENODEV;

	cursor_x = 0;
	cursor_y = 0;
	font = &font_vga_8x16;
	console_columns = dev->width / font->width;
	console_rows = dev->height / font->height;
	char_width = font->width;
	char_height = font->height;
	color_bg = dev->maprgb(dev, 0, 0, 0);
	color_fg = dev->maprgb(dev, 200, 200, 200);
	return -EOK;
}

void fb_console_putc(char chr)
{
	if (!dev)
		return;
	
	switch (chr) {
		case '\n':
			goto _newline;
		case '\t':
			cursor_x = (cursor_x & ~3) + 4;
			if (cursor_x >= console_columns)
				goto _newline;
			break;
		default:
			fb_generic_drawchar(dev, font, cursor_x * char_width, cursor_y * char_height,
							 	chr, color_bg, color_fg);
			/* Pulando a linha */
			if (++cursor_x >= console_columns) {
			_newline:
				cursor_x = 0;
				if (++cursor_y >= console_rows) {
					cursor_y--;

					/* faz a rolagem de uma linha */
					fb_generic_scroll(dev, char_height, color_bg);
				}
			}
	}
}