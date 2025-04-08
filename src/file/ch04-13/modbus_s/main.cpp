
#include "mb.h"
#include "mbport.h"
#include "mbutils.h"
#include "logger.hpp"
#include <sys/stat.h>
#include <unistd.h>
#include <mutex>

#define MODBUS_DEF_ADDRESS          0x01        //从设备地址
#define MODBUS_DEF_UBAUD            115200      //设备波特率
#define MODBUS_DEF_PARITY           MB_PAR_NONE //奇偶校验位
#define MODBUS_DEF_TCP_PORT         19920

#define MODBUS_RUN_RTU              0
#define MODBUS_RUN_ASCII            1
#define MODBUS_RUN_TCP              2
#define MODBUS_RUN_MODE             MODBUS_RUN_TCP


static std::mutex mutex_protect;
/* ----------------------- Start implementation -----------------------------*/
void
EnterCriticalSection(  )
{
    mutex_protect.lock();
}

void
ExitCriticalSection(  )
{
    mutex_protect.unlock();
}

static eMBErrorCode modbus_initialize(void)
{
    eMBErrorCode eStatus = MB_EINVAL;

#if MODBUS_RUN_MODE == MODBUS_RUN_RTU
    eStatus = eMBInit( MB_RTU, MODBUS_DEF_ADDRESS, 0, MODBUS_DEF_UBAUD, MODBUS_DEF_PARITY );
#elif MODBUS_RUN_MODE == MODBUS_RUN_ASCII
    eStatus = eMBInit( MB_ASCII, MODBUS_DEF_ADDRESS, 0, MODBUS_DEF_UBAUD, MODBUS_DEF_PARITY );
#elif MODBUS_RUN_MODE == MODBUS_RUN_TCP
    eStatus = eMBTCPInit( MODBUS_DEF_TCP_PORT );
#endif
    if ( eStatus != MB_ENOERR ) {
        LOG_ERROR(0, "modbus init failed, error code: {}", static_cast<int>(eStatus));
        return eStatus;
    }
    eStatus = eMBEnable(  );
    if ( eStatus!= MB_ENOERR ){
        LOG_ERROR(0, "modbus enable failed, error code: {}", static_cast<int>(eStatus));
        return eStatus;
    }
    
    return eStatus;
}

int main(int argc, char *argv[])
{
    logger_manage::get_instance()->set_log_level(logger_manage::LOGGER_LEVEL::INFO);

    //modbus initialize.
    if ( modbus_initialize() != MB_ENOERR ) {
        LOG_ERROR(0, "modbus initialize failed");
        exit(-1);
    }

    for(;;)
    {
        ( void )eMBPoll(  );
    }    
}

/*
CRC校验: 高位在前, 低位在后

功能码: 01(0x01), 读线圈状态，读取连续的值(每个寄存器代表1bit数据)，可读可写
#define MB_FUNC_READ_COILS                    ( 1 )
eMBException eMBFuncReadCoils( UCHAR * pucFrame, USHORT * usLen )
RTU请求:  | 01 | 01 | 00 00 | 00 08 | 3d cc | => | 从设备地址 | 功能码 | 寄存器首地址,实际+1 | 寄存器长度 | CRC校验 |
从机响应: | 01 | 01 | 01 | 10 | 50 44 | => | 从设备地址 | 功能码 | 数据个数 | 寄存器内数据 | CRC校验 |

功能码: 05(0x05), 写单个线圈(0xFF 0x00表示1, 0x00 0x00为0, 其它则错误)
#define MB_FUNC_WRITE_SINGLE_COIL             (  5 )
eMBException eMBFuncWriteCoil( UCHAR * pucFrame, USHORT * usLen )
RTU请求:  | 01 | 05 | 00 01 | FF 00 | dd fa | => | 从设备地址 | 功能码 | 寄存器首地址 | 变更数据 | CRC校验 |
从机响应: | 01 | 05 | 00 01 | FF 00 | dd fa | => | 从设备地址 | 功能码 | 寄存器首地址 | 变更数据 | CRC校验 |

功能码: 15(0x0F), 写多个线圈
#define MB_FUNC_WRITE_MULTIPLE_COILS          ( 15 )
eMBException eMBFuncWriteMultipleCoils( UCHAR * pucFrame, USHORT * usLen )
RTU请求:  | 01 | 0F | 00 00 | 00 04 | 01 | 0x0F | 7f 5e | => | 从设备地址 | 功能码 | 寄存器首地址 | 线圈数目 | 写入字节个数 | 写入字节 | CRC校验 |
从机响应: | 01 | 0F | 00 00 | 00 04 | 54 08 |             => | 从设备地址 | 功能码 | 寄存器首地址 | 线圈数目 | CRC校验 |

对应硬件: LED, mos管等
*/
#define REG_COIL_START           0x0001
#define REG_COIL_NREGS           10
static UCHAR   usRegCoilBuf[REG_COIL_NREGS] = {0x10, 0x01, 0x01, 0x00, 0x01, 0x01, 0x00, 0x01};

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    int i;

    /* check if we away of table size */
    if (usAddress < REG_COIL_START
    || usAddress + usNCoils > REG_COIL_START+REG_COIL_NREGS) {
        return MB_ENOREG;
    } 
    
    //covert to register list
    usAddress -= REG_COIL_START;
    
    switch (eMode)
    {
        case MB_REG_WRITE:
            for (i = 0; i < usNCoils; i++) {
                UCHAR wbit = xMBUtilGetBits(pucRegBuffer, i, 1 );
                xMBUtilSetBits( usRegCoilBuf, usAddress+i, 1, wbit );
            }
            break;
        case MB_REG_READ:
            for (i = 0; i < usNCoils; i++) 
            {
                UCHAR rbit = xMBUtilGetBits( usRegCoilBuf, usAddress+i, 1 );
                xMBUtilSetBits( pucRegBuffer, i, 1, rbit );
            }
            break;
    }

    //printf("eMode:%d, usAddress:%d, usNCoils:%d, %d\n", eMode, usAddress, usNCoils, pucRegBuffer[0]);
    return MB_ENOERR;
}

/*
功能码: 02(0x02), 读取离散输入(每个寄存器代表1bit数据)，只读
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
eMBException eMBFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )调用eMBRegDiscreteCB
RTU请求:  | 01 | 02 | 00 00 | 00 04 | 25 3a | => | 从设备地址 | 功能码 | 离散寄存器地址 | 离散寄存器长度 | CRC校验 |
从机响应: | 01 | 02 | 01 | 03 | 2a 1c | => | 从设备地址 | 功能码 | 数据个数 | 寄存器内数据 | CRC校验 |
*/
#define REG_DISCRETE_START           0x0001
#define REG_DISCRETE_NREGS           10
static UCHAR   usRegDiscreateBuf[REG_DISCRETE_NREGS] = {0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01};  
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode eStatus = MB_ENOERR;

    /* check if we away of table size */
    if (usAddress <  REG_COIL_START
    || usAddress + usNDiscrete > REG_COIL_START+REG_COIL_NREGS) {
        return MB_ENOREG;
    }
    usAddress -= REG_COIL_START;
    
    while( usNDiscrete > 0 )
    {
        *pucRegBuffer++ = xMBUtilGetBits(usRegDiscreateBuf, usAddress, (uint8_t)( usNDiscrete > 8 ? 8 : usNDiscrete ) );
        
        if(usNDiscrete > 8)
        {
            usNDiscrete -= 8;
            usAddress += 8;
        }
        else
        {
            break;
        }
    }

    return eStatus;    
}

/*
功能码: 03(0x03), 读取保持寄存器((每个寄存器代表16bit数据)
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
eMBException eMBFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )调用eMBRegDiscreteCB
RTU请求:  | 01 | 03 | 00 00 | 00 02 | 25 3a | => | 从设备地址 | 功能码 | 保持寄存器地址 | 保持寄存器长度 | CRC校验 |
从机响应: | 01 | 03 | 02 | 0x10 0x00 | 2a 1c | => | 从设备地址 | 功能码 | 数据个数 | 寄存器内数据 | CRC校验 |

功能码: 06(0x06), 写入保持寄存器((每个寄存器代表16bit数据)
#define MB_FUNC_WRITE_REGISTER                (  6 )
eMBException eMBFuncWriteHoldingRegister( UCHAR * pucFrame, USHORT * usLen )
RTU请求:  | 01 | 06 | 00 00 | 00 0A | 25 3a | => | 从设备地址 | 功能码 | 保持寄存器地址 | 保持寄存器数据 | CRC校验 |
从机响应: | 01 | 06 | 00 00 | 00 0A | 25 3a | => | 从设备地址 | 功能码 | 数据个数 | 寄存器内数据 | CRC校验 |

*/
#define REG_HOLDING_START           0x0001
#define REG_HOLDING_NREGS           10
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {0x1000, 0x1001, 0x1002, 0x1003};
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if((usAddress >= REG_HOLDING_START)&&\
    ((usAddress+usNRegs) <= (REG_HOLDING_START + REG_HOLDING_NREGS)))
    {
        iRegIndex = (int)(usAddress - usRegHoldingStart);
        switch(eMode)
        {                                       
            case MB_REG_READ://读 MB_REG_READ = 0
                while(usNRegs > 0)
                {
                    *pucRegBuffer++ = (usRegHoldingBuf[iRegIndex] >> 8);            
                    *pucRegBuffer++ = (usRegHoldingBuf[iRegIndex] & 0xFF); 
                    iRegIndex++;
                    usNRegs--;					
                }                            
            break;
            case MB_REG_WRITE://写 MB_REG_WRITE = 0
                while(usNRegs > 0)
                {         
                    usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                    usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                    iRegIndex++;
                    usNRegs--;
                }
            break;
        }
    }
    else//错误
    {
        eStatus = MB_ENOREG;
    }	

    return eStatus;
}

/*
功能码: 04, 读取输入寄存器(每个数据2字节)

#define MB_FUNC_READ_INPUT_REGISTER          (  4 )
eMBException eMBFuncReadInputRegister( UCHAR * pucFrame, USHORT * usLen )eMBRegInputCB

RTU请求:  | 01 | 04 | 00 00 | 00 02 | 25 3a | => | 从设备地址 | 功能码 | 离散寄存器地址 | 离散寄存器长度 | CRC校验 |
从机响应: | 01 | 04 | 02 | 0x10 0x00 0x10 0x01 | 2a 1c | => | 从设备地址 | 功能码 | 数据个数 | 寄存器内数据 | CRC校验 |
*/
#define REG_INPUT_START           0x0001
#define REG_INPUT_NREGS           10
static USHORT   usRegInputStart = REG_INPUT_START;
static USHORT   usRegInputBuf[REG_INPUT_NREGS] = {0x1000, 0x1001, 0x1002, 0x1003};

eMBErrorCode
eMBRegInputCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    if( ( usAddress >= REG_INPUT_START )
        && ( usAddress + usNRegs <= REG_INPUT_START + REG_INPUT_NREGS ) )
    {
        iRegIndex = ( int )( usAddress - usRegInputStart );
        while( usNRegs > 0 )
        {
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
            *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
            iRegIndex++;
            usNRegs--;
        }
    }
    else
    {
        eStatus = MB_ENOREG;
    }

    return eStatus;
}
