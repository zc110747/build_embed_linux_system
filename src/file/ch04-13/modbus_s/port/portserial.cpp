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
/* ----------------------- Modbus includes ----------------------------------*/
#include <stdio.h>
#include <string.h>
#include "port.h"
#include "mb.h"
#include "mbport.h"

#include <thread>
#include <memory>

#include "semaphore.hpp"
#include "logger.hpp"
#include "fifo_manage.hpp"
#include "tty.hpp"

#define PORT_SERIAL_NODE        "/dev/ttyS1"

#define PORT_SERIAL_MODE        0
#define PORT_FIFO_MODE          1
#define PORT_RUN_MODE           PORT_SERIAL_MODE


/* ----------------------- static parameter ---------------------------------*/
#define RX_BUFFER_SIZE      1024
#define TX_BUFFER_SIZE      1024

typedef struct {
    char tx_buffer[TX_BUFFER_SIZE];
    int tx_buffer_size;
    char rx_buffer[RX_BUFFER_SIZE]; 
    int rx_buffer_size;
    bool is_tx_enable;

#if PORT_RUN_MODE == PORT_SERIAL_MODE
    tty_control tty;
#else
    std::unique_ptr<fifo_manage> fifo_point_;
#endif

    EVENT::semaphore tx_semaphore;
}portserial_t;

static portserial_t portserial;

/* ----------------------- static functions ---------------------------------*/
static BOOL prvvUARTTxReadyISR( void );
static void prvvUARTRxISR( void );

/* ----------------------- Start implementation -----------------------------*/
void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{  
    if (xTxEnable) {
        portserial.tx_buffer_size = 0;
        portserial.tx_semaphore.signal();
    } 

    if (xRxEnable) {
        portserial.rx_buffer_size = 0; 
    }
}

void
vMBPortClose( void )
{
}

static std::string convert_parity(eMBParity eParity)
{
    if (eParity == MB_PAR_NONE) {
        return "N";
    } else if (eParity == MB_PAR_ODD) {
        return "O";
    } else {
        return "E";
    }
}

void port_tty_tx_thread(void)
{
    while (1) {
        if( portserial.tx_semaphore.wait(TIME_ACTION_ALWAYS) ) {
            while (!prvvUARTTxReadyISR()) {  //调用xMBPortSerialPutByte，将数据写入缓存中    
            }
#if PORT_RUN_MODE == PORT_SERIAL_MODE 
            portserial.tty.write(portserial.tx_buffer, portserial.tx_buffer_size);
#else
            LOG_INFO(0, "fifo_point_ tx:{}", portserial.tx_buffer_size);
            portserial.fifo_point_->write(portserial.tx_buffer, portserial.tx_buffer_size);
#endif
        }
    }
}

void port_tty_rx_thread(void)
{
    ssize_t n_size;

    while (1) {
#if PORT_RUN_MODE == PORT_SERIAL_MODE
        n_size = portserial.tty.read(portserial.rx_buffer, RX_BUFFER_SIZE);
#else
        n_size = portserial.fifo_point_->read(portserial.rx_buffer, RX_BUFFER_SIZE);
#endif
        if (n_size > 0) {        
            LOG_INFO(0, "rx n_size:{}", n_size); 
            portserial.rx_buffer_size = 0;
            for (int index=0; index<n_size; index++)
            {
                prvvUARTRxISR();
            }
        } else if ( n_size == 0) {
            continue;
        } else {
        }
    }
}

BOOL
xMBPortSerialInit( UCHAR ucPORT, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
    std::string parity;

#if PORT_RUN_MODE == PORT_SERIAL_MODE
    if (!portserial.tty.init(PORT_SERIAL_NODE, O_RDWR | O_NOCTTY)) {
        return FALSE;
    }

    if (!portserial.tty.set_opt(ulBaudRate, ucDataBits, convert_parity(eParity), 1)) {
        return FALSE;
    }
#else
    portserial.fifo_point_ = std::make_unique<fifo_manage>("/tmp/fifo_modbus_slave", S_FIFO_WORK_MODE);
    if(portserial.fifo_point_ == nullptr) {
        LOG_ERROR(0, "fifo_manage malloc error");
        return FALSE;
    }

    if (!portserial.fifo_point_->create()) {
       LOG_ERROR(0, "fifo_manage create error");
       return FALSE;
    }
#endif
    std::thread(port_tty_tx_thread).detach();
    std::thread(port_tty_rx_thread).detach();

    LOG_INFO(0, "port serial init success");
    return TRUE;
}

BOOL xMBPortSerialPutByte( UCHAR ucByte )
{
    portserial.tx_buffer[portserial.tx_buffer_size++] = ucByte;
    return TRUE;
}

BOOL xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = portserial.rx_buffer[portserial.rx_buffer_size++];
    return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
static BOOL
prvvUARTTxReadyISR( void )
{
    return pxMBFrameCBTransmitterEmpty( );
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
static void
prvvUARTRxISR( void )
{
    pxMBFrameCBByteReceived(  );
}
