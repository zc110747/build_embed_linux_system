

#include <stdio.h>
#include "application/test_file.h"

int main(int argc, char *argv[])
{
    int val = 5;

    printf("arg num: %d, %s, %s\n", argc, argv[1], argv[2]);
    
    test_file_process(val);

    return 0;
}