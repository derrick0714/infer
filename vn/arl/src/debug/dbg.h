#ifndef DBG_FUNCS
#define DBG_FUNCS

#ifdef DEBUG
 #include <stdio.h>
 #include <iostream>

 #define DBG(x) x

extern "C" {

    #define printhex(_desc, _str, _len)             \
     {                                              \
        unsigned char* str = (unsigned char*)_str;  \
        int len = _len;                             \
                                                    \
        printf(_desc); printf("\n\t");              \
        for (int i = 0; i < len; i++) {             \
            if (i > 0 && i % 8 == 0 && i % 16 != 0) { \
                printf("| ");                       \
            }                                       \
            if (i > 0 && i % 16 == 0) {             \
                printf("\n\t");                     \
            }                                       \
            printf("%02X ", str[i]);                \
        }                                           \
        printf("\n");                               \
    }
}

#else
 #define DBG(x)
#endif

#endif