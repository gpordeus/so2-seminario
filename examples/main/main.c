#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "arch/riscv/trap.h"

volatile uintptr_t* clint_address() {
	return (uintptr_t*) 0x200004;
}

volatile uintptr_t* msip_address(unsigned int hart_id) {
	return (uintptr_t*) (0x020000UL + (4UL * hart_id));
}

/*
unsigned long read_mtime() {
	return *mtime_address();
}

unsigned long read_mtimecmp(unsigned int hart_id) {
	return *mtimecmp_address(hart_id);
}

void write_mtimecmp(unsigned int hart_id, uintptr_t time) {
	*mtimecmp_address(hart_id) = time;
}
*/

static unsigned int read_mhartid() {
	unsigned int id;
	asm("csrr %0, mhartid" : "=r"(id) : : );
	return id;
}

static void set_mstatus(unsigned int bits) {
	asm("csrs mstatus, %0" : : "r"(bits) : );
}

static void set_mie(unsigned int bits) {
	asm("csrs mie, %0" : : "r"(bits) : );
}
static void clear_mie() {
	asm("csrw mie, x0");
}

/*
void enable_machine_timer_interrupt() {
	unsigned int mie = 3;
	set_mstatus(1 << mie);
	
	unsigned int machine_timer = 7;
	set_mie(1 << machine_timer);
}

void schedule_interrupt(uintptr_t delay) {
	write_mtimecmp(read_mhartid(), read_mtime() + delay);
}
*/

void my_handler(uintptr_t* regs, uintptr_t mcause, uintptr_t mepc) {
	uintptr_t interrupt_bit = 1UL << 63;
	unsigned is_interrupt = (mcause & interrupt_bit != 0);
	
	if (!is_interrupt) {
		printf("machine mode: not interrupt %d @ %p\n", mcause, mepc);
		exit(-1);
		return;
	}

	unsigned int exception_code = mcause & 0b1111111111;

	printf("cpu: %ld - %ld\n", read_mhartid(), exception_code);

	if (exception_code == 3) {
		printf("IPI happened");
	}
}

int main(int argc, char** argv) {
	unsigned int hart_id = read_mhartid();
	printf("%ld\n", hart_id);
	
	if (hart_id == 0) {
		set_trap_fn(my_handler);
		set_mstatus(1 << 3);
		clear_mie(1 << 7);
		set_mie(1 << 3);
		for (unsigned int i = 0; i < 1000000000; i++);
		unsigned char one = 1;
		*clint_address(1) = one;
		printf("boop");
	}
	else {
		set_mstatus(1 << 3);
		clear_mie(1 << 7);
		set_mie(1 << 3);
		asm("wfi");
	}

}
