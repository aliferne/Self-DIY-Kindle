# Self-DIY-Kindle 实现计划

> 基于 2026-06-11 代码审查的完整评估与路线图

---

## 一、项目状态总览

**总体完成度：约 20-25%**

阶段性评估：项目处于**第一阶段（原型验证）中期**。5 层架构（chip → BSP → Middlewares → Services → Applications）已搭建完整，BSP 抽象层和芯片层实现扎实。但所有面向用户的功能（阅读器、GUI、联网、音频）均处于空壳状态。

### 各层完成度速览

```
Applications:  ░░░░░░░░░░  10%  (5 个任务中 4 个是空循环)
Services:     ░░░░░░░░░░  15%  (storage 完整, input 完整, 其余空壳)
Middlewares:  ██████░░░░  60%  (LVGL/e-paper/FatFs/miniz 就绪, 端口未全部启用)
BSP:          ██████████  95%  (GPIO/SPI/I2C/SDIO/UART/DWT 全完整)
Chip:         ██████████  95%  (所有芯片实现文件完整)
```

---

## 二、已完成模块清单

### ✅ BSP 层（板级支持包）
| 模块 | 文件 | 说明 |
|------|------|------|
| GPIO 抽象 | `bsp_gpio.h` | `void *` 句柄封装，不暴露 HAL 类型 |
| SPI 抽象 | `bsp_spi.h/.c` | 硬件 SPI + 软件 SPI (bit-bang) 双模式，Mode 0-3 |
| I2C 抽象 | `bsp_i2c.h/.c` | 软件 I2C (bit-bang)，强制开漏 |
| SDIO 抽象 | `bsp_sdio.h` | Polling/DMA/IT 三模式，已集成 FatFs |
| UART 抽象 | `bsp_uart.h` | init/send/recv/irq 接口完整 |
| DWT 微秒延时 | `bsp_sys.h` | 基于 DWT CYCCNT，不占用硬件定时器 |
| RTC (空壳) | `bsp_rtc.h` | 仅 `#pragma once`，无实现 |
| I2S (空壳) | `bsp_i2s.h` | 仅 `#pragma once`，无实现 |

### ✅ Chip 层（STM32F4 实现）
| 模块 | 说明 |
|------|------|
| `gpio_chip.c` | GPIO init/deinit/write/read/toggle/attach_irq/detach_irq 完整 |
| `spi_chip.c` | 硬件 SPI 收发完整 |
| `i2c_chip.c` | 软件 I2C bit-bang init/deinit 完整 |
| `sdio_chip.c` | SDIO 初始化 + FatFs disk I/O 接入 |
| `uart_chip.c` | UART init/send/recv 完整 |
| `i2s_chip.c` | **空文件** |
| `sys_chip.c` | 系统初始化、printf 重定向、DWT 延时、os_delay 封装 |
| `irq_chip.h` | GPIO EXTI ISR 分发表 |
| `pin_src.h` | 所有引脚宏定义集中管理（按键、LED、TFT、ePaper） |

### ✅ 中件间层
| 模块 | 说明 |
|------|------|
| TFT (ST7735S) 驱动 | 含中文显示、基本绘图 API |
| e-Paper (4.2" V2) 驱动 | 支持 Normal/Fast/4Gray 全程初始化、局部刷新、休眠 |
| 统一显示屏驱动 `disp_drv.h` | `display_init/backlight/set_region/write_pixels/draw_xxx` 全 API |
| `mid_config.c` | 全局 `Disp_Drv_t display` 实例，`mid_init_tft()` 已启用 |
| ePaper 初始化函数 | `mid_init_epaper()` 已实现但**注释**（使用 TFT 中） |
| FreeRTOS 集成 | CMSIS-RTOS 封装，6 个任务已创建 |
| FatFs 文件系统 | SDIO 接入，SD 卡读写可用 |
| Miniz (ZIP 解压) | 库文件就绪，未接入服务 |
| LVGL 配置 | `lv_conf.h` 已启用大部分 widgets |
| **LVGL 显示端口 `lv_port_disp.c`** | **`#if 1`，`disp_flush()` 已调通 `display_write_pixels()`** |
| **LVGL 输入端口 `lv_port_indev.c`** | **`#if 0`，未启用** |
| **LVGL 文件系统端口 `lv_port_fs.c`** | **`#if 0`，保持禁用**（直接使用 `storage_srv`，不经过 LVGL） |

### ✅ 服务层
| 模块 | 说明 |
|------|------|
| `srv_config.c` | 全局 `Storage_t sdcard` 实例，`service_init()` 创建 4 个 SD 目录 |
| `storage_srv.h/.c` | **完整**：路径前缀、目录自动创建、写后 `f_sync` 防掉电、`open/stat/unlink/rename/opendir` |
| `input_srv.h/.c` | **完整**：按键 irq_flag 轮询、5 按键 ID 定义、事件结构体定义 |

### ✅ 应用层
| 模块 | 说明 |
|------|------|
| 6 个 FreeRTOS 任务 | 已创建（UI/Input/Process/Net/Music/Default） |
| `main.c` | 初始化流程正确：HAL → clock → MX → chip_init → bsp_init → mid_init → service_init → FreeRTOS |

---

## 三、未完成模块详细评估

### 🔴 P0 — 核心阅读器功能（必须首先完成）

| # | 功能 | 文件 | 当前状态 | 代码量 | 依赖 |
|---|------|------|---------|--------|------|
| **0-1** | LVGL 输入端口启用 | `lv_port_indev.c` | `#if 0`，`button_read()` 空实现 | ~30 行 | — |
| **0-2** | Input 任务激活 | `input_task.c` | `handle_btn_event()` 被注释 | ~5 行 | P0-1 |
| **0-3** | UI 任务完善 | `ui_task.c` | 缺少 indev_init，刷新率 500ms 太慢 | ~10 行 | P0-1 |
| **0-4** | EPUB/TXT 解析 | `ebook_srv.h/.c` | `.h` 空，`.c` 无用 | ~400 行 | P0-5 (storage_srv) |
| **0-5** | Process 任务管道 | `process_task.c`, `freertos.c` | 空循环，无消息队列 | ~200 行 | P0-4 |
| **0-6** | UI 阅读器界面 | `ui_task.c` | 仅 "Confirm" 演示 | ~300 行 | P0-3, P0-5 |

> **P0 总计**：约 **1000 行代码**，工期预估 **2~3 周**

### 🟡 P1 — 必备体验功能（阅读器可用）

| # | 功能 | 文件 | 当前状态 | 代码量 | 依赖 |
|---|------|------|---------|--------|------|
| **1-1** | 书架屏幕 | `ui_task.c` | 不存在 | ~200 行 | P0-3, P0-5 |
| **1-2** | 阅读进度保存 | `process_task.c` / `ebook_srv.c` | 不存在 | ~50 行 | P0-4, P0-5 |
| **1-3** | ePaper 切换 | `mid_config.c`, `lv_port_disp.c` | 驱动就绪，未切换 | ~100 行 | P0-3 |
| **1-4** | 显示服务层 | `display_srv.h/.c` **新建** | 不存在 | ~80 行 | — |

> **P1 总计**：约 **400 行代码**，工期预估 **2~3 周**

### 🟢 P2 — 联网与音频

| # | 功能 | 文件 | 当前状态 | 代码量 | 依赖 |
|---|------|------|---------|--------|------|
| **2-1** | I2S 抽象实现 | `bsp_i2s.h`, `i2s_chip.c` | 空 | ~150 行 | — |
| **2-2** | MP3 播放 | `music_srv.h/.c`, `music_task.c` | 空 | ~500 行 | P2-1 |
| **2-3** | ESP32 通信协议 | `net_srv.h/.c` | 空 | ~500 行 | — |
| **2-4** | Wi-Fi 传书 + OTA | `net_task.c` | 空循环 | ~400 行 | P2-3 |

> **P2 总计**：约 **1500 行代码**，工期预估 **3~4 周**

### 🔵 P3 — 高级功能

| # | 功能 | 文件 | 代码量 | 依赖 |
|---|------|------|--------|------|
| **3-1** | RTC 实现 | `bsp_rtc.h/.c`, `time_srv.c` | ~200 行 | — |
| **3-2** | GT911 触摸驱动 | `Middlewares/gt911/` **新建** | ~200 行 | P0-3 |
| **3-3** | 触摸 indev 桥接 | `lv_port_indev.c` | ~50 行 | P3-2 |
| **3-4** | BLE 键盘 HID | `net_srv.c` 扩展 | ~200 行 | P2-3 |
| **3-5** | 低功耗模式 | `applications/lowpower.c` **新建** | ~300 行 | P3-1 |
| **3-6** | 电量检测 | ADC / MAX17048 | ~100 行 | — |

> **P3 总计**：约 **1000 行代码**，工期预估 **2~3 周**

### ⚫ P4 — 硬件设计（可并行）

| 阶段 | 内容 | 工期 |
|------|------|------|
| F411CEU6 + ESP32-C3 原理图 | 最小系统 + e-Paper FPC + SDIO + I2S + RTC + USB-C | 1 周 |
| 4 层 PCB | 天线净空、SDIO 等长、数模地分离、测试点 | 1~2 周 |
| 打样焊接调试 | 嘉立创 → 手工焊接 → 各外设调试 | 1~2 周 |
| 外壳 | SolidWorks → PLA 3D 打印 → 打磨组装 | 2 周 |

---

## 四、详细实施路线图

### Step 0-1：激活 LVGL 输入端口

**目标**：物理按键按下后，LVGL 能收到导航事件，焦点可移动。

**涉及文件**：`Middlewares/lvgl/porting/lv_port_indev.c`

```
按键按下 → GPIO ISR 置 irq_flag
              → InputTask 轮询调用 handle_input_events()
                   → btn_event 被填充
                        → lv_port_indev.c 的 button_read() 读取 btn_event
                             → 映射到 LV_KEY_NEXT/PREV/ENTER/HOME/ESC
                                  → LVGL 处理焦点移动 / 按钮激活
```

**具体修改**：

```c
// lv_port_indev.c — 将 button_read() 桥接到 input_srv

#include "input_srv.h"

static void button_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    /* 轮询按键事件 */
    InputEvent_t evt;
    evt.type = InputEventType_BtnPress;
    evt.data.btn_press.btn_id = Btn_ID_None;
    handle_input_events(&evt);

    switch (evt.data.btn_press.btn_id) {
        case Btn_ID_PageUp:
            data->key = LV_KEY_PREV;
            break;
        case Btn_ID_PageDown:
            data->key = LV_KEY_NEXT;
            break;
        case Btn_ID_Confirm:
            data->key = LV_KEY_ENTER;
            break;
        case Btn_ID_Home:
            data->key = LV_KEY_HOME;
            break;
        case Btn_ID_Back:
            data->key = LV_KEY_ESC;
            break;
        default:
            data->state = LV_INDEV_STATE_REL;
            return;
    }
    data->state = LV_INDEV_STATE_PR;
}
```

**验证**：TFT 上创建一个 `lv_btn` 组，按翻页键移动焦点，按确认键触发。

---

### Step 0-2：激活 Input 任务

**涉及文件**：`applications/input_task.c`

```c
void StartInputTask(void const *argument)
{
    for (;;) {
        handle_btn_event();     // 取消此行的注释
        os_delay_ms(10);        // 10ms 轮询周期（去抖效果）
    }
}
```

---

### Step 0-3：完善 UI 任务

**涉及文件**：`applications/ui_task.c`

```c
void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();       // 新增此行

    for (;;) {
        lv_timer_handler();
        os_delay_ms(5);         // 从 500ms 改为 5ms
    }
}
```

> **注意**：`lv_port_fs_init()` **不调用**。所有文件 IO 通过 `storage_srv` 在 Process 任务中完成，不走 LVGL 文件系统。

---

### Step 0-4：EPUB/TXT 解析服务

**涉及文件**：`services/ebook_srv.h`, `services/ebook_srv.c`, `Middlewares/miniz/`

**架构决策 — 分层设计**：

```
eBookSrv 作为纯数据处理层，不关心显示和输入：
  - 打开文件 (storage_srv)
  - 解压 ZIP (miniz)
  - 解析 OPF/NCX/XHTML (手写 XML 解析器或 sxmlc)
  - 按行/按字分页
  - 缓存当前页文本到内存
  - 提供 API：open(), get_page(), get_total_pages(), close()
```

**`ebook_srv.h` 接口设计**：

```c
typedef enum {
    EBook_Format_TXT,
    EBook_Format_EPUB,
    EBook_Format_Unknown,
} EBook_Format_t;

typedef struct {
    char path[128];
    EBook_Format_t format;
    uint16_t total_pages;
    uint16_t current_page;
    uint16_t lines_per_page;
    /* ... 内部状态 */
} EBookDoc_t;

EBook_Err_t ebook_open(EBookDoc_t *doc, const char *path);
void ebook_close(EBookDoc_t *doc);
EBook_Err_t ebook_get_page(EBookDoc_t *doc, uint16_t page, char *out, uint16_t out_sz);
EBook_Err_t ebook_next_page(EBookDoc_t *doc);
EBook_Err_t ebook_prev_page(EBookDoc_t *doc);
```

**TXT 解析**：
```
storage_open() → storage_read() → 以 \n 分行 → 计算总行数 / lines_per_page → 按页索引
```

**EPUB 解析**：
```
miniz 解压 ZIP → 读 container.xml 得 OPF 路径 → 读 OPF 得 spine/itemref → 按顺序读 XHTML → strip HTML tags → 分行 → 分页
```

> EPUB 解析的 XML 部分可以用一个极轻量的 XML 解析器（如 sxmlc，~500 行纯 C），不需要引入 TinyXML2。

---

### Step 0-5：Process 任务 + 消息队列

**涉及文件**：`applications/process_task.h`, `applications/process_task.c`, `Core/Src/freertos.c`

**消息队列设计**（在 `freertos.c` 中定义）：

```c
// 消息类型
typedef enum {
    Msg_Cmd_OpenBook,
    Msg_Cmd_NextPage,
    Msg_Cmd_PrevPage,
    Msg_Cmd_GoToPage,
    Msg_Cmd_JumpToBookShelf,
} ProcessCmd_t;

typedef struct {
    ProcessCmd_t cmd;
    union {
        char book_path[64];
        uint16_t page_num;
    } data;
} ProcessMsg_t;

// 返回给 UI 的消息
typedef enum {
    Msg_UI_ShowText,
    Msg_UI_ShowBookShelf,
    Msg_UI_UpdateProgress,
} UIMsgType_t;

typedef struct {
    UIMsgType_t type;
    char text[2048];      // 一页文本内容
    uint16_t current_page;
    uint16_t total_pages;
    char title[64];
} UIMsg_t;
```

**消息队列使用 `osMessagePut`/`osMessageGet`**（在 `freertos.c` 中用 `osMessageDef` 创建两条消息队列）：

```
Input Task ──[翻页命令]──→ Process Task
Process Task ──[文本内容]──→ UI Task
```

**Process 任务主循环**：

```c
void StartProcessTask(void const *argument)
{
    for (;;) {
        // 等待 Input 任务的命令
        osMessageGet(process_queue, osWaitForever, &msg);

        switch (msg.cmd) {
            case Msg_Cmd_OpenBook:
                ebook_open(&doc, msg.data.book_path);
                ebook_get_page(&doc, 1, text_buf);
                send_text_to_ui(text_buf);
                break;
            case Msg_Cmd_NextPage:
                ebook_next_page(&doc);
                ebook_get_current_page(&doc, text_buf);
                send_text_to_ui(text_buf);
                break;
            // ...
        }
        save_progress(&doc);  // 每翻页保存进度
    }
}
```

> **关键架构原则**：`ebook_srv` 和 `Process` 任务不依赖 LVGL。UI 任务只负责"收到文本 → 显示"。这样拆分使得：
> - 以后换 GUI 框架（甚至不用 LVGL）时不需要改动解析逻辑
> - Process 任务可以做耗时解析而不阻塞 UI 刷新

---

### Step 0-6：UI 阅读器界面

**涉及文件**：`applications/ui_task.c`

**界面设计**—— 分两个"屏幕"，通过 `lv_scr_load()` 切换：

| 屏幕 | LVGL 控件 | 数据来源 |
|------|-----------|---------|
| **书架** `shelf_scr` | `lv_list` + `lv_btn` | Process 任务经消息队列发送文件列表 |
| **阅读器** `reader_scr` | `lv_label` + `lv_label` (顶部书名/页码) | Process 任务经消息队列发送文本 |

```
书架屏幕                      阅读器屏幕
┌──────────────────┐          ┌──────────────────┐
│  书架              │          │  三体 · P23/498   │
│  ┌──────────────┐ │          │                  │
│  │ 📖 三体       │ │          │  汪淼倒吸了一口   │
│  │ 📖 百年孤独   │ │          │  凉气...         │
│  │ 📖 活着       │ │          │                  │
│  └──────────────┘ │          │                  │
│                  │          │   ██░░░░ 5%      │
│  [打开] [删除]   │          │                  │
└──────────────────┘          └──────────────────┘
```

---

### Step 1-1 ~ 1-2：书架 + 进度保存

在 Step 0 的架构上增量实现：

- 书架：Process 任务用 `storage_opendir("/books")` → `storage_readdir()` 遍历得到文件名列表 → 通过消息队列发给 UI
- 进度保存：每翻页在 Process 任务中以 JSON 格式写入 `storage_open("/books/.progress")`，格式如 `{"path":"0:/books/三体.epub", "page":23}`

---

### Step 1-3：ePaper 切换

**涉及文件**：
- `Middlewares/mid_config.c`：注释 `mid_init_tft()`，取消 `mid_init_epaper()` 注释
- `Middlewares/lvgl/porting/lv_port_disp.c`：ePaper 适配

**ePaper 的 LVGL 适配策略**：

```c
// lv_conf.h
#define LV_COLOR_DEPTH     1       // 黑白墨水屏
#define LV_USE_ANIMATION   0       // 墨水屏不需要动画

// lv_port_disp.c — 使用全屏缓冲区 + 全刷
#define MY_DISP_HOR_RES 800
#define MY_DISP_VER_RES 600

static lv_disp_draw_buf_t draw_buf_dsc;
static lv_color_t buf_1[MY_DISP_HOR_RES * MY_DISP_VER_RES]; // 全屏缓冲

// disp_flush() 中：
//   1. 将 LVGL 的 lv_color_t 缓冲转换为 ePaper 的像素格式（1bit）
//   2. 调用 ePaper 驱动全屏刷新
//   3. 全刷后调用 lv_disp_flush_ready()
```

> **注意**：ePaper 显示涉及 SPI 时序，全屏刷新约 1~2 秒，翻页时需要考虑用户体验（如 loading 动画或"正在刷新"提示）。后续可以优化为局部刷新（仅刷变化的区域）。

---

### Step 2-1 ~ 2-2：I2S + MP3 音频

**I2S 实现**：
```
STM32F407 I2S3 (SPI3 复用) → PCM5102 模块:
  - SCK   = PB3  (I2S_CK)
  - SD    = PB5  (I2S_SD)
  - WS    = PA4  (I2S_WS)
  - MCK   = PC7  (I2S_MCK) — PCM5102 需要主时钟
  - I2S 使用 DMA 双缓冲模式
```

**MP3 解码**：使用 **libhelix-mp3**（已包含在 `Middlewares/Third_Party/`？需要确认），或者使用 **libmad** 或 **dr_mp3**（单头文件库，更轻量，推荐）。

```c
// dr_mp3.h — 单头文件 MP3 解码器
// https://github.com/mackron/dr_libs/blob/master/dr_mp3.h

void music_task_play(const char *path)
{
    // 1. storage_open + storage_read 加载 MP3 文件到内存
    // 2. drmp3_init_memory() 解码
    // 3. I2S DMA 双缓冲播放
    // 4. 解码回调填充 DMA 缓冲
}
```

---

### Step 2-3 ~ 2-4：ESP32 联网

**ESP32 通信协议（AT 指令 vs 自定义）**：

建议使用 **ESP-AT 固件**，STM32 侧通过 UART 发送 AT 指令：

```
AT+CWJAP="SSID","PASSWORD"     // 连接 Wi-Fi
AT+CIPSTART="TCP","192.168.x.x",8080  // 建立 TCP 连接
AT+CIPSEND=<len>               // 发送数据
+IPD,<len>:<data>              // 接收数据
```

**Wi-Fi 配网流程**：
```
1. ESP32 启动 SoftAP ("Kindle-Setup")
2. 手机连此 Wi-Fi，打开浏览器/App 配置路由 Wi-Fi
3. ESP32 连接路由 Wi-Fi，启动 TCP Server
4. 手机传书：HTTP PUT /books/三体.epub
5. STM32 通过 UART 接收数据 → storage_write 到 SD 卡
```

---

### Step 5 及以后：P3 + P4

按前文 P3/P4 表执行。RTC、触摸、低功耗、BLE 键盘、PCB 设计依次推进。

---

## 五、架构全景：任务间通信

```
                    ┌──────────────────────┐
                    │     Interrupts        │
                    │  (GPIO EXTI ISR)      │
                    │  仅置 irq_flag        │
                    └──────────┬────────────┘
                               │  irq_flag
                               ▼
  ┌─────────────────────────────────────────────────────┐
  │                  Input Task (中优先级)               │
  │  轮询 irq_flag → handle_input_events() → 命令       │
  └─────────┬───────────────────────────────────────────┘
            │  osMessagePut (命令: 翻页/打开/退出)
            ▼
  ┌─────────────────────────────────────────────────────┐
  │                Process Task (中优先级)                │
  │  接收命令 → ebook_srv 解析 → storage_srv 读文件      │
  │  分页缓存 → 保存进度                                │
  └─────────┬───────────────────────────────────────────┘
            │  osMessagePut (数据: 文本内容/文件列表)
            ▼
  ┌─────────────────────────────────────────────────────┐
  │                  UI Task (高优先级)                   │
  │  lv_timer_handler() → LVGL GUI                     │
  │  lv_label_set_text() / lv_scr_load()               │
  └─────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────┐
  │                  Net Task (低优先级)                   │
  │  轮询 ESP32 UART → 文件传输/OTA                    │
  └─────────────────────────────────────────────────────┘

  ┌─────────────────────────────────────────────────────┐
  │                  Music Task (中优先级)                │
  │  I2S DMA 填充 → 播放控制队列                        │
  └─────────────────────────────────────────────────────┘
```

**关键原则**：
1. **ISR 不做业务逻辑** — 仅置位 `irq_flag`，不会导致优先级反转
2. **Input 任务专有** — 不直接调用 UI 或 Process，只发消息
3. **Process 任务是"大后方"** — 所有耗时操作（解析、文件 IO）在此进行，不阻塞 UI
4. **UI 任务只负责显示** — 不直接调用 `storage_srv` 或 `ebook_srv`，只接受消息队列的文本
5. **不使用 LVGL 文件系统** — 所有文件操作通过 `storage_srv` 在 Process 任务中完成

---

## 六、依赖关系图

```
  ┌──── Step 0-1 (lv_port_indev.c) ── 30 行
  │      └── 依赖: input_srv, bsp_config
  │
  ├──── Step 0-2 (input_task.c) ── 5 行
  │
  ├──── Step 0-3 (ui_task.c) ── 10 行
  │      └── 调用 lv_port_indev_init()
  │
  ├──── Step 0-4 (ebook_srv.h/.c) ── 400 行
  │      └── 依赖: storage_srv, miniz
  │
  ├──── Step 0-5 (process_task + msg queue) ── 200 行
  │      └── 依赖: ebook_srv, freertos.c (queue 定义)
  │
  └──── Step 0-6 (UI reader views) ── 300 行
         └── 依赖: process_task (消息队列)

                    ▼
           ┌────────┴────────┐
           ▼                 ▼
     Step 1-1 (书架)   Step 1-2 (进度保存)
           │                 │
           ▼                 │
     Step 1-3 (ePaper)  ────┘
           │
           ▼
     Step 1-4 (display_srv)

    ┌───────────┴───────────┐
    ▼                       ▼
Step 2-1 (I2S)        Step 2-3 (ESP32 通信)
    ▼                       ▼
Step 2-2 (MP3)        Step 2-4 (Wi-Fi + OTA)

    ┌───────────┴───────────┐
    ▼                       ▼
Step 3-1 (RTC)        Step 3-2 (触摸)
    ▼                       ▼
Step 3-5 (低功耗)      Step 3-3 (触摸 indev)
                         Step 3-4 (BLE 键盘)
```

---

## 七、各步骤验证标准

| 步骤 | 验证标准 |
|------|---------|
| **0-1 ~ 0-3** | TFT 显示 LVGL 界面，按翻页键焦点移动，按确认键触发按钮 |
| **0-4 ~ 0-5** | SD 卡中 `.txt` 文件打开后文本显示在 TFT 上，翻页键控制前后翻页 |
| **0-6** | 上电看到书架列表，点击书籍进入阅读模式，顶部显示书名/页码 |
| **1-1** | 书架列表从 SD 卡动态加载，支持上下滚动选择 |
| **1-2** | 关闭电源重新打开后，回到上次阅读位置和页码 |
| **1-3** | ePaper 显示内容，翻页刷新干净无残影，无 LVGL 动画残留 |
| **2-1 ~ 2-2** | PCM5102 耳机输出 MP3 音乐，音量可控，无爆音/杂音 |
| **2-3 ~ 2-4** | ESP32 连接 Wi-Fi，手机上传 EPUB 到 SD 卡，重启后可在书架看到新书 |
| **3-1** | 屏幕显示正确时间，RTC 断电后时间维持 |
| **3-2 ~ 3-3** | 触摸点击可操作 LVGL 按钮和滚动列表 |
| **3-5** | 待机电流 < 100µA，按键唤醒后恢复阅读界面 |
| **4** | 自制 PCB 正常运行所有功能，外壳组装完整 |

---

## 八、关键技术决策清单

| 决策 | 选项 | 选择 | 理由 |
|------|------|------|------|
| LVGL 文件系统 | 启用 / **跳过** | **跳过** | 所有文件 IO 走 `storage_srv`，在 Process 任务中完成 |
| EPUB XML 解析 | TinyXML2 / **手写轻量解析** | **手写** | 只需要解析 container.xml + OPF 两个固定格式，不需要完整 XML 库 |
| MP3 解码 | libhelix / **dr_mp3** / libmad | **dr_mp3** | 单头文件，无依赖，MIT 协议，最轻量 |
| ESP32 固件 | **ESP-AT** / 自研自定义 | **ESP-AT** | 开箱即用，STM32 侧只需 UART AT 指令解析，无需 ESP-IDF 开发 |
| 触摸驱动 | I2C 中断 / **轮询** | **轮询** | 同按键架构统一：IRQ 只置 flag，应用层轮询读取坐标 |
| 低功耗策略 | RTC 定时唤醒 / **按键唤醒** | 按键唤醒 | 阅读器用户交互模式是"长时间不操作"而非"定时事件" |
| 进度保存格式 | 二进制 / **JSON** / 自定义 | **JSON** | 人类可读，方便调试，`time_srv` 可写入时间戳 |

---

## 九、当前下一步行动（最短路径）

### 立即做（代码量约 50 行，1 天内完成）

1. **`lv_port_indev.c`**：`#if 0` → `#if 1`，在 `button_read()` 中桥接 `input_srv` 获取按键事件，映射到 LV_KEY_xxx
2. **`input_task.c`**：取消 `handle_btn_event()` 的注释
3. **`ui_task.c`**：添加 `lv_port_indev_init()`，将 `os_delay_ms(500)` 改为 `os_delay_ms(5)`

做完这三步，项目就从一个"静态演示"变成一个**可交互的原型**——物理按键可以控制 LVGL 界面了。

### 下一步（~1 周）

4. **`ebook_srv.h/.c`**：实现 TXT 文件解析和基本分页
5. **`process_task.c` + `freertos.c`**：添加消息队列，实现 Process 任务的翻页命令处理和文本发送
6. **`ui_task.c`**：实现阅读器屏幕（`lv_label` 显示文本，翻页刷新）

做完这三步，项目就是一个**能读 TXT 电子书的最小可行产品**。
