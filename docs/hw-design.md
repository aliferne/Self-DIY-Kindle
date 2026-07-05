# 硬件设计说明

## 项目功能及硬件选型方案

该项目为自制 Kindle，本质上是电子阅读器，但为了将其扩展得更有意思和挑战性，我决定实现如下功能：

- 电子阅读器
- 音乐播放器
- 经典游戏的游玩
- 联网功能实现 OTA 和网络文件传输
- 一定的姿态识别功能
- USB + 锂电池的双供电模式

以下是草案（AI 整理，需要审查）：

| 类别      | 型号                   | 数量 | 备注           |
| --------- | ---------------------- | ---- | -------------- |
| 主控      | STM32F411RET6          | 1    | 手头已有       |
| 协处理器  | ESP8266 (ESP-12F)      | 1    | 手头已有或另购 |
| 屏幕      | 4.2寸 IPS SPI (ST7789) | 1    | 带触摸 (GT911) |
| DAC       | PCM5102                | 1    | I2S接口        |
| 充电      | TP4056                 | 1    | 带电池保护     |
| LDO       | RT9013-3.3             | 1    | 600mA输出      |
| PMOS      | AO3401                 | 1    | 电源开关       |
| SPI Flash | W25Q64                 | 1    | 64MB           |
| SD卡座    | 自弹式 microSD         | 1    | -              |
| RTC晶振   | 32.768kHz              | 1    | FC-135或类似   |
| USB串口   | CH340                  | 1    | 含USB-C座      |
| 按键      | 6×6×5mm轻触开关        | 4    | -              |
| 耳机座    | 3.5mm音频座            | 1    | -              |
| USB-C座   | 16pin Type-C           | 1    | -              |

### 主控和协处理器

主控选用 [STM32F411RET6][STM32F411 Datasheet]，当前原型验证阶段使用 STM32F407VET6，协处理器使用 [ESP8266][ESP8266 Datasheet]。

协处理器需要实现联网的需求，因此选择 ESP8266，并要求 ESP8266 可以实现网络透穿，实现通过 Web 端/App 端配对以达到给 STM32 上传文件的需求，此外由于 ESP8266 除此以外并不参与整机逻辑，因此要求它还能实现通过网络端给 STM32 实现 ISP 固件升级。

由此基本确定协处理器 GPIO 占用资源，即 TXD、RXD 一对，两个 GPIO 引脚连接 BOOT0 和 NRST，并在默认情况下外接弱上/下拉电路，避免因电平不稳问题被强行复位

根据 ESP8266 的手册，我们需要的接线逻辑为：

| 引脚 | 功能描述                         | 备注                                     |
| ---- | -------------------------------- | ---------------------------------------- |
| IO0  | 0 = 下载模式， 1 = 悬空/外部拉高 | 建议使用按键                             |
| RST  | 复位                             | 建议使用按键                             |
| EN   | 使能端，高电平有效               | 建议 主控 进行控制，外部电路接弱上拉电阻 |
| TXD0 | 串口发送                         | 接 主控                                  |
| RXD0 | 串口接收                         | 接 主控                                  |

需要注意的是 ESP8266 模组有些 IO 口是不可用的

对于主控，我们需要的接线逻辑则为：

| 引脚  | 功能描述                       | 备注                                           |
| ----- | ------------------------------ | ---------------------------------------------- |
| BOOT0 | 参与 主控 启动时读取内存的配置 | 在接低电平的基础上与 协处理器 的任意 GPIO 相连 |
| BOOT1 | 参与 主控 启动时读取内存的配置 | 需要接低电平                                   |
| TXD0  | 串口发送                       | 接 协处理器                                    |
| RXD0  | 串口接收                       | 接 协处理器                                    |

这对应 STM32 BOOT0 和 BOOT1 配置的启动逻辑：

| 引脚电平         | 对应功能                            |
| ---------------- | ----------------------------------- |
| BOOT0=0, BOOT1=x | 从用户闪存(Flash)启动，正常模式     |
| BOOT0=1, BOOT1=0 | 从系统存储器启动，通常用于 ISP 模式 |
| BOOT0=1, BOOT1=1 | 从内置 SRAM 启动，通常用于调试      |

一般而言是这两个引脚都会被拉到低电平，但是我们对于 BOOT0 有些特殊需求（需要 ISP 功能），因此 BOOT0 不能完全接低电平。
此外注意设计时需要接弱下拉电阻，以避免 GND 的噪声。

对于上面提到的 ISP 方案，可以参考 [STM32RomWebFlasher][STM32RomWebFlasher]

ESP8266 最好不要在一路 UART 上既做与 STM32 的通信，又做自动下载电路的串口，这样子可能会导致硬件上的 data race（想象一下当 ESP8266 准备烧录时 STM32 突然发送消息，固件就乱套了）。

根据[乐鑫官方的资料][Espressif ESP8266 Datasheet]和[这篇博客][ESP8266 UART1]，我们可以在 Arduino 中使用 `Serial.swap()` 来将通信串口换到 IO13(RX) 和 IO15(TX)，但是需要注意 IO15 在上电瞬间必须为低电平，否则上电不正常。
因此可以设计一个电路，该电路包含一个使能接口，当主控给该接口高/低电平时让 IO15 可以通信。

### IMU

### 音频模块

音频模块采用 TI 的 PCM5102A 作为 IC，并采用 I2S 协议进行音频播放上的处理，数据手册见[此处][TI PCM5102A Datasheet & PCB Design]。

该芯片具有如下特性：

- 可借助 BCK 直接内部生成 SCK
- 采用硬件引脚进行配置（DEMP, FLT, FMT）
- PCM 数据样式，I2S/左对齐格式（借助FMT进行配置，FMT=0 => I2S）
- 有省电模式，触发条件只需让 LRCK 和 BCK 无效
- 可利用软件进行静音（XSMT=0 => 软静音）
- 单电源供电（3V3模拟供电，1V8/3V3数字供电）

由于 BCK 可以直接生成 SCK, 我们不再需要额外引出 SCK 作为时钟信号线，可以节省硬件资源，该芯片使用硬件的设置方法详见数据手册的 P5，由于可以直接使用硬件配置，无疑进一步降低了引脚占用数，省电模式的存在使得我们做低功耗设备成为可能。

由数据手册 P26 提供的典型应用实例图，我们真正需要从主控中分配的特殊引脚仅仅只有 DIN, LRCK 和 BCK，它们用做 I2S 外设，而其他配置可以酌情选择 GPIO 进行配置，或直接在电路图中定死配置。

在 P34 的 Layout 界面， TI 的数据手册中甚至特地提及 PCM5102A 可以不做数模地分离，这无疑会简化电路设计，此外 TI 给出的专业设计参考也为自行设计电路提供了一定程度上的简化。

不过最重要的原因还是因为我们有现成物料，且该模块购置方便，仅需不到十元即可获得包括 LDO 和各种参数合适的电容电阻在内的一小块模块，比直接购置芯片还便宜（虽然大概率是国内仿制 IC），而且提供了现成的 PCB 设计方案，且基于该模块的 ESP32 的开源较多，可以作为一定的参考。

因此总的来说我们需要：
BCK, DIN, LRCK, XSMT
它们分别对应主控中的：
I2Sx_CK, I2Sx_SD, I2Sx_WS, 任意 GPIO
需要注意的是 I2Sx_extSD 仅作为扩展使用，而 I2Sx_MCK 可以不接 SCK

## 物料选择及合理性计算

## 芯片引脚分配情况

1. 电源: 12 pins, 其中 VBAT 可以接电池用于给 RTC 供电
2. 调试: 2 pins:
   - SWD: PA13
   - SCK: PA14
3. 显示(SPI3 + PWM): 6/7 pins
   - NSS/CS: PA15
   - SCK: PB3
   - MISO: PB4 (可选, 我决定舍弃)
   - MOSI: PB5
   - DC: PB4
   - RST: PC13
   - BLK: (PWM -> TIM1_CH1): PA8
4. 触摸(I2C1): 4 pins
   - SCL: PB6
   - SDA: PB7
   - INT: PB8
   - RST: PB9
   - 备注：放在芯片正上方
5. 光照模组(I2C1: 可与触摸共用总线, BH1750) 1 pins:
   - SCL: PB6
   - SDA: PB7
   - ADDR: 地址选择，直接接逻辑电平即可
6. IMU(I2C1: 可与触摸共用总线, MPU6050): 3 - 2 = 1 pins
   - SCL: PB6
   - SDA: PB7
   - AD0: 不占用引脚，可接电源或地，但需要注意触摸的 I2C_ADDR, 不能冲突
   - INT: PA12
   - 备注：按照这个引脚分配情况大概是放芯片 48 脚的右侧
7. 音频(I2S1, PCM5102A): 4 pins
   - WS(LRCK): PA4
   - CK(BCK): PA5
   - SD(DIN): PA7
   - XSMT: PA6
8. 控制协处理器(USART1 + GPIO, ESP8266): 4 pins
   - TXD: PA9
   - RXD: PA10
   - EN(使能引脚): PA11
   - ESP_nRST: PC7
   - BOOT0: 不计入消耗
   - ST_nRST: 不计入消耗
9. 按键: 4 pins:
   - HOME(WKUP): PA0
   - PG_UP: PA1
   - PG_DOWN: PA2
   - CONFIRM: PA3
10. LED: 2 pins:
   - 充电指示灯(充电中灯亮，充满灯灭): 接 TP4056 的 CHRG, 不占用主控
   - 用户自定义指示灯（暂未想好做什么）: PC1
11. SD卡(SDIO): 3~6 pins (建议使用 4Bit 模式):
    - CMD: PD2
    - CLK: PC12
    - D0~D3: PC8, PC9, PC10, PC11
    - 备注：按照 LQFP64 的引脚布局，由于数据线和 CLK 啥的不完全在同侧，看来只能让芯片旋转 45 度来画板子了(之后看下如何布局合理)。
12. MSE(主晶振) 和 LSE(RTC): 4 pin
13. SPI Flash(SPI2, 存放字库等):
    - SCK: PB10
    - MISO: PB14
    - MOSI: PB15
    - NSS: PB12

## 参考资料

[AiThinker ESP8266 Datasheet]: https://item.szlcsc.com/datasheet/ESP-12E/90477.html?spm=sc.gbn.xds.a___sc.gbn.hd.ss&c=&lcsc_vid=FAcMUwUFTlUNAl1eR1APV1IHFVEPXgZVQABXV1YAFVQxVlNeRFBaX1dRT1dZUjtW
[Espressif ESP8266 Datasheet]: https://documentation.espressif.com/esp8266-technical_reference_cn.pdf
[STM32RomWebFlasher]: https://github.com/DavidSAlexander/STM32RomWebFlasher
[ESP8266 UART1]: https://www.cnblogs.com/corehouse/p/13770833.html
[STM32F411 Datasheet]: https://atta.szlcsc.com/upload/public/pdf/source/20170908/C94355_1504870172026958155.pdf
[TI PCM5102A Datasheet & PCB Design]: https://www.ti.com.cn/product/cn/PCM5102A#tech-docs
[I2S 总线讲解]: https://juejin.cn/post/7175107789411811384
[SDIO 协议从入门到精通]: https://zhuanlan.zhihu.com/p/689459798
[SD 卡槽布线指导-jlc]: https://wiki.lceda.cn/zh-hans/design-production/pcb-design/moduler-design/sd-card.html
[SD 卡槽布线指导-华秋]: https://zhuanlan.zhihu.com/p/718040692
[单片机复位电路]: https://www.cnblogs.com/bujidao1128/p/18455356
[WCH CH340 自动下载电路方案1]: https://www.wch.cn/products/CH340.html
[WCH CH340 自动下载电路方案1]: https://www.wch.cn/application/575.html
[TP4056 Datasheet]: https://item.szlcsc.com/datasheet/TP4056-MS/8533532.html?spm=sc.gbn.xds.a___sc.gbn.hd.ss&lcsc_vid=FAcMUwUFTlUNAl1eR1APV1IHFVEPXgZVQABXV1YAFVQxVlNeRFFWV1VUT1hdVTsOAxUeFF5JWAIASQYPGQZABAsLWA%3D%3D
