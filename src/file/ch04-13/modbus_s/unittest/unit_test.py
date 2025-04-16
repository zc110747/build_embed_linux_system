
# /usr/bin/python3
# 
# 使用前安装: pip3 install pymodbus
#
from pymodbus.client import ModbusTcpClient as ModbusClient

SOCK_IPADDRESS = '127.0.0.1'
SOCK_PORT = 19920

def hex_to_bool_list(coils_list, bit_count, start_bit=0):
    bool_list = []
    for hex_num in coils_list:
        binary_str = bin(hex_num)[2:].zfill(8)
        binary_str = binary_str[::-1]
        bool_list.extend([bit == '1' for bit in binary_str])

    result = bool_list[start_bit:start_bit+bit_count]

    if len(result) % 8 != 0:
        padding_length = 8 - (len(result) % 8)
        result.extend([False] * padding_length)

    return result

def coils_regs_test(client):
    print("============================ coils regs test ============================")
    usRegCoilBuf = [0xc8, 0xf2, 0x35, 0x00, 0x00, 0x00]
    
    # write multi coils, address:0
    response = client.write_coils(address=0, values=hex_to_bool_list(usRegCoilBuf, 48))

    # read coils, address:0, count:4
    response = client.read_coils(address=0, count=4)
    print("read_coilds, addr:0, count:4", response.bits)
    assert response.bits == hex_to_bool_list(usRegCoilBuf, 4), "read coils failed, adr:0, count:4"
    
    # read coils, address:0, count:8
    response = client.read_coils(address=0, count=8)
    print("read_coilds, addr:0, count:8", response.bits)
    assert response.bits == hex_to_bool_list(usRegCoilBuf, 8), "read coils failed, adr:0, count:8"
    
    # read coils, address:0, count:15
    response = client.read_coils(address=0, count=15)
    print("read_coilds, addr:0, count:15", response.bits)
    assert response.bits == hex_to_bool_list(usRegCoilBuf, 15), "read coils failed, adr:0, count:15"

    # write coils, address:0, value:1
    response = client.write_coil(address=0, value=True)
    usRegCoilBuf[0] |= 0x01
    response = client.read_coils(address=0, count=8)
    print("write_coils, addr:0, val:1", response.bits)
    assert response.bits == hex_to_bool_list(usRegCoilBuf, 8), "read coils failed, adr:0, count:8"

    # write coils, address:3, value:0
    response = client.write_coil(address=3, value=False)
    usRegCoilBuf[0] &= 0xf7
    response = client.read_coils(address=0, count=16)
    print("write_coils, addr:3, val:0", response.bits)
    assert response.bits == hex_to_bool_list(usRegCoilBuf, 16), "read coils failed, adr:0, count:8"

    # write multi coils, address:0
    usRegCoilBuf[1] = 0x3f
    response = client.write_coils(address=7, values=hex_to_bool_list(usRegCoilBuf, 40, 7)[:40])
    response = client.read_coils(address=0, count=16)
    print("write_coils, addr:3, val:0", response.bits)

    # read coils overflow
    respone = client.read_coils(address=10, count=39)
    print(respone)
    assert respone.exception_code == 0x02, "read coils overflow not action"

def discrete_regs_test(client):
    print("=========================  discrete regs test ============================")
    usRegDiscreateBuf = [0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde]
    response = client.read_discrete_inputs(address=0, count=16)
    print("read_discrete_inputs, addr:0, count:16", response.bits)
    assert response.bits == hex_to_bool_list(usRegDiscreateBuf, 16), "read discrete inputs failed, adr:0, count:16"

    response = client.read_discrete_inputs(address=7, count=16)
    print("read_discrete_inputs, addr:7, count:16", response.bits)
    assert response.bits == hex_to_bool_list(usRegDiscreateBuf, bit_count=16, start_bit=7), "read discrete inputs failed, adr:7, count:16"

    response = client.read_discrete_inputs(address=12, count=8)
    print("read_discrete_inputs, addr:12, count:8", response.bits)
    assert response.bits == hex_to_bool_list(usRegDiscreateBuf, bit_count=8, start_bit=12), "read discrete inputs failed, adr:12, count:8"  

    # read discrete overflow
    response = client.read_discrete_inputs(address=0, count=49)
    print(response)
    assert response.exception_code == 0x02, "read discrete inputs overflow not action"

def holding_regs_test(client):
    print("=========================  holding regs test ============================")
    usRegHoldingBuf = [0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A]
    response = client.write_registers(address=0, values=usRegHoldingBuf)
    print(response)

    response = client.read_holding_registers(address=0, count=4)
    print("read_holding_registers, addr:0, count:4", response.registers)
    assert response.registers == usRegHoldingBuf[0:0+4], "read holding registers failed, adr:0, count:4"

    response = client.read_holding_registers(address=1, count=6)
    print("read_holding_registers, addr:1, count:6", response.registers)
    assert response.registers == usRegHoldingBuf[1:1+6], "read holding registers failed, adr:1, count:6"

    usRegHoldingBuf[3] = 0x1235
    response = client.write_register(address=3, value=0x1235)
    response = client.read_holding_registers(address=0, count=10)
    print("write_register, addr:0, val=0x1235", response.registers)
    assert response.registers == usRegHoldingBuf, "read holding registers failed, adr:0, count:10"

    # read holding overflow
    response = client.read_holding_registers(address=2, count=12)
    print(response)
    assert response.exception_code == 0x02, "read holding registers overflow not action"

def input_regs_test(client):
    print("=========================  input regs test ============================")
    usRegInputBuf = [0x1000, 0x1001, 0x1002, 0x1003, 0, 0, 0, 0, 0, 0]
    respone = client.read_input_registers(address=0, count=5)
    print("read_input_registers, addr:0, count:5", respone.registers)
    assert respone.registers == usRegInputBuf[0:0+5], "read input registers failed, adr:0, count:5"

    # read input overflow
    respone = client.read_input_registers(address=0, count=12)
    print(respone)
    assert respone.exception_code == 0x02, "read input registers overflow not action"

def modbus_test():
    client = ModbusClient(SOCK_IPADDRESS, port=SOCK_PORT)
    
    connection = client.connect()

    if connection:

        coils_regs_test(client)

        discrete_regs_test(client)

        holding_regs_test(client)
    
        input_regs_test(client)
    else:
        print("link failed")

    client.close()

if __name__ == '__main__':
    modbus_test()
