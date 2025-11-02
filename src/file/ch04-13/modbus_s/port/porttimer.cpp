/*
 * FreeModbus Libary: LPC214X Port
 * Copyright (C) 2007 Tiago Prado Lone <tiago@maxwellbohr.com.br>
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

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mbport.h"
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>

#include <atomic>

#include "logger.hpp"

/* ----------------------- static functions ---------------------------------*/
static uint8_t tick = 0;

/* ----------------------- Start implementation -----------------------------*/
void vTimerCallback(int sig) 
{
    LOG_INFO(0, "vTimerCallback")
    EnterCriticalSection();
    pxMBPortCBTimerExpired();
    ExitCriticalSection();
}

BOOL
xMBPortTimersInit( USHORT usTim1Timerout50us )
{
    signal(SIGALRM, vTimerCallback);
    return TRUE;
}

void
vMBPortTimersEnable( void )
{
    struct itimerval tick;

    tick.it_value.tv_sec = 0;
    tick.it_value.tv_usec = 200;
 
    //After first, the Interval time for clock
    tick.it_interval.tv_sec = 0;
    tick.it_interval.tv_usec = 0;
    
    LOG_DEBUG(0, "vMBPortTimersEnable")
    setitimer(ITIMER_REAL, &tick, NULL);
}

void
vMBPortTimersDisable( void )
{
    struct itimerval value;
    value.it_value.tv_sec = 0;  
    value.it_value.tv_usec = 0;  
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = 0;

    LOG_DEBUG(0, "vMBPortTimersDisable")
    setitimer(ITIMER_REAL, &value, NULL); 
}
