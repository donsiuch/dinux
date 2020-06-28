
#include "dinux/inc/math.h"

int power(int base, int power)
{
    int res = 1;

    while (power > 0)
    {
        if (power & 1)
        {            
            res = res * base;
        }
        base = base * base;
        power >>= 1;
    }
    return res;
}

