
#ifndef __PIC_H__
#define __PIC_H__

#include <stdint.h>

// PIC Master I/0 Command/Data Base Addresses
#define PIC_MASTER		0x20
#define PIC_MASTER_COMMAND	PIC_MASTER
#define PIC_MASTER_DATA		PIC_MASTER + 1

// PIC Slave I/0 Command/Data Base Addresses
#define PIC_SLAVE		0xA0
#define PIC_SLAVE_COMMAND	PIC_SLAVE
#define	PIC_SLAVE_DATA		PIC_SLAVE + 1

// Initialization
#define ICW1_INIT		0x10

// ICW4 needed
#define ICW1_ICW4		0x01

// 8086/88 mode
#define ICW4_8086		0x01

#define PIC_READ_IRR		0x0a
#define PIC_READ_ISR		0x0b

void remapIrq();

uint16_t pic_get_irr(void);
uint16_t pic_get_isr(void);

#endif
