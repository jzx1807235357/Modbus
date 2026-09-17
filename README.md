# STM32F429 Modbus RTU (RS485)

Keil 工程：`Project_Modbus.uvprojx`

## 硬件
- MCU: STM32F429IGTx
- USART2: PD5(TX) / PD6(RX)
- 485 DE/RE: PD11（高发低收）
- HSE: 25 MHz → SYSCLK 180 MHz
- 波特率: 115200 8N1
- 从站地址: 1

## 已实现
- RS485 收发（IDLE 分帧）
- Modbus RTU 从站：功能码 03 / 06 / 10
- 保持寄存器：64 个（地址 0~63）

## 目录
- `User/` 应用与驱动
- `Libraries/` ST HAL + CMSIS
- `Startup/` 启动文件
- `Tools/` 调试辅助（可选）

## 编译
用 Keil MDK 打开 `Project_Modbus.uvprojx` 编译下载。
