
#include "dinux/inc/mm.h"

unsigned long num_avail_pages = 0;
unsigned long num_used_pages = 0;

// Array of data desribing the state of every page in the
// system
struct page *mem_map;

