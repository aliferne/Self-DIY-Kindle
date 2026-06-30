# 第一阶段

目前只是原型验证阶段，所需要完成的工作为：

1. [ ] 环境配置
    - [ ] 初始化相关外设
    - [ ] 移植 FreeRTOS， FatFs， Miniz，LVGL 等相关模块
    - [ ] 测试通过所有外设均无问题
2. [ ] 基础电子书功能实现
    - [ ] 完成屏幕的相关功能，如 UI 渲染，界面绘制
    - [ ] 完成文件系统相关功能，如正确流式加载 txt、epub 等文件
    - [ ] 完成解析相关功能
3. [ ] 基础音频播放功能实现
    - [ ] UI 和功能界面的绘制
    - [ ] 完成文件系统相关功能，如正确加载音频文件
    - [ ] 完成播放功能，如送入 DAC 中解码并加载到耳机孔
4. [ ] 联网功能实现
    - [ ] ESP32 完成 AT 指令固件（烧录官方固件即可）
    - [ ] 完成与 ESP32 的互通信
    - [ ] 手机 APP 开发，通过 WiFi 传书/歌曲
    - [ ] 远程 OTA，升级系统固件
5. [ ] RTC 功能实现
6. [ ] TODO: 更多功能

目前感觉有些模块的延时函数设计不太好，我想是时候给它们的结构体统一加入回调了

# 参考资料

## 博客

- [天空星开发板原理图和 PCB](https://oshwhub.com/li-chuang-kai-fa-ban/li-chuang-liang-shan-pai-tian-kong-xing-kai-fa-ban)
- [软件 I2C 原理](https://www.cnblogs.com/yxysuanfa/p/19106787)
- [软件 SPI 原理](https://zhuanlan.zhihu.com/p/654500844)
- [0.96 寸 OLED I2C 屏幕驱动](https://wiki.lckfb.com/zh-hans/tkx/tkx-stm32f407vxt6/module/screen/0-96-iic-single-screen.html)
- [waveshare e-paper related docs](https://www.waveshare.net/wiki/4.2inch_e-Paper_Module_Manual)
- [st7735s 超详细讲解+例程](https://blog.csdn.net/riversuer/article/details/145291688)
- [野火 SDIO 示例程序](https://doc.embedfire.com/mcu/stm32/h743prov/hal/zh/latest/book/SDIO.html)
- [FatFs 移植教程](https://www.cnblogs.com/star-light-glimmer/p/17983261)
- [FatFs 官网](https://elm-chan.org/fsw/ff/)
- [LVGL 移植指南](https://lvgl.100ask.net/8.3/porting/index.html)
- [LVGL 显示中文](https://www.cnblogs.com/shumei52/p/18595202)
- [LVGL 转换自定义字体教程](https://www.cnblogs.com/hwd00001/p/17393135.html)
- [LVGL Font Tool 下载](http://dz.lfly.xyz/forum.php?mod=viewthread&tid=24&extra=page=1)
- [LVGL 绘图机制](https://zhuanlan.zhihu.com/p/651924974)

---

下面的暂未使用

- [事件驱动模型](https://blog.csdn.net/sinat_36817189/article/details/106205029)
- [I2S 原理讲解](https://zhuanlan.zhihu.com/p/678943329)
- [STM32 I2S 代码示例](https://doc.embedfire.com/mcu/stm32/f429tiaozhanzhe/std/zh/latest/book/I2S.html)
- [ESP32-with-arduino 驱动 PCM5102 模块代码示例](https://jishuzhan.net/article/1863030601469136897)
- [TI PCM5102 数据手册](https://www.ti.com/product/PCM5102)

--- 

这部分则是因为烧写到外部 Flash 实在太麻烦了于是懒得搞，
反正都有 FS 了我直接做个缓冲区然后用 FS 去加载就完事了，全存 SD 卡就没那么多屁事了

- [JLink 烧写 SPI Flash](https://zhuanlan.zhihu.com/p/54729190)
- [MCU 如何将资源烧写至外部 Flash](https://www.cnblogs.com/skullboyer/p/13812510.html)

## 书籍, PDF 文档

王利 —— 《嵌入式C语言自我修养：从芯片、编译器到操作系统》 [ Chapter 8 ~ 9 ]
安富莱电子 —— 《安富莱_STM32-V6开发板_用户手册、含BSP驱动包设计》 [ Chapter 50 ]

## 代码

STM32 HAL 库、标准库、Linux 部分驱动源代码
