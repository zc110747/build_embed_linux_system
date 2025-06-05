
#include <stdio.h>

extern void print_lib_obj1(void);
extern void print_lib_obj2(void);

int main(int argc, char *argv[])
{
    printf("Hello, World!\n");

    print_lib_obj1();

    print_lib_obj2();
    
    return 0;
}
