
#ifndef __MM__
#define __MM__

// Describes physical page frame
struct page {
    // Number of pte's that map to this frame
    unsigned long in_use;

    // Reserved for future use
    unsigned long dummy0;
    unsigned long dummy1;
    unsigned long dummy2;
    unsigned long dummy3;
    unsigned long dummy4;
    unsigned long dummy5;
    unsigned long dummy6;
} __attribute((packed));

#endif
