#ifndef __CALLBACK_H__
#define __CALLBACK_H__

register void *reg_a4 asm("%a4");

#define CALLBACK_PROLOGUE \
	void *save_a4 = reg_a4; asm("move.l %%a5,%%a4; sub.l #edata,%%a4" : :);

#define CALLBACK_EPILOGUE reg_a4 = save_a4;

#endif
