OUTPUT_ARCH(arm)
ENTRY(pre_init)
SECTIONS {

	/*
	 * O código que carrega nosso kernel no endereço correto,
	 * ele precisa estar linkado no endereço físico.
	 */
	.stext 0x10000 : AT(0x10000) {
		obj/init.o
	}

	/*
	 * Apenas o bootloader é ligado no endereço físico, todas as outras
	 * variáveis estão ligadas no endereço virtual
	 */
	. = 0xC0000000 + .;

	/*
	 * Nosso diretório de páginas do kernel, tem que
	 * ser alinhado nos 16KB, definimos dois diretórios, 
	 * k_init_pgdir será utilizado somente no boot, apenas para colocar
	 * o kernel rodando no endereçamento virtual, k_pgdir será contruido
	 * assim que o kernel iniciar.
	 */
	. = ALIGN(4096 * 4);

	PROVIDE(k_pgdir = .);
	. += 16K;
	PROVIDE(k_init_pgdir = .);
	. += 16K;
	

	/*
	 * Nosso kernel é mapeado em 0xC0000000, no entanto como
	 * esse endereço é o mapeamento linear da memória física,
	 * o endereço físico do kernel tem que ser o mesmo.
	 */
	. = 0xC0020000;
	.text : AT(0x20000) {
		PROVIDE(k_stack_svc = .);
		PROVIDE(k_reloc_start = .);
		obj/start.o
		*(.text)
		*(.text.*)
	}
	. = ALIGN(4096);

	.data : {
		*(.data)
		*(.data.*)
	}
	. = ALIGN(4096);
	.rodata : {
		*(.rodata)
		*(.rodata.*)
	}
	. = ALIGN(4096);
	.bss : {
		*(.bss)
	}
	. = ALIGN(4096);
	/* Vamos criar as stacks para outros modos, todos com 4kb */
	. = . + 4096;
	PROVIDE(k_stack_irq = .);
	. = . + 4096;
	PROVIDE(k_stack_abt = .);
	. = . + 4096;
	PROVIDE(k_stack_und = .);
	PROVIDE(k_reloc_end = .);
	/DISCARD/ : {
		*(.comment*)
	}
}
