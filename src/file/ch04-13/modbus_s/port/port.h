/*
 * FreeModbus Libary: BARE Port
 * Copyright (C) 2006 Christian Walter <wolti@sil.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id$
 */

#ifndef _PORT_H
#define _PORT_H

#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#ifdef __cplusplus
PR_BEGIN_EXTERN_C
#endif

#include <assert.h>
#include <stdio.h>
#include <inttypes.h>

#define	INLINE

#define ENTER_CRITICAL_SECTION( )		EnterCriticalSection( )
#define EXIT_CRITICAL_SECTION( )    ExitCriticalSection( )

#define CCLK	60000000L
#define PCLK	CCLK/4

void            EnterCriticalSection( void );
void            ExitCriticalSection( void );

typedef uint8_t BOOL;

typedef unsigned char UCHAR;
typedef char    CHAR;

typedef uint16_t USHORT;
typedef int16_t SHORT;

typedef uint32_t ULONG;
typedef int32_t LONG;

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#define PORT_SERIAL_NODE        "/dev/ttyS1"
#define PORT_FIFO_NODE          "/tmp/fifo_modbus_slave"

#define PORT_SERIAL_MODE        0
#define PORT_FIFO_MODE          1
#define PORT_RUN_MODE           PORT_SERIAL_MODE

#ifdef __cplusplus
PR_END_EXTERN_C
#endif
#endif
