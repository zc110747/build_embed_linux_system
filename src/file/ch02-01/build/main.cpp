#include "application.hpp"
#include <stdio.h>

extern void static_test(void);
extern void dynamic_test(void);

int main(void)
{
    printf("hello world!\r\n");
    
    application_run();

    static_test();

    dynamic_test();
    
    return 0;
}