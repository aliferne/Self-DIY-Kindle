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


# 参考资料

## 博客

[事件驱动模型](https://blog.csdn.net/sinat_36817189/article/details/106205029)
[天空星开发板原理图和 PCB](https://oshwhub.com/li-chuang-kai-fa-ban/li-chuang-liang-shan-pai-tian-kong-xing-kai-fa-ban)
[软件 I2C 原理](https://www.cnblogs.com/yxysuanfa/p/19106787)
[软件 SPI 原理](https://zhuanlan.zhihu.com/p/654500844)
[0.96 寸 OLED I2C 屏幕驱动](https://wiki.lckfb.com/zh-hans/tkx/tkx-stm32f407vxt6/module/screen/0-96-iic-single-screen.html)
[I2S 原理讲解](https://zhuanlan.zhihu.com/p/678943329)
[STM32 I2S 代码示例](https://doc.embedfire.com/mcu/stm32/f429tiaozhanzhe/std/zh/latest/book/I2S.html)
[ESP32-with-arduino 驱动 PCM5102 模块代码示例](https://jishuzhan.net/article/1863030601469136897)
[TI PCM5102 数据手册](https://www.ti.com/product/PCM5102)
[LVGL 移植指南](https://www.cnblogs.com/RefuelW/articles/18940211)
[LVGL 生成中文字库指南](https://www.cnblogs.com/hwd00001/p/17393135.html)

## 书籍

王利 —— 《嵌入式C语言自我修养：从芯片、编译器到操作系统》 [ Chapter 8 ~ 9 ]

## 代码

STM32 HAL 库、标准库、Linux 部分驱动源代码
