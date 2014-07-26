/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções relacionadas ao leitor de cartões eMMC
 *
 * Este é sem dúvidas o primeiro driver "de verdade"
 * que eu escrevo.
 * Marcos Medeiros
 */
#include <asm/io.h>
#include <asm/irq.h>
#include <kernel/printk.h>
#include <driver/bcm2835.h>
#include <types.h>
#include <errno.h>

#define EMMC_IOBASE	0x20300000
#define EMMC_SIZE	0x100
#define EMMC_IRQ	ARM_IRQ(20)
static volatile unsigned *emmc_reg = NULL;

/* Registradores */
#define REG_ARG2			(0x00 >> 2)
#define REG_BLKSIZECNT		(0x04 >> 2)
#define REG_ARG1			(0x08 >> 2)
#define REG_CMDTM			(0x0C >> 2)
#define REG_RESP0			(0x10 >> 2)
#define REG_RESP1			(0x14 >> 2)
#define REG_RESP2			(0x18 >> 2)
#define REG_RESP3			(0x1C >> 2)
#define REG_DATA			(0x20 >> 2)
#define REG_STATUS			(0x24 >> 2)
#define REG_CONTROL0		(0x28 >> 2)
#define REG_CONTROL1		(0x2C >> 2)
#define REG_INT				(0x30 >> 2)
#define REG_INT_MASK		(0x34 >> 2)
#define REG_INT_ENABLE		(0x38 >> 2)
#define REG_CONTROL2		(0x3C >> 2)
#define REG_FORCE_INT		(0x50 >> 2)
#define REG_BOOT_TIMEOUT	(0x70 >> 2)
#define REG_DBG_SEL			(0x74 >> 2)
#define REG_EXRDFIFO_CFG	(0x80 >> 2)
#define REG_EXRDFIFO_EN		(0x84 >> 2)
#define REG_TUNE_STEP		(0x88 >> 2)
#define REG_TUNE_STEPS_STD	(0x8C >> 2)
#define REG_TUNE_STEPS_DDR	(0x90 >> 2)
#define REG_SPI_INT_SPT		(0xF0 >> 2)
#define REG_SLOTISR_VER		(0xFC >> 2)

/* Flags do registrador CMDTM */
#define TM_BLKCNT_EN		0x02
#define TM_NOCMD			0x00	
#define TM_CMD12			0x04
#define TM_CMD23			0x08
#define TM_CMDRESERVED		0x0C
#define TM_DIR_HOST2CARD	0x00
#define TM_DIR_CARD2HOST	0x10
#define TM_SINGLE_BLOCK		0x00
#define TM_MULTI_BLOCK		0x20
#define TM_NORESPONSE		0x00
#define TM_136BITS_RESP		0x1000
#define TM_48BITS_RESP		0x2000
#define TM_48BITS_BUSY_RESP	0x3000
#define TM_NO_CHK_CRC		0x0000
#define TM_CHK_CRC			0x8000
#define TM_CHK_RESP_INDEX	0x10000
#define TM_DATATRANSFER		0x20000
#define TM_CMD_NORMAL		0x00000
#define TM_CMD_SUSPEND		0x40000
#define TM_CMD_RESUME		0x80000
#define TM_CMD_ABORT		0xC0000
#define TM_CMD_INDEX(idx)	((idx & 0x3F) << 24)
#define TM_MASK				(~0xC004FFC1)

/* Flags do registrador STATUS */
#define ST_CMD_INHIBIT		0x00000001
#define ST_DAT_INHIBIT		0x00000002
#define ST_DAT_ACTIVE		0x00000004
#define ST_WRITE_TRANSFER	0x00000100
#define ST_READ_TRANSFER	0x00000200
#define ST_DAT_LINE_0		0x00100000
#define ST_DAT_LINE_1		0x00200000
#define ST_DAT_LINE_2		0x00400000
#define ST_DAT_LINE_3		0x00800000
#define ST_CMD_LINE			0x01000000
#define ST_DAT_LINE_4		0x02000000
#define ST_DAT_LINE_5		0x04000000
#define ST_DAT_LINE_6		0x08000000
#define ST_DAT_LINE_7		0x10000000

/* Flags do registrador INT,INT_MASK,INT_EN e FORCE_INT */
#define	INT_CMD_DONE		0x00000001
#define INT_DATA_DONE		0x00000002
#define INT_BLOCK_GAP		0x00000004
#define INT_WRITE_READY		0x00000010
#define INT_READ_READY		0x00000020
#define INT_CARD_IRQ		0x00000100
#define INT_RETUNE			0x00001000
#define INT_BOOTACK			0x00002000
#define INT_ENDBOOT			0x00004000
#define INT_ERROR			0x00008000
#define INT_CMD_TO_ERROR	0x00010000
#define INT_CMD_CRC_ERROR	0x00020000
#define INT_CMD_END_ERROR	0x00040000
#define INT_CMD_BAD_ERROR	0x00080000
#define INT_DATA_TO_ERROR	0x00100000
#define INT_DATA_CRC_ERROR	0x00200000
#define INT_DATA_END_ERROR	0x00400000
#define INT_ACMD_ERROR		0x01000000

/* Flags do registrador CONTROL1 */
#define C1_CLK_INTLEN		0x00000001
#define C1_CLK_STABLE		0x00000002
#define C1_CLK_ENABLE		0x00000004
#define C1_CLK_GENSEL		0x00000020
#define C1_CLK_FREQ_MS2(v)	(((v) & 0x3) << 6)
#define C1_CLK_FREQ8(v)		(((v) & 0xFF) << 8)
#define C1_DATA_TOUNIT(v)	(((v) & 0xF) << 16)
#define C1_HOST_RESET		0x00100000
#define C1_CMD_RESET		0x00200000
#define C1_DATA_RESET		0x00400000

/* Comandos SD, especificado em:
 * https://www.sdcard.org/downloads/pls/simplified_specs/part1_410.pdf
 * Página 69
 */
#define CMD_GO_IDLE_STATE			0
#define CMD_ALL_SEND_CID			2
#define CMD_SEND_RELATIVE_ADDR		3
#define CMD_SET_DSR					4
#define CMD_SWITCH_FUNC				6
#define CMD_SELECT_DESELECT_CARD	7
#define CMD_SEND_IF_COND			8
#define CMD_SEND_CSD				9
#define CMD_SEND_CID				10
#define CMD_VOLTAGE_SWITCH			11
#define CMD_STOP_TRANSMISSSION 		12
#define CMD_SEND_STATUS				13
#define CMD_GO_INACTIVE_STATE		15
#define CMD_SET_BLOCKLEN			16
#define CMD_READ_SINGLE_BLOCK		17
#define CMD_READ_MULTIPLE_BLOCK		18
#define CMD_SPEED_TUNING_BLOCK		19
#define CMD_SPEED_CLASS_CONTROL		20
#define CMD_SET_BLOCK_COUNT			23
#define CMD_WRITE_BLOCK				24
#define CMD_WRITE_MULTIPLE_BLOCK	25
#define CMD_PROGRAM_CSD				27
#define CMD_SET_WRITE_PROT			28
#define CMD_CLR_WRITE_PROT			29
#define CMD_SEND_WRITE_PROT			30
#define CMD_ERASE_WR_BLK_START		32
#define CMD_ERASE_WR_BLK_END		33
#define CMD_ERASE					38
#define CMD_LOCK_UNLOCK				42
#define CMD_READ_EXTR_SINGLE		48
#define CMD_WRITE_EXTR_SINGLE		49
#define CMD_APP_CMD					55
#define	CMD_GEN_CMD					56
#define CMD_READ_EXTR_MULTI			58
#define CMD_WRITE_EXTR_MULTI		59

/* Comandos específicos de aplicação */
#define	ACMD_SET_BUS_WIDTH			6
#define	ACMD_SD_STATUS				13
#define	ACMD_SEND_NUM_WR_BLOCKS		22
#define	ACMD_SET_WR_BLK_ERASE_COUNT	23
#define ACMD_SEND_OP_COND			41
#define ACMD_SET_CLR_CARD_DETECT	42
#define ACMD_SEND_SCR				51


/* Intervalos de voltagem */
#define OCR_VDD_27_28	0x008000
#define OCR_VDD_28_29	0x010000
#define OCR_VDD_29_30	0x020000
#define OCR_VDD_30_31	0x040000
#define OCR_VDD_31_32	0x080000
#define OCR_VDD_32_33	0x100000
#define OCR_VDD_33_34	0x200000
#define OCR_VDD_34_35	0x400000
#define OCR_VDD_35_36	0x800000

/* Card status */
#define CS_AKE_SEQ_ERROR		0x00000008
#define CS_APP_CMD				0x00000020
#define CS_READY_FOR_DATA		0x00000100
#define CS_CURRENT_STATE(v)		(((v) & 0x1E00) >> 9)	
#define CS_ERASE_RESET			0x00002000
#define CS_CARD_ECC_DISABLE		0x00004000
#define CS_WP_ERASE_SKIP		0x00008000
#define CS_CSD_OVERWRITE		0x00010000
#define CS_ERROR				0x00080000
#define CS_CC_ERROR				0x00100000
#define CS_CARD_ECC_FAILED		0x00200000
#define CS_ILLEGAL_COMMAND		0x00400000
#define CS_COM_CRC_ERROR		0x00800000
#define CS_LOCK_UNLOCK_FAILED	0x01000000
#define CS_CARD_IS_LOCKED		0x02000000
#define CS_WP_VIOLATION			0x04000000
#define CS_ERASE_PARAM			0x08000000
#define CS_ERASE_SEQ_ERROR		0x10000000
#define CS_BLOCK_LEN_ERROR		0x20000000
#define CS_ADDRESS_ERROR		0x40000000
#define CS_OUT_OF_RANGE			0x80000000

static const char *card_status_name[16] = {
	"idle", "ready", "ident", "stby", "tran",
	"data", "rcv", "prg", "dis", "res9", "res10",
	"res11", "res12", "res13", "res14", "res_io"
};

struct emmc_command {
	unsigned opcode;
	unsigned arg;
	unsigned rsp0;
	unsigned rsp1;
	unsigned rsp2;
	unsigned rsp3;
};

#define RSP_NONE	0
#define RSP_R1		TM_48BITS_RESP | TM_CHK_CRC
#define RSP_R1b		TM_48BITS_BUSY_RESP | TM_CHK_CRC
#define RSP_R2		TM_136BITS_RESP | TM_CHK_CRC	
#define RSP_R3		TM_48BITS_RESP
#define RSP_R4		TM_136BITS_RESP | TM_CHK_CRC
#define RSP_R5		TM_48BITS_RESP | TM_CHK_CRC
#define RSP_R6		TM_48BITS_RESP | TM_CHK_CRC
#define RSP_R7		TM_48BITS_RESP | TM_CHK_CRC

static const struct {
	const char *name;
	unsigned resp_type;
} emmc_command_flags[64] = {
/* 00 */ { "go_idle_state",			RSP_NONE },
/* 01 */ { "reserved",				RSP_NONE },
/* 02 */ { "all_send_cid",			RSP_R2 },
/* 03 */ { "send_relative_address",	RSP_R6 },
/* 04 */ { "set_dsr",				RSP_NONE },
/* 05 */ { "reserved",				RSP_NONE },
/* 06 */ { "switch_func",			RSP_R1 },
/* 07 */ { "select_deselect_card",	RSP_R1b },
/* 08 */ { "send_if_cond",			RSP_R7 },
/* 09 */ { "send_csd",				RSP_R2 },
/* 10 */ { "send_cid",				RSP_R2 },
/* 11 */ { "voltage_switch",		RSP_R1 },
/* 12 */ { "stop_transmission",		RSP_R1b },
/* 13 */ { "send_status",			RSP_R1 },
/* 14 */ { "reserved",				RSP_NONE },
/* 15 */ { "go_inactive_state",		RSP_NONE },
/* 16 */ { "set_blocklen",			RSP_R1 },
/* 17 */ { "read_single_block",		RSP_R1 },
/* 18 */ { "read_multiple_block",	RSP_R1 },
/* 19 */ { "speed_tuning_block",	RSP_R1 },
/* 20 */ { "speed_class_control",	RSP_R1b },
/* 21 */ { "reserved",				RSP_NONE },
/* 22 */ { "reserved",				RSP_NONE },
/* 23 */ { "set_block_count",		RSP_R1 },
/* 24 */ { "write_single_block",	RSP_R1 },
/* 25 */ { "write_multiple_block",	RSP_R1 },
/* 26 */ { "reserved",				RSP_NONE },
/* 27 */ { "program_csd",			RSP_R1 },
/* 28 */ { "set_write_prot",		RSP_R1b },
/* 29 */ { "clr_write_prot",		RSP_R1b },
/* 30 */ { "send_write_prot",		RSP_R1 },
/* 31 */ { "reserved",				RSP_NONE },
/* 32 */ { "erase_wr_block_start",	RSP_R1 },
/* 33 */ { "erase_wr_block_end",	RSP_R1 },
/* 34 */ { "reserved",				RSP_NONE },
/* 35 */ { "reserved",				RSP_NONE },
/* 36 */ { "reserved",				RSP_NONE },
/* 37 */ { "reserved",				RSP_NONE },
/* 38 */ { "erase",					RSP_R1b },
/* 39 */ { "reserved",				RSP_NONE },
/* 40 */ { "reserved",				RSP_NONE },
/* 41 */ { "reserved",				RSP_NONE },
/* 42 */ { "lock_unlock",			RSP_R1 },
/* 43 */ { "reserved",				RSP_R1 },
/* 44 */ { "reserved",				RSP_NONE },
/* 45 */ { "reserved",				RSP_NONE },
/* 46 */ { "reserved",				RSP_NONE },
/* 47 */ { "reserved",				RSP_NONE },
/* 48 */ { "read_extr_single",		RSP_R1 },
/* 49 */ { "write_extr_single",		RSP_R1 },
/* 50 */ { "reserved",				RSP_NONE },
/* 51 */ { "reserved",				RSP_NONE },
/* 52 */ { "reserved",				RSP_NONE },
/* 53 */ { "reserved",				RSP_NONE },
/* 54 */ { "reserved",				RSP_NONE },
/* 55 */ { "app_cmd",				RSP_R1 },
/* 56 */ { "gen_cmd",				RSP_R1 },
/* 57 */ { "reserved",				RSP_NONE },
/* 58 */ { "read_extr_multi",		RSP_R1 },
/* 59 */ { "write_extr_multi",		RSP_R1 },
/* 60 */ { "reserved",				RSP_NONE },
/* 61 */ { "reserved",				RSP_NONE },
/* 62 */ { "reserved",				RSP_NONE },
/* 63 */ { "reserved",				RSP_NONE },
};


asm("udelay:			\n"
	"subs	r0, r0, #1	\n"
	"bhi	udelay		\n"
	"mov	pc, lr		\n");
void udelay(unsigned cycles);

/* Envia um comando normal */
static int send_command(struct emmc_command *cmd)
{
	unsigned pkt, rsp_type;
	if (!cmd)
		return -EINVPARAM;

	/* Limpa as respostas anteriores */
	cmd->rsp0 = 0;
	cmd->rsp1 = 0;
	cmd->rsp2 = 0;
	cmd->rsp3 = 0;

	rsp_type = emmc_command_flags[cmd->opcode & 0x3F].resp_type;
	pkt = TM_CMD_INDEX(cmd->opcode);
	pkt = pkt | rsp_type;

	/* Espera até podermos enviar o comando */
	while (1) {
		unsigned status = emmc_reg[REG_STATUS];
		if (status & (ST_CMD_INHIBIT | ST_DAT_INHIBIT)) {
			udelay(100);
			continue;
		}
		break;
	}

	emmc_reg[REG_ARG1] = cmd->arg;
	emmc_reg[REG_CMDTM] = pkt;

	while (1) {
		unsigned status = emmc_reg[REG_STATUS];
		if (status & (ST_CMD_INHIBIT | ST_DAT_INHIBIT)) {
			udelay(100);
			continue;
		}
		break;
	}

	/* Lê as respostas */
	switch (rsp_type) {
		/* respostas de 136bits */
		case RSP_R2:	/* R4 */
			cmd->rsp0 = emmc_reg[REG_RESP0];
			cmd->rsp1 = emmc_reg[REG_RESP1];
			cmd->rsp2 = emmc_reg[REG_RESP2];
			cmd->rsp3 = emmc_reg[REG_RESP3];
			break;
			
		default:
			cmd->rsp0 = emmc_reg[REG_RESP0];
			break;

	}
	return -EOK;
}

static int send_app_command(struct emmc_command *cmd)
{
	struct emmc_command acmd;
	acmd.opcode = CMD_APP_CMD;
	acmd.arg = 0;
	send_command(&acmd);
	send_command(cmd);
	return -EOK;
}

/* Envia um comando de transferência */
static int send_transfer_command(struct emmc_command *cmd, int write)
{
	unsigned pkt, rsp_type;
	if (!cmd)
		return -EINVPARAM;

	/* Limpa as respostas anteriores */
	cmd->rsp0 = 0;
	cmd->rsp1 = 0;
	cmd->rsp2 = 0;
	cmd->rsp3 = 0;

	rsp_type = emmc_command_flags[cmd->opcode & 0x3F].resp_type;
	pkt = TM_CMD_INDEX(cmd->opcode);
	pkt = pkt | rsp_type | TM_DATATRANSFER;

	if (write)
		pkt |= TM_DIR_HOST2CARD;
	else
		pkt |= TM_DIR_CARD2HOST;

	/* Espera até podermos enviar o comando */
	while (1) {
		unsigned status = emmc_reg[REG_STATUS];
		if (status & (ST_CMD_INHIBIT | ST_DAT_INHIBIT)) {
			udelay(100);
			continue;
		}
		break;
	}

	emmc_reg[REG_ARG1] = cmd->arg;
	emmc_reg[REG_CMDTM] = pkt;

	while (1) {
		unsigned status = emmc_reg[REG_STATUS];
		if (status & (ST_CMD_INHIBIT | ST_DAT_INHIBIT)) {
			udelay(100);
			continue;
		}
		break;
	}

	/* Lê as respostas */
	switch (rsp_type) {
		/* respostas de 136bits */
		case RSP_R2:	/* R4 */
			cmd->rsp0 = emmc_reg[REG_RESP0];
			cmd->rsp1 = emmc_reg[REG_RESP1];
			cmd->rsp2 = emmc_reg[REG_RESP2];
			cmd->rsp3 = emmc_reg[REG_RESP3];
			break;
			
		default:
			cmd->rsp0 = emmc_reg[REG_RESP0];
			break;

	}
	return -EOK;
}

static int bcm2835_emmc_handler()
{
	return -EOK;
}

static void decode_cid(struct emmc_command *cmd)
{
	char name[6];
	struct cid_format {
		unsigned mdt : 12;
		unsigned     : 4;
		unsigned psn : 32;
		unsigned prv : 8;
		unsigned pnm_lo : 32;
		unsigned pnm_hi : 8;
		unsigned oid : 16;
		unsigned mid : 8;
	} __attribute__((packed));

	struct cid_format *fmt = (struct cid_format *) &cmd->rsp0;

	name[0] = fmt->pnm_hi & 0xFF;
	name[1] = (fmt->pnm_lo & 0xFF000000) >> 24;
	name[2] = (fmt->pnm_lo & 0xFF0000) >> 16;
	name[3] = (fmt->pnm_lo & 0xFF00) >> 8;
	name[4] = (fmt->pnm_lo & 0xFF);
	name[5] = '\0';

	//printk("CRC %x\n", fmt->crc);
	printk("MDT %x\n", fmt->mdt);
	printk("PSN %x\n", fmt->psn);
	printk("PRV %x\n", fmt->prv);
	printk("PNM %s\n", name);
	printk("OID %x\n", fmt->oid);
	printk("MID %x\n", fmt->mid);
}


static unsigned card_rca = 0;
static void read_test()
{
	struct emmc_command pkt;
	unsigned status = 0;

	printk("READ_BEGIN:\n");
	/* Define o tamanho do bloco */
	pkt.opcode = CMD_SET_BLOCKLEN;
	pkt.arg = 512;
	send_command(&pkt);

	pkt.opcode = CMD_READ_SINGLE_BLOCK;
	pkt.arg = 0;
	send_transfer_command(&pkt, 0);

	status = emmc_reg[REG_STATUS];

	int counter = 0;
	while (status & (1 << 11)) {
		unsigned data = emmc_reg[REG_DATA];
		printk("%X ", data);
		status = emmc_reg[REG_STATUS];
		if (counter++ >= 10) {
			printk("\n");
			counter = 0;
		}
	}

	printk("\nREAD_END;\n");
}

static void init_sequence()
{
	unsigned rca = 0;
	struct emmc_command pkt;
	pkt.opcode = CMD_GO_IDLE_STATE;
	pkt.arg = 0;
	send_command(&pkt);

	/* Envia as condições de operação e um padrão de verificação */
	pkt.opcode = CMD_SEND_IF_COND;
	pkt.arg = 0x13 | (1 << 8);
	send_command(&pkt);

	/* Verifica o padrão */
	if ((pkt.rsp0 & 0xFF) != 0x13)
		return;

	/* Envia a voltagem para operar entre 3.2V e 3.4V */
	pkt.opcode = ACMD_SEND_OP_COND;
	pkt.arg = OCR_VDD_32_33 | OCR_VDD_33_34;
	do {
		send_app_command(&pkt);
		udelay(100);
	} while (!(pkt.rsp0 & 0x80000000));

	/* Pede ao cartão que envie suas informações */
	pkt.opcode = CMD_ALL_SEND_CID;
	pkt.arg = 0;
	send_command(&pkt);
	decode_cid(&pkt);

	/* Pede ao cartão o RCA */
	pkt.opcode = CMD_SEND_RELATIVE_ADDR;
	pkt.arg = 0;
	send_command(&pkt);

	rca = (pkt.rsp0 >> 16) & 0xFFFF;
	printk("RCA %x\n", rca);

	/* Seleciona o cartão */
	pkt.opcode = CMD_SELECT_DESELECT_CARD;
	pkt.arg = rca << 16;
	send_command(&pkt);

	/* Verifica o status atual, esperamos que esteja em trans :) */
	pkt.opcode = CMD_SEND_STATUS;
	send_command(&pkt);

	printk("CS %x %s\n", pkt.rsp0, card_status_name[CS_CURRENT_STATE(pkt.rsp0)]);

	card_rca = rca;
}

void bcm2835_emmc_init()
{
	emmc_reg = mmio_address(EMMC_IOBASE);
	irq_install_service(EMMC_IRQ, &bcm2835_emmc_handler);
	init_sequence();
	read_test();
}