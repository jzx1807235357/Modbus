# -*- coding: utf-8 -*-
"""简易 Modbus RTU 主站测试：读/写保持寄存器"""
import argparse
import sys

from pymodbus.client import ModbusSerialClient


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", default="COM5")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--slave", type=int, default=1)
    ap.add_argument("--addr", type=int, default=0)
    ap.add_argument("--count", type=int, default=2)
    ap.add_argument("--write", type=int, nargs=2, metavar=("ADDR", "VALUE"),
                    help="写单个保持寄存器，例如 --write 1 4660")
    args = ap.parse_args()

    client = ModbusSerialClient(
        port=args.port,
        baudrate=args.baud,
        bytesize=8,
        parity="N",
        stopbits=1,
        timeout=1.0,
    )
    if not client.connect():
        print(f"打开串口失败: {args.port}")
        return 1

    try:
        if args.write is not None:
            addr, value = args.write
            rr = client.write_register(address=addr, value=value, device_id=args.slave)
            if rr.isError():
                print(f"写失败: {rr}")
                return 2
            print(f"写成功: slave={args.slave} addr={addr} value={value}")

        rr = client.read_holding_registers(
            address=args.addr, count=args.count, device_id=args.slave
        )
        if rr.isError():
            print(f"读失败: {rr}")
            return 3

        regs = rr.registers
        print(f"读成功: slave={args.slave} addr={args.addr} count={args.count}")
        for i, v in enumerate(regs):
            print(f"  reg[{args.addr + i}] = {v} (0x{v:04X})")
        return 0
    finally:
        client.close()


if __name__ == "__main__":
    sys.exit(main())
