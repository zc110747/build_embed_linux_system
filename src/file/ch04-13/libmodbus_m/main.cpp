/*
对于Ubuntu平台，使用sudo apt-get install libmodbus-dev安装支持库
也可以下载https://github.com/stephane/libmodbus自行编译
*/
#include <modbus/modbus.h>
#include <cstdio>
#include <cerrno>
#include <cstdint>
#include <cassert>

#define MODBUS_TCP_IP       "127.0.0.1"
#define MODBUS_TCP_PORT     19920
#define MODBUS_TCP_UNIT_ID  1

void convert_reg_coils(uint8_t *coilBuffer, int coil_count, uint8_t *regBuffer, uint32_t reg_bit_count)
{
    for (int i = 0; i < coil_count; ++i) {
        for (int j = 0; j < 8; ++j) {
            int bitIndex = i * 8 + j;
            if (bitIndex < reg_bit_count) {
                regBuffer[bitIndex] = (coilBuffer[i] >> j) & 0x01;
            }
        }
    }
}

void show_reg_coils(uint8_t *regBuffer, uint32_t reg_bit_count)
{
    printf("usRegCoilReadBuf: [");

    for (int i = 0; i < reg_bit_count; ++i) {
        if (i != reg_bit_count - 1) {
            printf("0x%x, ", regBuffer[i]);
        }
        else {
            printf("0x%x", regBuffer[i]);
        }
    }

    printf("]\n");
}

void assert_coils_equal(uint8_t *bitBuffer, uint8_t *usRegCoilReadBuf, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        assert(bitBuffer[i] == usRegCoilReadBuf[i]);
    }
}

static void app_modbus_coils(modbus_t *ctx)
{
    uint8_t usRegCoilBuf[] = {0xc8, 0xf2, 0x35, 0x00, 0x00, 0x00};
    constexpr int reg_bit_count = 48;
    uint8_t bitBuffer[reg_bit_count];

    printf("============================ coils regs test ============================\n");
    convert_reg_coils(usRegCoilBuf, sizeof(usRegCoilBuf)/sizeof(uint8_t), bitBuffer, reg_bit_count);

    // modbus_write_bits 写入多个bit
    modbus_write_bits(ctx, 0, 48, bitBuffer);
    uint8_t usRegCoilReadBuf[48];
    
    // modbus_read_bits 读取多个bit
    int rc = modbus_read_bits(ctx, 0, 4, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    } 
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);

    rc = modbus_read_bits(ctx, 0, 8, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);

    rc = modbus_read_bits(ctx, 0, 15, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);

    // modbus_write_bit 写入一个bit
    modbus_write_bit(ctx, 0, 1);
    usRegCoilBuf[0] |= 1;
    convert_reg_coils(usRegCoilBuf, sizeof(usRegCoilBuf)/sizeof(uint8_t), bitBuffer, reg_bit_count);
    rc = modbus_read_bits(ctx, 0, 8, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);

    modbus_write_bit(ctx, 3, 0);
    usRegCoilBuf[0] &= ~(1 << 3);
    convert_reg_coils(usRegCoilBuf, sizeof(usRegCoilBuf)/sizeof(uint8_t), bitBuffer, reg_bit_count);
    rc = modbus_read_bits(ctx, 0, 8, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);

    // modbus_write_bits 写入多个bit
    usRegCoilBuf[0] = 0x3f;
    convert_reg_coils(usRegCoilBuf, sizeof(usRegCoilBuf)/sizeof(uint8_t), bitBuffer, reg_bit_count);
    modbus_write_bits(ctx, 0, 8, bitBuffer);
    rc = modbus_read_bits(ctx, 0, 8, usRegCoilReadBuf);
    if (rc < 0) {
        printf("modbus_read_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegCoilReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegCoilReadBuf, rc);
}

static void app_modbus_input_bits(modbus_t *ctx)
{
    uint8_t usRegDiscreateBuf[] = {0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde};
    constexpr int reg_bit_count = 48;
    uint8_t usRegDiscreateReadBuf[reg_bit_count];
    uint8_t bitBuffer[reg_bit_count];;

    printf("============================ input bits regs test ============================\n");
    convert_reg_coils(usRegDiscreateBuf, sizeof(usRegDiscreateBuf)/sizeof(uint8_t), bitBuffer, reg_bit_count);

    int rc = modbus_read_input_bits(ctx, 0, 16, usRegDiscreateReadBuf);
    show_reg_coils(usRegDiscreateReadBuf, rc);
    assert_coils_equal(bitBuffer, usRegDiscreateReadBuf, rc);

    rc = modbus_read_input_bits(ctx, 7, 16, usRegDiscreateReadBuf);
    if (rc < 0) {
        printf("modbus_read_input_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegDiscreateReadBuf, rc);
    assert_coils_equal(bitBuffer+7, usRegDiscreateReadBuf, rc);

    rc = modbus_read_input_bits(ctx, 12, 8, usRegDiscreateReadBuf);
    if (rc < 0) {
        printf("modbus_read_input_bits error: %s\n", modbus_strerror(errno));
        return;
    }
    show_reg_coils(usRegDiscreateReadBuf, rc);
    assert_coils_equal(bitBuffer+12, usRegDiscreateReadBuf, rc);
}

void show_hold_regs(uint16_t *regBuffer, uint32_t  count)
{
    printf("usRegHoldingBuf: [");

    for (int i = 0; i < count; ++i) {
        if (i != count - 1) {
            printf("0x%04x, ", regBuffer[i]);
        }
        else {
            printf("0x%04x", regBuffer[i]);
        }
    }

    printf("]\n");
}

void assert_hold_equal(uint16_t *RegBuffer, uint16_t *holdReadBuffer, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        assert(RegBuffer[i] == holdReadBuffer[i]);
    }
}

static void app_modbus_holds(modbus_t *ctx)
{
    uint16_t usRegHoldingBuf[] = {0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A};
    uint16_t usRegHoldingReadBuf[10];

    printf("============================ holds regs test ============================\n");
    // modbus_write_registers 写入多个hold寄存器
    modbus_write_registers(ctx, 0, 10, usRegHoldingBuf);
    
    // modbus_read_registers 读取多个hold寄存器
    int rc = modbus_read_registers(ctx, 0, 10, usRegHoldingReadBuf);
    if (rc < 0) {
        printf("modbus_read_registers error: %s\n", modbus_strerror(errno));
        return;
    }
    show_hold_regs(usRegHoldingReadBuf, rc);
    assert_hold_equal(usRegHoldingBuf, usRegHoldingReadBuf, rc);

    rc = modbus_read_registers(ctx, 1, 6, usRegHoldingReadBuf);
    if (rc < 0) {
        printf("modbus_read_registers error: %s\n", modbus_strerror(errno));
        return;
    }
    show_hold_regs(usRegHoldingReadBuf, rc);
    assert_hold_equal(usRegHoldingBuf+1, usRegHoldingReadBuf, rc);

    // modbus_write_register 写入一个hold寄存器
    modbus_write_register(ctx, 3, 0x1235);
    usRegHoldingBuf[3] = 0x1235;
    rc = modbus_read_registers(ctx, 0, 10, usRegHoldingReadBuf);
    if (rc < 0) {
        printf("modbus_read_registers error: %s\n", modbus_strerror(errno));
        return;
    }
    show_hold_regs(usRegHoldingReadBuf, rc);
    assert_hold_equal(usRegHoldingBuf, usRegHoldingReadBuf, rc);
}

void show_input_regs(uint16_t *regBuffer, uint32_t  count)
{
    printf("usRegInputBuf: [");

    for (int i = 0; i < count; ++i) {
        if (i != count - 1) {
            printf("0x%04x, ", regBuffer[i]);
        }
        else {
            printf("0x%04x", regBuffer[i]);
        }
    }

    printf("]\n");
}


void assert_input_equal(uint16_t *RegBuffer, uint16_t *inputReadBuffer, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        assert(RegBuffer[i] == inputReadBuffer[i]);
    }
}

static void app_modbus_input(modbus_t *ctx)
{
    uint16_t usRegInputBuf[] = {0x1000, 0x1001, 0x1002, 0x1003, 0, 0, 0, 0, 0, 0};
    uint16_t usRegInputReadBuf[10];

    printf("============================ input regs test ============================\n");
    int rc = modbus_read_input_registers(ctx, 0, 5, usRegInputReadBuf);
    if (rc < 0) {
        printf("modbus_read_input_registers error: %s\n", modbus_strerror(errno));
        return;
    }
    show_input_regs(usRegInputReadBuf, rc);
    assert_input_equal(usRegInputBuf, usRegInputReadBuf, rc);
}

int main() 
{
    modbus_t *ctx = modbus_new_tcp(MODBUS_TCP_IP, MODBUS_TCP_PORT);
    if (!ctx) {
        fprintf(stderr, "Failed to create TCP context\n");
        return -1;
    }

    if (modbus_set_slave(ctx, MODBUS_TCP_UNIT_ID) == -1) {
        fprintf(stderr, "Failed to set slave ID\n");
        modbus_free(ctx);
        return -1;
    }

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    app_modbus_coils(ctx);

    app_modbus_input_bits(ctx);

    app_modbus_holds(ctx);

    app_modbus_input(ctx);
    
__end:
    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
