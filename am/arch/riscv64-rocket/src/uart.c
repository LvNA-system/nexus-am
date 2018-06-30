#include <klib.h>
#include "riscv64-rocket.h"

#define UART_BASE 0x60000000

static volatile uint8_t* uart_base_ptr;

#define UART_RX_FIFO_REG 0
#define UART_TX_FIFO_REG 0x4
#define UART_STAT_REG 0x8
#define UART_CTRL_REG 0xc

void uart_send(uint8_t data) {
	// wait until THR not full
	while((*(uart_base_ptr + UART_STAT_REG) & 0x08));
	*(uart_base_ptr + UART_TX_FIFO_REG) = data;
}

int uart_recv() {
	// check whether RBR has data
	if(! (*(uart_base_ptr + UART_STAT_REG) & 0x01u)) {
		return -1;
	}
	return *(uart_base_ptr + UART_RX_FIFO_REG);
}

void uart_init() {
  uart_base_ptr = (volatile void *)UART_BASE + read_const_csr(mhartid) * 0x1000;
  *(uart_base_ptr + UART_CTRL_REG) = 0x3;
}
