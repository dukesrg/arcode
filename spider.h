#ifndef _SPIDER_H
#define _SPIDER_H

#define START_SECTION __attribute__ ((section (".text.start"), naked))

// make sure code is PIE
#ifndef __PIE__
#error "Must compile with -fPIE"
#endif

int(*GX_SetTextureCopy)(void *input_buffer, void *output_buffer, unsigned int size, int in_x, int in_y, int out_x, int out_y, int flags) = (void *)0x0011DD48;
int(*GSPGPU_FlushDataCache)(void *addr, unsigned int len) = (void *)0x00191504;
int(*svcSleepThread)(unsigned long long nanoseconds) = (void *)0x0023FFE8;

int uvl_entry();
int uvl_exit(int status);

// TODO: move out into .c file
int START_SECTION uvl_start() {
	__asm__ volatile (".word 0xE1A00000");
	uvl_entry();
	__asm__ volatile ("svc 0x03");
}

// TODO: move out into .c file
int uvl_exit(int status) {
	return 0;
}


#endif /* _SPIDER_H */
