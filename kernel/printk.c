/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Implementação da função printk
 *
 * Marcos Medeiros
 */

/* TODO:
 *   Implementar um buffer de memória para guardar as mensagens antes
 * de um driver de framebuffer ser instalado, mais ou menos o que acontece
 * no kernel do linux.
 *   Isso também ajudaria futuros programas em user space a lerem as
 * mensagens de log do kernel.
 */
#include <stdarg.h>
#include <memory.h>
#include <driver/fb_console.h>
#include <types.h>
 
/* Número máximo de digitos que um número decimal pode ter 
 * 000.000.000.000
 */
#define DECIMAL_MAX	12

/* Emite um caracter no console */
static inline void putk(uint c)
{
	/* TODO: Implementar o log em buffer */
	fb_console_putc(c);
}

/* Emite um valor em hexadecimal */
static inline void print_hex(uint val, uint hcase)
{
	static const char *hex_chars[2] = { 
		"0123456789ABCDEF",
		"0123456789abcdef"
	};
	/* escolhe a tabela de caracters */
	const char *table = hex_chars[hcase ? 1 : 0];
	int i = 32;
	int k = 7;
	
	while (i >= 4) {
		char c = table[(val >> (k * 4)) & 0xF];
		putk(c);
		k--;
		i -= 4;
	}
}

/* Emite um valor em decimal */
static inline void print_decimal(uint val)
{
	uint digits[DECIMAL_MAX];

	memset(digits, 0, DECIMAL_MAX * sizeof(uint));

	int i = 0;

	/* Decompõe o valor em digitos da direita para esquerda */
	do {
		digits[i] = 0x30 + (val % 10);
		val /= 10;
		i++;
	} while ((val != 0) && (i < DECIMAL_MAX));

	/* Vamos começar do ultimo digito e imprimir da esquerda para direita */
	i--;
	while (i >= 0) {
		putk(digits[i]);
		i--;
	}
}

/* Emite um valor em binário */
static inline void print_binary(uint val, int size)
{
	if (!size)
		return;
	size -= 1;

	/* Nosso tamanho máximo é de 32bits */
	if (size > 32)
		size = 32;
	
	int i = size;
	while (i >= 0) {
		putk(0x30 + ((val >> i) & 1));
		i--;
	}
}

/* Implementação básica da função printk */
void printk(const char *fmt, ...)
{
	va_list va;
	va_start(va, fmt);

	while (*fmt) {
		if (*fmt != '%') {
			putk(*fmt);
			fmt++;
		} else {
			fmt++;
			if (*fmt == '%') {
				putk(*fmt);
				fmt++;
			} else
			if (*fmt == 'd') {
				print_decimal(va_arg(va, int));
				fmt++;
			} else
			if (*fmt == 'x') {
				print_hex(va_arg(va, int), 0);
				fmt++;
			} else
			if (*fmt == 'X') {
				print_hex(va_arg(va, int), 1);
				fmt++;
			} else
			if (*fmt == 's') {
				printk(va_arg(va, const char *));
				fmt++;
			} else
			if (*fmt == 'b') {
				int size = 0;
				fmt++;
				if (*fmt == 'b')
					size = 8;
				else
				if (*fmt == 'w')
					size = 16;
				else
				if (*fmt == 'd')
					size = 32;
				else {
					fmt -= 2;
					size = 32;
				}
				fmt++;
				print_binary(va_arg(va, int), size);
			}
		}
	}
	va_end(va);
}
