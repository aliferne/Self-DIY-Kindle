与 DeepSeek 的讨论：

https://chat.deepseek.com/a/chat/s/1a2ffe77-d533-4012-b42a-67e33c68c291

与 YTR 的讨论：

> 我感觉我的那台二手kindle操作还是太麻烦了
> 比如说
> 我不能一下跳到桌面
> 或者说最外面
> 然后不能精确定位页码

下面是其中一部分内容

---

# 手搓 Kindle 项目完整需求与技术方案汇总

基于我们所有的讨论，以下是你这个项目的全部需求、技术选型、开发要点和路线图。已按模块整理，便于你后续查阅和执行。

---

## 一、项目核心目标

从头设计并制造一台**功能完整的电子墨水屏阅读器**（Kindle-like），具备：
- 电子书阅读（TXT、EPUB，尽量支持 AZW3）
- MP3 音频播放（顺带功能）
- Wi-Fi 联网（手机配网、无线传书、远程 OTA）
- 蓝牙键盘支持（实现终端交互）
- 触摸屏（可选，提升体验）
- 低功耗设计（待机数周）
- 全自制：画 4 层 PCB、3D 打印外壳、编写分层固件

---

## 二、硬件选型（最终 PCB 方案）

### 主控 MCU
- **型号**：STM32F411CEU6（QFN48，512KB Flash，128KB RAM，100MHz）
- **原因**：128KB RAM 足够跑 LVGL+FreeRTOS+EPUB 解析，封装小，适合便携。
- **备选**：手头也有 F411RET6，但 CEU6 更小，优先用 CEU6。
- **开发工具**：STM32CubeMX + HAL 库 + ARM GCC / Keil

### 协处理器（联网 + 蓝牙）
- **型号**：ESP32-C3（或 S3，根据需要）
- **原因**：支持 Wi-Fi 和 BLE，功耗较低，UART 与 STM32 通信，运行 ESP-AT 固件。
- **如需更强性能**：可换 ESP32-S3，但增加功耗。

### 电子纸显示屏
- **型号**：微雪 6寸 800×600 电子纸模块（SPI 接口，自带波形文件及电平转换）
- **备选**：4.2寸 400×300（更便宜）
- **驱动**：SPI + 几个 GPIO（CS, DC, RST, BUSY）

### 存储
- **主存储**：microSD 卡（SDIO 模式，速度快）
- **外部 Flash**：W25Q64（8MB，存放字库、配置、缓存）

### 音频输出
- **DAC**：PCM5102（I2S 接口，直接输出至耳机）
- **无需功放**：仅耳机输出，不需要喇叭
- **需注意**：PCB 上做数模地分离（磁珠单点接地）

### 输入设备
- **物理按键**：至少 5 个（翻页上/下、主页、返回、确认），其中两个侧边安放。
- **触摸屏**：选配 6寸电容触摸（GT911，I2C 接口），通过闲鱼购买二手带触摸的电子纸模块（成本 40-70 元）。
- **蓝牙键盘**：通过 ESP32-C3 的 BLE 接收 HID 输入，ST 只需解析键值。

### 电源管理
- **电池**：旧手机锂电池（1000-2000mAh），利用现有。
- **充电**：TP4056 + USB-C 口（支持 5V 输入）
- **系统电源**：LDO ME6211/RT9013（3.3V）
- **电源开关**：PMOS（AO3401）+ 按键实现长按开机/软件关机，彻底切断电池输出。
- **电量检测**：ADC 采样（电阻分压） + 可选 MAX17048 电量计
- **低功耗**：STM32 STOP 模式（~10µA），ESP32 深度睡眠或断电

### RTC
- **方案**：STM32F411 内部 RTC + 外接 32.768kHz 晶振（FC-135 + 10pF 电容）
- **功能**：显示时间、书籍阅读时间戳、低功耗定时唤醒

### 其他接口
- **USB**：USB-C 口用于充电，同时连接 CH340C 做调试串口（或直接用 ST-Link 的虚拟串口）
- **调试接口**：SWD（4pin）+ 串口调试输出

### PCB 设计
- **层数**：4 层（完整地平面，改善信号完整性）
- **天线**：ESP32 板载天线区域下方所有层挖空，靠近 PCB 边缘
- **音频**：模拟地与数字地用磁珠单点连接
- **测试点**：预留 VBAT, 3.3V, GND, SWD, 关键信号

---

## 三、软件架构（分层设计，模仿 Linux 驱动模型）

```
Application (App)
    │
    ▼
Services (服务层)
    │
    ▼
Middleware (中间件：FreeRTOS, LVGL, FatFS, libepub)
    │
    ▼
Drivers (驱动层：抽象设备接口)
    │
    ▼
BSP (板级支持包：HAL/寄存器封装 + 硬件配置头文件)
```

### 关键驱动抽象接口（类似 Linux file_operations）

每个外设都定义统一操作集，应用只通过全局设备指针调用：

- **显示设备** `display_device_t`：init, clear, draw_pixel, draw_area, refresh, sleep
- **输入设备** `input_device_t`：init, get_event, set_callback
- **存储设备** `storage_device_t`：init, read_blocks, write_blocks, get_block_count
- **RTC 设备** `rtc_device_t`：init, set_time, get_time, set_alarm, disable_alarm, enter_low_power_wakeup
- **音频设备** `audio_device_t`：init, play, stop, set_volume
- **联网设备** `net_device_t`：init, connect, send, recv, ota_update

### BSP 层硬件资源配置

所有引脚、外设编号集中在 `bsp/hardware_config.h`，驱动代码只使用宏。换芯片只需修改该文件和 BSP 底层实现。

### 任务划分（FreeRTOS）

| 任务名 | 优先级 | 职责 |
|--------|--------|------|
| UI 任务 | 高 | 运行 LVGL `lv_timer_handler()`，刷新屏幕 |
| 数据处理任务 | 中 | EPUB 解析、文件读取、音频解码（非实时） |
| 音频任务 | 高 | I2S DMA 填充，MP3 解码实时输出 |
| 输入任务 | 中 | 轮询按键/触摸，通过队列发送事件 |
| 联网任务 | 低 | 处理 ESP32 串口通信，Wi-Fi/蓝牙事件 |

---

## 四、功能模块详细需求

### 1. 电子书阅读
- 支持格式：TXT（纯文本）、EPUB（ZIP+XML+HTML）
- 解析方法：miniz 解压 + TinyXML2 解析 OPF/NCX，提取文本及样式
- 排版：LVGL 作为 GUI，FreeType 渲染字体（或预置字库）
- 进度保存：记录当前书籍路径、页码、章节位置到 Flash
- 书架：扫描 SD 卡中的电子书，显示列表

### 2. 音频播放
- 格式：MP3（软件解码，libhelix-mp3）
- 输出：I2S → PCM5102 → 耳机
- 控制：上一曲/下一曲/暂停（通过按键或触摸屏）

### 3. 联网与无线传书
- **硬件**：ESP32-C3 通过 UART 连接 STM32，运行 ESP-AT 固件（或自定义固件）
- **功能**：
  - 手机蓝牙配网（BluFi）
  - 手机 App（Flutter 开发）通过 Wi-Fi 局域网传输电子书文件到设备
  - 设备在 SD 卡中保存接收到的文件
  - 可选：ESP32 运行 HTTP 服务器，电脑浏览器上传
- **远程 OTA**：
  - ESP32 从云端下载 STM32 固件，通过串口发送给 STM32
  - STM32 写入内部 Flash 并重启

### 4. 蓝牙键盘与终端
- **原理**：ESP32 作为 BLE 从设备接收键盘 HID 报告，通过 UART 发给 STM32
- **STM32 侧**：实现 HID 解析器，将键值注入 LVGL 输入系统
- **终端效果**：在屏幕上模拟命令行，执行系统命令（如 `ls`, `reboot`, `upload` 等）

### 5. 触摸屏（可选）
- 驱动 GT911（I2C 地址 0x5D）
- 通过中断或轮询读取触摸坐标
- 接入 LVGL 的 `lv_port_indev.c`，实现点击、滑动

### 6. RTC 与低功耗
- 显示当前时间（可同步网络 NTP）
- 记录每本书最后阅读时间
- 低功耗模式：
  - 用户无操作 5 分钟后进入深度睡眠
  - RTC 定时唤醒（如每隔 10 秒检查一次按键）
  - 按键中断唤醒
  - 唤醒后恢复上下文，刷新屏幕

### 7. 物理按键
- 至少 5 个：上一页、下一页、主页、返回、确认/菜单
- 两个侧边按键（用于翻页）
- 使用 GPIO 中断 + 去抖动（软件滤波）

---

## 五、开发路线图（分阶段执行）

### 第一阶段：原型验证（用现成开发板，约 3 周）
- 工具：STM32F407 开发板 + ESP32-C3 开发板 + 电子纸模块 + 面包板
- 目标：
  - 点亮电子纸，显示图片/文字
  - 移植 FatFS，读取 SD 卡中的 TXT 并显示
  - 移植 LVGL 到电子纸（SPI 接口）
  - ESP32 与 STM32 UART 通信，发送 AT 指令连接 Wi-Fi
  - 串口打印 RTC 时间（外接晶振焊接在洞洞板上）

### 第二阶段：原理图与 PCB 设计（约 2-3 周）
- 根据最终选型（F411CEU6 + ESP32-C3）绘制原理图（KiCad/立创EDA）
- 完成 4 层 PCB 布局布线（注意电子纸 FPC 连接器、天线净空、音频地分离）
- 导出 Gerber，在嘉立创打样

### 第三阶段：固件移植与调试（约 4 周）
- 焊接样板，烧录 bootloader
- 将原型代码移植到新 PCB，调试各外设：
  - 电子纸、SDIO、SPI Flash、按键、I2S、RTC
  - ESP32 通信、蓝牙键盘 HID 解析
- 整合 FreeRTOS 任务，实现基本阅读器功能

### 第四阶段：高级功能集成（约 4 周）
- 实现 EPUB 解析（miniz + TinyXML2）
- LVGL 界面美化（书架、设置、时间显示）
- 联网传书（手机 App 开发）
- 远程 OTA 流程
- 低功耗模式调试（测量电流）

### 第五阶段：外壳设计与整机装配（约 2 周）
- 用 SolidWorks 设计 3D 外壳（分前壳、中框、后盖）
- PLA 3D 打印，打磨组装
- 最终测试续航、手感、散热

---

## 六、关键设计模式与代码规范

### 1. 设备驱动抽象（Linux 风格）
```c
struct display_device {
    void (*init)(void);
    void (*draw_pixel)(int x, int y, uint16_t color);
    void (*fill_rect)(int x, int y, int w, int h, uint16_t color);
    void (*refresh)(void);
    void (*sleep)(void);
};

extern struct display_device *display_dev;
```

### 2. 硬件配置集中管理
```c
// bsp/hardware_config.h
#define EPAPER_SPI        SPI1
#define EPAPER_CS_PIN     GPIO_PIN_4
#define EPAPER_CS_PORT    GPIOB
#define EPAPER_DC_PIN     GPIO_PIN_5
...
```

### 3. 服务层 API 隐藏硬件细节
```c
// services/display.h
void display_clear(void);
void display_draw_text(const char *text, int x, int y);
void display_refresh(void);
```

### 4. 低功耗状态机
```c
typedef enum {
    POWER_ACTIVE,
    POWER_IDLE_TIMEOUT,
    POWER_DEEP_SLEEP_REQUEST,
    POWER_DEEP_SLEEP,
    POWER_WAKEUP
} power_state_t;
```

---

## 七、需要补充或特别注意的点

- **天线净空**：ESP32 的板载天线下方所有层必须挖空，周围至少 15mm 内无金属、无铺铜。
- **音频干扰**：I2S 信号线尽量短，用地线包裹；PCM5102 的 AVDD 和 DVDD 分开供电或加磁珠。
- **SDIO 布线**：4 条数据线等长，时钟线独立，上拉电阻 10k~47k。
- **RTC 晶振**：负载电容按晶振手册选择（通常 6pF~12pF），紧贴芯片。
- **电源去耦**：每个 IC 的电源引脚附近放置 0.1µF + 10µF 电容。
- **测试点**：务必留出 VBAT, 3.3V, GND, SWDIO, SWCLK, 以及关键 GPIO 测试点。
- **程序稳健性**：所有外设初始化应有超时和错误处理，避免死循环。

---

## 八、可参考的开源项目

- **InkPlate 系列**：ESP32 电子纸项目，硬件开源。
- **Papyrix Reader**：ESP32 上 EPUB 阅读器，代码结构清晰。
- **ZEReader**：Zephyr RTOS 实现的阅读器。
- **立创开源平台**：搜索“墨水屏 阅读器 STM32”参考原理图和 PCB。

---

## 九、总结与下一步行动

你已经完成了从需求到技术的全部构思。接下来的具体行动建议：

1. **立即开始原型验证**：用 F407 开发板点亮电子纸和 SD 卡，跑通 LVGL 显示“Hello Kindle”。
2. **同步学习 PCB 设计**：安装立创EDA，参考开源项目绘制 F411CEU6 最小系统板。
3. **购买缺少的物料**：32.768kHz 晶振、PCM5102 模块、ESP32-C3 开发板（如果手头没有）。
4. **建立代码仓库**：按分层架构建立文件夹结构，从 BSP 开始逐步实现。
