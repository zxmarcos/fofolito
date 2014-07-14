/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Essas definições são incluidas por arquivos escritos em assembly
 *
 * Marcos Medeiros
 */
#ifndef __PLATFORM_H
#define __PLATFORM_H

/* O frequência padrão é de 250MHz */
#define SYS_CLOCK_FREQ	250000000

void arch_early_init();
void arch_setup();

#endif
