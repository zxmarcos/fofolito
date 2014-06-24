/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Manipulador de irqs específico para o RaspberryPi
 * O maior problema aqui é que podemos receber interrupções
 * de dispositivos da GPU e também de dispositivos do ARM.
 * Marcos Medeiros
 */
#include <types.h>
#include <asm/irq.h>
#include <asm/asm.h>
#include <asm/io.h>
#include <asm/context.h>
#include <errno.h>
#include <kernel/printk.h>

/* Dependente da plataforma, no RPi são 64 */
#define GPU_IRQ_MAX	64
#define ARM_IRQ_MAX	8

#define IRQ_SIZE	0x30
#define IRQ_IOBASE	0x2000B000

static volatile unsigned *irqregs = NULL;

/* registrados para IRQs do arm */
#define REG_IRQBPEN	(0x200 >> 2)
#define REG_IRQBENA	(0x218 >> 2)
#define REG_IRQBDIS	(0x224 >> 2)

/* registrados para IRQs da gpu */
#define REG_IRQPEN1	(0x204 >> 2)
#define REG_IRQPEN2	(0x208 >> 2)
#define REG_IRQENA1	(0x210 >> 2)
#define REG_IRQENA2	(0x214 >> 2)
#define REG_IRQDIS1	(0x21C >> 2)
#define REG_IRQDIS2	(0x220 >> 2)

#define GPU_PENDING_1	(1 << 8)
#define GPU_PENDING_2	(1 << 9)

/* Tabela com os serviços para IRQs */
irq_service_t arm_irq_service_table[ARM_IRQ_MAX] = {
	NULL
};

irq_service_t gpu_irq_service_table[GPU_IRQ_MAX] = {
	NULL
};

/* Desabilita as interrupção por requisição (IRQs) */
void irq_disable()
{
	asm volatile("mrs r0, cpsr		\n"
		         "orr r0, %[bits]	\n"
		         "msr cpsr, r0		\n"
		:: [bits] "r" (CPSR_IRQ_DISABLE));
}

/* Habilita as interrupção por requisição (IRQs) */
void irq_enable()
{
	asm volatile("mrs r0, cpsr		\n"
		         "bic r0, %[bits]	\n"
		         "msr cpsr, r0		\n"
		:: [bits] "r" (CPSR_IRQ_DISABLE));
}

/* Um contador de quantas irqs perdidas */
volatile uint gpu_unhandled_irqs = 0;
volatile uint arm_unhandled_irqs = 0;

/* Nosso servidor de interrupções padrão */
static int gpu_isr_default(void)
{
	/* Apenas conta */
	gpu_unhandled_irqs++;
	return -EOK;
}

static int arm_isr_default(void)
{
	/* Apenas conta */
	arm_unhandled_irqs++;
	return -EOK;
}


static inline void gpu_irq_handler_1()
{
	uint val = irqregs[REG_IRQPEN1];
	if (val & 0xFFFFFFFF) {
		int i = 0;
		while ((i < 32) && val) {
			if (val & 1) {
				gpu_irq_service_table[i]();
			}
			val >>= 1;
			i++;
		}
	}
}

static inline void gpu_irq_handler_2()
{
	uint val = irqregs[REG_IRQPEN2];
	if (val & 0xFFFFFFFF) {
		int i = 0;
		while ((i < 32) && val) {
			if (val & 1) {
				gpu_irq_service_table[32 + i]();
			}
			val >>= 1;
			i++;
		}
	}
}

/* Habilita uma IRQ específica (GPU) */
static inline void gpu_irq_enable_line(uint irq)
{
	if (irq >= GPU_IRQ_MAX)
		return;
	uint bank = irq / 32;
	uint reg = REG_IRQENA1 + bank;
	irqregs[reg] = (1 << (irq % 32));
}

/* Desabilita uma IRQ específica (GPU) */
static inline void gpu_irq_disable_line(uint irq)
{
	if (irq >= GPU_IRQ_MAX)
		return;
	uint bank = irq / 32;
	uint reg = REG_IRQDIS1 + bank;
	irqregs[reg] = (1 << (irq % 32));
}

/* Habilita uma IRQ específica (ARM) */
static inline void arm_irq_enable_line(uint irq)
{
	if (irq >= ARM_IRQ_MAX)
		return;
	irqregs[REG_IRQBENA] = (1 << (irq & 7));
}

/* Desabilita uma IRQ específica (ARM) */
static inline void arm_irq_disable_line(uint irq)
{
	if (irq >= ARM_IRQ_MAX)
		return;
	irqregs[REG_IRQBDIS] = (1 << (irq & 7));
}


/* Instala um serviço para tratar uma IRQ (GPU) */
static inline int gpu_irq_install_service(uint irq, irq_service_t service)
{
	if (irq >= GPU_IRQ_MAX)
		return -EINVPARAM;
	if (!service)
		gpu_irq_service_table[irq] = gpu_isr_default;
	else
		gpu_irq_service_table[irq] = service;
	return -EOK;
}

/* Instala um serviço para tratar uma IRQ (ARM) */
static inline int arm_irq_install_service(uint irq, irq_service_t service)
{
	if (irq >= ARM_IRQ_MAX)
		return -EINVPARAM;
	if (!service)
		arm_irq_service_table[irq] = arm_isr_default;
	else
		arm_irq_service_table[irq] = service;
	return -EOK;
}

void irq_enable_line(uint irq)
{
	if (irq & GPU_IRQ_FLAG)
		gpu_irq_enable_line(irq & ~GPU_IRQ_FLAG);
	else {
		arm_irq_enable_line(irq);
		printk("Habilitando interrupcao ARM %d\n", irq);
	}
}

void irq_disable_line(uint irq)
{
	if (irq & GPU_IRQ_FLAG)
		gpu_irq_disable_line(irq & ~GPU_IRQ_FLAG);
	else
		arm_irq_disable_line(irq);
}

int irq_install_service(uint irq, irq_service_t service)
{
	if (irq & GPU_IRQ_FLAG)
		return gpu_irq_install_service(irq & ~GPU_IRQ_FLAG, service);
	else
		return arm_irq_install_service(irq, service);
}

/*
 * Nossa rotina de tratamento, aqui é nossa função é investigar
 * quem causou a interrupção e chamar o serviço.
 */
void irq_handler()
{
	uint val = irqregs[REG_IRQBPEN];

	int i = 0;
	/* Despacha as interrupções vindas pelo ARM */
	for (; i < ARM_IRQ_MAX; i++) {
		if (val & (1 << i))
			arm_irq_service_table[i]();
	}

	if (val & GPU_PENDING_1)
		gpu_irq_handler_1();
	if (val & GPU_PENDING_2)
		gpu_irq_handler_2();
}


void irq_init()
{
	irqregs = ioremap(IRQ_IOBASE, IRQ_SIZE);
	int i = 0;
	for (; i < GPU_IRQ_MAX; i++) {
		gpu_irq_service_table[i] = &gpu_isr_default;
		gpu_irq_disable_line(i);
	}
	for (; i < ARM_IRQ_MAX; i++) {
		arm_irq_service_table[i] = &arm_isr_default;
		arm_irq_disable_line(i);
	}
}