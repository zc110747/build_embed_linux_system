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