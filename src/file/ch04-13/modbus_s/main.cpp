/*
Modbus TCP格式
功能 | TID | PID | Length | UID | FUNC | Data |
长度 |  2  |  2  |   2    |  1  |   1  |  Length-2     |

TID（Transaction Identifier) : 发送标识，用于区分不同的请求，每次请求都需要递增。
PID（Protocol Identifier）：协议标识符，固定为0。
Length（Length）：数据长度，包括功能码和数据的长度。
UID（Unit Identifier）：允许单元设备使用多个地址，以区分多个设备。
FUNC（Function Code）：功能码，用于标识请求的功能类型。

{0xC8, 0xf2, 0x35, 0x00, 0x00, 0x00}
//8位的倍数直接返回，非8的倍数，剩余填充0
//读线圈: 0x01, start: 1 len: 4
发送: 00 00 00 00 00 06 01 01 00 00 00 04
响应: 00 00 00 00 00 04 01 01 01 08

//读线圈: 0x01, start: 1 len: 8
发送: 00 00 00 00 00 06 01 01 00 00 00 08
响应: 00 00 00 00 00 04 01 01 01 C8

//读线圈: 0x01, start: 1 len: 15
发送: 00 00 00 00 00 06 01 01 00 00 00 0f
响应: 00 00 00 00 00 04 01 01 01 C8 72

//写单个线圈: 0x05, start: 1 value: 1
发送: 00 00 00 00 00 06 01 05 00 00 FF 00
响应: 00 00 00 00 00 06 01 05 00 00 FF 00

发送: 00 00 00 00 00 06 01 01 00 00 00 08
响应: 00 00 00 00 00 04 01 01 01 C9

//写单个线圈: 0x05, start: 4 value: 0
发送: 00 00 00 00 00 06 01 05 00 03 00 00
响应: 00 00 00 00 00 06 01 05 00 03 00 00

发送: 00 00 00 00 00 06 01 01 00 00 00 08
响应: 00 00 00 00 00 04 01 01 01 C1

//写多个线圈: 0x0F, start: 0 len: 4, val:A
发送: 00 00 00 00 00 08 01 0F 00 00 00 04 01 0A
响应: 00 00 00 00 00 06 01 0F 00 00 00 04

//读线圈
发送: 00 00 00 00 00 06 01 01 00 00 00 08
响应: 00 00 00 00 00 04 01 01 01 CA
*/
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
*/
#define REG_COIL_START           0x0001
#define REG_COIL_NREGS           48
static UCHAR   usRegCoilBuf[REG_COIL_NREGS/8] = {0x10, 0xf2, 0x35, 0x00, 0x00, 0x00};

#define REG_COIL_LED_R           0x0000
#define REG_COIL_LED_G           0x0001
#define REG_COIL_LED_B           0x0002
#define REG_COIL_BEEP            0x0003
#define REG_COIL_TUBE_A          0x0004
#define REG_COIL_TUBE_B          0x0005

static void coil_hardware_process(UCHAR *pbuffer, USHORT usAddress, USHORT usNCoils)
{
    USHORT reg_addr;
    UCHAR uBit;
    
    auto hardware_process = [](USHORT reg_addr, UCHAR uBit) {
        switch (reg_addr) {
            case REG_COIL_LED_R:
                if (uBit) {
                    std::cout << "led r on" << std::endl;
                } else {
                    std::cout << "led r off" << std::endl;
                }
                break;
            case REG_COIL_LED_G:
                if (uBit) {
                    std::cout << "led g on" << std::endl;
                } else {
                    std::cout << "led g off" << std::endl;
                }
                break;
            case REG_COIL_LED_B:
                if (uBit) {
                    std::cout << "led b on" << std::endl;
                } else {
                    std::cout << "led b off" << std::endl;
                }
                break;
            case REG_COIL_BEEP:
                if (uBit) {
                    std::cout << "beep on" << std::endl;
                } else {
                    std::cout << "beep off" << std::endl;
                }
                break;
            case REG_COIL_TUBE_A:
                if (uBit) {
                    std::cout << "tube a on" << std::endl;
                } else {
                    std::cout << "tube a off" << std::endl;
                }
                break;
            case REG_COIL_TUBE_B:
                if (uBit) {
                    std::cout << "tube b on" << std::endl;
                } else {
                    std::cout << "tube b off" << std::endl;
                }
                break;
            default:
                break;
            }  
    };
    
    for (int i = 0; i < usNCoils; i++) {
        reg_addr = usAddress + i - 1;
        uBit = xMBUtilGetBits(pbuffer, reg_addr, 1);
        hardware_process(reg_addr, uBit);
    }
}

eMBErrorCode eMBRegCoilsCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode )
{
    UCHAR *pucStartRegBuffer;
    USHORT usRegIndex;
    USHORT usCoilnums;

    // 检查寄存器范围是否满足要求
    if (usAddress < REG_COIL_START
    || (usAddress + usNCoils > REG_COIL_START+REG_COIL_NREGS)) {
        return MB_ENOREG;
    } 
    
    usRegIndex = usAddress - REG_COIL_START;
    usCoilnums = usNCoils;
    pucStartRegBuffer = pucRegBuffer;

    switch (eMode) {
        case MB_REG_WRITE: //写入bit，每次写入8bit，不足写入剩余bit
            while( usCoilnums > 0 ) {
                xMBUtilSetBits( usRegCoilBuf, usRegIndex, (uint8_t)( usCoilnums > 8 ? 8 : usCoilnums ), *pucStartRegBuffer++ );
                if (usCoilnums > 8) {
                    usCoilnums -= 8;
                    usRegIndex += 8;
                } else {
                    break;
                }
            }
            coil_hardware_process(usRegCoilBuf, usAddress, usNCoils);
            break;
        case MB_REG_READ: //读取bit，每次读取8bit，不足8bit剩余位用0填充
            while( usCoilnums > 0 ) {
                *pucStartRegBuffer++ = xMBUtilGetBits(usRegCoilBuf, usRegIndex, ( uint8_t )( usCoilnums > 8 ? 8 : usCoilnums ) );
                if (usCoilnums > 8) {
                    usCoilnums -= 8;
                    usRegIndex += 8;
                } else {
                    break;
                }
            }
            break;
    }
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
#define REG_DISCRETE_NREGS           48
static UCHAR   usRegDiscreateBuf[REG_DISCRETE_NREGS/8] = {0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};  
eMBErrorCode eMBRegDiscreteCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete )
{
    eMBErrorCode eStatus = MB_ENOERR;
    USHORT usRegIndex;

    // 检查寄存器范围是否满足要求
    if (usAddress < REG_COIL_START
    || (usAddress + usNDiscrete > REG_COIL_START+REG_COIL_NREGS)) {
        return MB_ENOREG;
    }
    
    usRegIndex = usAddress - REG_COIL_START;

    // 读取离散输入寄存器值，每次读取8bit，不足8bit剩余位用0填充
    while( usNDiscrete > 0 ) {
        *pucRegBuffer++ = xMBUtilGetBits(usRegDiscreateBuf, usRegIndex, (uint8_t)( usNDiscrete > 8 ? 8 : usNDiscrete ) );
        if(usNDiscrete > 8) {
            usNDiscrete -= 8;
            usRegIndex += 8;
        } else {
            break;
        }
    }

    return eStatus;    
}

/*
功能码: 03(0x03), 读取保持寄存器((每个寄存器代表16bit数据)
#define MB_FUNC_READ_DISCRETE_INPUTS          (  2 )
eMBException eMBFuncReadDiscreteInputs( UCHAR * pucFrame, USHORT * usLen )eMBRegHoldingCB
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
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS] = {0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A};
eMBErrorCode
eMBRegHoldingCB( UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode )
{
    eMBErrorCode    eStatus = MB_ENOERR;
    int             iRegIndex;

    // 检查寄存器范围是否满足要求
    if((usAddress < REG_HOLDING_START) 
    || (usAddress + usNRegs > REG_HOLDING_START + REG_HOLDING_NREGS)) {
        return MB_ENOREG;   
    }

    iRegIndex = usAddress - usRegHoldingStart;

    switch (eMode) {                                       
        case MB_REG_READ: //读取寄存器双字节
            while (usNRegs > 0) {
                *pucRegBuffer++ = (usRegHoldingBuf[iRegIndex] >> 8);            
                *pucRegBuffer++ = (usRegHoldingBuf[iRegIndex] & 0xFF); 
                iRegIndex++;
                usNRegs--;					
            }                            
            break;
        case MB_REG_WRITE: //写入寄存器双字节
            while(usNRegs > 0) {         
                usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
                usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
                iRegIndex++;
                usNRegs--;
            }
        break;
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

    // 检查寄存器范围是否满足要求
    if(usAddress < REG_INPUT_START
    || (usAddress + usNRegs > REG_INPUT_START + REG_INPUT_NREGS)) {
        return MB_ENOREG;
    }

    iRegIndex = ( int )( usAddress - usRegInputStart );

    // 读取输入寄存器值双字节
    while (usNRegs > 0) {
        *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] >> 8 );
        *pucRegBuffer++ = ( unsigned char )( usRegInputBuf[iRegIndex] & 0xFF );
        iRegIndex++;
        usNRegs--;
    }

    return eStatus;
}
