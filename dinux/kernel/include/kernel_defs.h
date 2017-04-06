
#ifndef __KERNEL_DEFS__
#define __KERNEL_DEFS__

const unsigned char	KERNEL_CS = 0x10;
const unsigned char	KENREL_DS = 0x18;

// Pass arguments on the stack only.
#define asmlinkage __attribute__((regparm(0)))

#endif
