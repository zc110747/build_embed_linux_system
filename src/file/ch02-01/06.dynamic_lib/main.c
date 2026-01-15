//////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2025-by Persion Inc.  
//  All Rights Reserved
//
//  Name:
//      main.c
//
//  Purpose:
//     
//
// Author:
//      @公众号：<嵌入式技术总结>
//
// Revision History:
// 
/////////////////////////////////////////////////////////////////////////////
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
