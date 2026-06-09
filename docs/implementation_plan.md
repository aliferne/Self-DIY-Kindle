# Self-DIY-Kindle 实现计划

> 基于 2026-06-10 代码审查的完整评估与路线图

---

## 一、项目状态总览

**总体完成度：约 25-30%**

阶段性评估：项目处于**第一阶段（原型验证）中期**。硬件抽象层和框架基础设施已相当扎实，但所有面向用户的业务功能（阅读器、GUI、联网、音频）仍处于空壳或演示阶段。

---

## 二、已完成模块清单

| 模块 | 状态 | 说明 |
|------|------|------|
| **BSP GPIO 抽象** | ✅ 完整 | `void *` 句柄封装，不暴露 HAL 头文件 |
| **BSP SPI 抽象** | ✅ 完整 | 支持硬件 SPI + 软件 SPI (bit-bang)，Mode 0-3 |
| **BSP I2C 抽象** | ✅ 完整 | 软件 I2C (bit-bang)，支持 write/read/write-read 复合操作 |
| **BSP SDIO 抽象** | ✅ 完整 | 支持 Polling/DMA/IT 模式，已集成 FatFs |
| **BSP UART 抽象** | ✅ 完整 | 基本 init/send/recv/irq 接口 |
| **DWT 微秒延时** | ✅ 完整 | 基于 DWT CYCCNT，不占用定时器 |
| **STM7735S TFT 驱动** | ✅ 完整 | 含中文显示、基本绘图 API |
| **4.2" e-Paper 驱动** | ✅ 完整 | 支持 Normal/Fast/4Gray 初始化、局部刷新、休眠 |
| **FreeRTOS 集成** | ✅ 完整 | 6 个 FreeRTOS 任务已创建（CMSIS-RTOS 封装） |
| **FatFs 文件系统** | ✅ 完整 | SDIO + FatFs，SD 卡存储 |
| **Storage 服务** | ✅ 完整 | 路径前缀、目录自动创建、写后 sync 防掉电损坏 |
| **Input 服务** | ✅ 完整 | 按键 irq_flag 轮询，事件类型定义完整 |
| **Miniz (ZIP 解压)** | ✅ 已包含 | 库文件就绪，未接入服务 |
| **LVGL 配置** | ✅ 已配置 | `lv_conf.h` 已启用大部分 widgets，但移植文件未实现 |
| **系统初始化流程** | ✅ 正确 | HAL → clock → MX 外设 → chip_init → bsp_init → mid_init → service_init → FreeRTOS → 内核启动 |

---

## 三、未完成模块及优先级

### 🔴 P0 — 核心阅读器功能

| # | 功能 | 当前状态 | 工作量 | 依赖 |
|---|------|---------|--------|------|
| 1 | LVGL 显示移植 `lv_port_disp.c` | 模板，`#if 0` 未启用 | ★☆☆ 小 | — |
| 2 | LVGL 输入设备移植 `lv_port_indev.c` | 模板，`#if 0` 未启用 | ★★☆ 中 | P0-1 |
| 3 | LVGL 文件系统移植 `lv_port_fs.c` | 模板，`#if 0` 未启用 | ★☆☆ 小 | — |
| 4 | UI 任务替换为 LVGL 主循环 | 当前仅是 TFT 中文演示 | ★★☆ 中 | P0-1, P0-2 |
| 5 | Input 任务接通按钮事件 | `handle_btn_event()` 被注释 | ★☆☆ 小 | P0-2 |
| 6 | EPUB/TXT 解析管线 `ebook_srv` | 头文件空壳 | ★★★ 大 | P0-3 (miniz) |
| 7 | Process 任务数据管道 | 空循环 | ★★☆ 中 | P0-6 |

### 🟡 P1 — 必备体验功能

| # | 功能 | 当前状态 | 工作量 | 依赖 |
|---|------|---------|--------|------|
| 8 | 书架与文件浏览 UI (LVGL 屏幕) | 不存在 | ★★☆ 中 | P0-1, P0-4 |
| 9 | 阅读器 UI（翻页、进度、字体设置） | 不存在 | ★★★ 大 | P0-4, P0-6 |
| 10 | 阅读进度保存 (SD/Flash) | 不存在 | ★☆☆ 小 | P0-7 |
| 11 | e-paper 替换 TFT | 驱动就绪，`mid_init_modules` 未启用 | ★☆☆ 小 | P0-1 |
| 12 | 显示服务层完整抽象 `display_srv` | 目前只封装了背光控制 | ★☆☆ 小 | P1-11 |

### 🟢 P2 — 联网与音频

| # | 功能 | 当前状态 | 工作量 | 依赖 |
|---|------|---------|--------|------|
| 13 | ESP32 UART 通信协议 `net_srv` | 空壳 | ★★★ 大 | — |
| 14 | Wi-Fi 配网与无线传书 `net_task` | 空循环 | ★★★ 大 | P2-13 |
| 15 | 远程 OTA 升级 | 不存在 | ★★★ 大 | P2-13 |
| 16 | I2S 音频抽象 `bsp_i2s.h` | 仅 `#pragma once` | ★★☆ 中 | — |
| 17 | MP3 解码与播放 `music_srv` | 空壳 | ★★★ 大 | P2-16 |
| 18 | Music 任务 `music_task` | 空循环 | ★★☆ 中 | P2-17 |

### 🔵 P3 — 高级功能

| # | 功能 | 当前状态 | 工作量 | 依赖 |
|---|------|---------|--------|------|
| 19 | RTC 驱动 + 时间服务 `time_srv` | `bsp_rtc.h` 仅 `#pragma once` | ★★☆ 中 | — |
| 20 | GT911 触摸驱动 (I2C 中断) | 不存在 | ★★☆ 中 | P0-1 |
| 21 | ESP32 BLE 键盘 HID 解析 | 不存在 | ★★★ 大 | P2-13 |
| 22 | 低功耗模式 (STM32 STOP) | 不存在 | ★★☆ 中 | P3-19 |
| 23 | 电量检测 (ADC/MAX17048) | 不存在 | ★☆☆ 小 | — |

### ⚫ P4 — 硬件设计

| # | 功能 | 当前状态 | 工作量 |
|---|------|---------|--------|
| 24 | F411CEU6 + ESP32-C3 PCB 设计 | 在 demands.md 中有方案描述 | ★★★ 大 |
| 25 | 3D 打印外壳 | 未开始 | ★★★ 大 |
| 26 | 整机组装与测试 | 未开始 | ★★☆ 中 |

---

## 四、实施路线图（分步执行）

### 第一步：让 LVGL 跑起来

**目标**：LVGL 在 TFT 上显示 "Hello Kindle"，物理按键触发 GUI 导航。

**涉及文件**：

| 文件 | 操作 |
|------|------|
| `Middlewares/lvgl/porting/lv_port_disp.c` | 实现 `disp_flush()` 调用 TFT 驱动，改 `#if 0` → `#if 1` |
| `Middlewares/lvgl/porting/lv_port_indev.c` | 实现 `button_read()` 桥接 `input_srv`，改 `#if 0` → `#if 1` |
| `Middlewares/lvgl/porting/lv_port_fs.c` | 实现文件操作桥接 `storage_srv`，改 `#if 0` → `#if 1` |
| `applications/ui_task.c` | 替换为 LVGL 主循环：`lv_init()` → `lv_port_disp_init()` → `lv_timer_handler()` → `osDelay(5)` |
| `applications/input_task.c` | 取消 `handle_btn_event()` 注释 |

**验证**：上电后 LVGL 文本显示在 TFT 上，按翻页键 LED 亮灭变化。

**预计工作量**：~1 周

---

### 第二步：EPUB 解析管线

**目标**：SD 卡中的 `.epub` 文件内容能显示在 TFT 上，翻页键控制前后翻页。

**涉及文件**：

| 文件 | 操作 |
|------|------|
| `services/ebook_srv.h` | 定义 `EbookDoc_t`, `ebook_open()`, `ebook_get_page_text()`, `ebook_close()` |
| `services/ebook_srv.c` | 实现 miniz 解压 → 解析 container.xml/OPF → 提取 XHTML 文本 |
| `services/ebook_srv.c` (续) | TXT 格式直接读取，分页管理 |
| `applications/process_task.c` | 实现消息队列：接收翻页事件 → 加载文本 → 发送给 UI |
| `applications/process_task.h` | 定义进程间消息结构体 |

**架构**：

```
SD卡 (.epub)
  │
  ▼
Process Task (解析线程)
  │  ebook_srv: miniz 解压 → 文本提取 → 分页缓存
  │
  ├── osMailPut() ──► UI Task (LVGL 显示)
  │
  └── osMailGet() ◄── Input Task (翻页事件)
```

**预计工作量**：~1-2 周

---

### 第三步：e-paper 集成 + 阅读器 UI

**目标**：e-paper 显示书架，选择书籍进入阅读模式，翻页流畅，关机进度保存。

**涉及文件**：

| 文件 | 操作 |
|------|------|
| `Middlewares/mid_config.c` | 切换 `mid_init_tft()` → `mid_init_epaper()` |
| `Middlewares/lvgl/porting/lv_port_disp.c` | `disp_flush()` 改为适配 e-paper（全屏刷/局部刷切换） |
| `services/display_srv.c` | 增加 `display_device_t` 抽象（init/clear/draw/refresh/sleep） |
| `services/storage_srv.c` | 可选增加 JSON 读写接口用于进度保存 |
| `applications/ui_task.c` | 书架屏幕：`lv_list` / `lv_table` 显示 SD 卡书籍 |
| `applications/ui_task.c` (续) | 阅读器屏幕：`lv_label` 显示文本，支持翻页 |

**e-paper LVGL 适配要点**：

- 使用 `LV_COLOR_DEPTH 1`（黑白 e-paper）或保持 16 位并转换
- 刷新模式：全刷（翻页时）vs 局部刷（高亮、进度条）
- `disp_drv.full_refresh = 1` 使用全屏缓冲区
- 避免 LVGL 动画效果（`LV_USE_ANIMATION = 0`）

**预计工作量**：~1-2 周

---

### 第四步：联网与音频（可并行）

**目标**：ESP32 连接 Wi-Fi、HTTP 传书到设备；耳机播放 MP3。

**涉及文件**：

| 文件 | 操作 |
|------|------|
| `services/net_srv.h` | 定义 AT 指令协议、文件传输 API |
| `services/net_srv.c` | 实现 ESP32 UART 通信（AT 指令 / 自定义固件） |
| `applications/net_task.c` | 联网状态机：配网 → 连接 → 文件接收 → 存储 |
| `bsp/bsp_i2s.h` | 补全 I2S 抽象接口（init/start/stop） |
| `chip/stm32f4/i2s_chip.c` | 实现硬件 I2S + DMA |
| `services/music_srv.c` | MP3 解码（libhelix）+ I2S DMA 输出 |
| `applications/music_task.c` | 播放控制队列 |

**预计工作量**：~2-3 周

---

### 第五步：高级功能

**目标**：RTC 时间、触摸交互、蓝牙键盘输入、低功耗待机。

**涉及文件**：

| 文件 | 操作 |
|------|------|
| `bsp/bsp_rtc.h/.c` | 补全 RTC 实现（init/set/get/alarm/wakeup） |
| `services/time_srv.c` | NTP 时间同步、时间显示 API |
| `Middlewares/new/gt911/` | 新增 GT911 触摸驱动（I2C 中断） |
| `chip/stm32f4/i2c_chip.c` | 添加硬件 I2C 支持（当前只有软件 I2C） |
| `applications/lowpower.c` (新) | 低功耗状态机：无操作 5 分钟 → STOP 模式 |
| `services/net_srv.c` (续) | BLE HID 键盘解析 |

**预计工作量**：~2 周

---

### 第六步：硬件设计（可在第三步后并行）

**目标**：F411CEU6 + ESP32-C3 的 4 层 PCB 打样、焊接、外壳组装。

**参考文档**：`docs/demands.md` 中的完整硬件方案。

| 阶段 | 内容 |
|------|------|
| 原理图 | F411CEU6 最小系统 + ESP32-C3 + e-paper FPC + SDIO + I2S + RTC |
| 4 层 PCB | 注意天线净空、SDIO 等长、数模地分离 |
| 打样焊接 | 嘉立创打样，手工焊接 / 回流焊 |
| 调试 | 各外设逐一调试，移植固件 |
| 外壳 | SolidWorks 设计 → PLA 3D 打印 → 打磨组装 |

**预计工作量**：~3-4 周

---

## 五、依赖关系图

```
P0-1 (LVGL 显示移植) ──────────────┐
P0-2 (LVGL 输入移植) ──────────────┤
P0-3 (LVGL 文件系统移植) ──────────┤
                                    ▼
              P0-4 (UI 任务替换 LVGL) ◄──── P0-5 (Input 接通)
                        │
                        ▼
              P0-6 (EPUB 解析) ◄──── P0-7 (Process 任务管道)
                        │
                        ▼
         ┌──────────────┼──────────────┐
         ▼              ▼              ▼
    P1-8 (书架)    P1-9 (阅读器)   P1-11 (e-paper)
         │              │              │
         └──────┬───────┘              │
                ▼                      ▼
          P1-10 (进度保存)        P1-12 (显示抽象)

P2-13 (ESP32 通信) ──── P2-14 (Wi-Fi 传书) ──── P2-15 (OTA)
P2-16 (I2S 抽象)  ──── P2-17 (MP3 解码) ──── P2-18 (Music 任务)

P3-19 (RTC) ──── P3-22 (低功耗)
P3-20 (触摸)          P3-21 (蓝牙键盘)
```

---

## 六、当前下一步行动

1. **修改三个 LVGL 移植文件**（`lv_port_disp.c`, `lv_port_indev.c`, `lv_port_fs.c`）：
   - 将 `#if 0` 改为 `#if 1`
   - 在 `lv_port_disp.c` 的 `disp_flush()` 中调用 TFT 驱动写入像素
   - 在 `lv_port_indev.c` 的 `button_read()` 中调用 `input_srv` 获取按键事件
   - 在 `lv_port_fs.c` 中桥接 `storage_srv` 的文件操作

2. **重写 `ui_task.c`** 为 LVGL 主循环

3. **取消 `input_task.c` 中 `handle_btn_event()` 的注释**

4. **编译验证**：确认无编译错误，LVGL 在 TFT 上显示内容
