# 仓库名：bootloader
## 工程文件
### 包括stm32f072c8t6和stm32f105c8t6两个工程，内含boot和app。
## 开发环境
### stm32f105c8t6 MDK V5.25，Keil.STM32F1xx_DFP.2.1.0，标准库V3.5.0
### stm32f072c8t6 MDK V5.25，Keil.STM32F0xx_DFP.2.0.0，标准库V1.5.0
## 功能说明
### stm32f105c8t6 基于裸机实现bootloader升级程序和app功能程序，bootloader程序负责串口升级交互，进行本地固件下载和指令解析功能。app程序负责串口指令交互和串口转CAN数据透传功能。
### stm32f072c8t6 基于裸机实现bootloader升级程序和app功能程序，bootloader程序负责CAN升级交互，进行本地固件下载和指令解析功能。app程序负责CAN指令交互和业务逻辑
## 备注
### 自定义串口接收协议，分包128字节，支持F105的串口升级和F072的CAN升级。
