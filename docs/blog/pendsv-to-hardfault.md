# FreeRTOS 下的栈溢出 —— 从 PendSV 到 HardFault

## 前言

打完比赛之后感到无事可做，因此最近正在自己 DIY 一个掌上阅读器，为了更好地~~折磨~~提升自己，决定引入一堆自己以前几乎从来没学过的东西，并且还尝试学 Linux 驱动代码的风格去做一个芯片无关的 BSP 层，不过这些不会是这篇文章的重点，等这个项目完工之后应该会整理文档并发博客，不过那都是后话了。

想造芯片无关的 BSP 层主要是因为我现在手头上并没有 STM32F411CEU6 这个芯片的开发板，之前 DIY MP3 因为种种原因没继续下去，其实也有点想再把这个坑填上，而之前的选型就是这个芯片，手头上也还有这个芯片的存货，就想着先用 F407 先搓个原型验证一下，后面简单修改下配置啥的就可以无痛迁移到其他芯片上，不知道自己造一个芯片无关 BSP 层的想法会不会比较理想化了。

那么就先介绍下这个问题的背景吧，由于掌上阅读器肯定需要 UI 和 SD 卡，所以我就**引入了 LVGL 和 FatFs**，而因为我雄心相对较大（这不完全只是一个掌上阅读器），所以还**引入了 FreeRTOS**，不过我的水平只局限于能创建一些任务，这个项目也顺带附带着我学习高级用法的想法。

之后我为了测试方便就索性一股脑全写到 ui_task.c 文件中，下面是源代码：

```c
void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();

    /* 文件操作部分 ------------------------- */
    FIL fp;
    /* 本质是 f_open */
    FRESULT res = storage_open(&sdcard, &fp, "test.txt", FA_READ | FA_OPEN_EXISTING);
    UINT fnum   = 0;
    ASSERT_FAIL(res != FR_OK,
                /* 本质是 f_close */
                storage_close(&fp);
                /* 本质是 vTaskDelay(1000); */
                for (;;) { os_delay_ms(1000); });
    /* 本质是 f_read */
    res = storage_read(&fp, buf, LEN(buf), &fnum);
    ASSERT_FAIL(res != FR_OK,
                storage_close(&fp);
                for (;;) { os_delay_ms(1000); });
    storage_close(&fp);
    /* 文件操作部分 End ------------------------- */

    /* UI 部分 ------------------------- */
    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 5);

    /*Make a gradient*/
    lv_style_set_width(&style, 128);
    lv_style_set_height(&style, LV_SIZE_CONTENT);

    lv_style_set_pad_ver(&style, 0);
    lv_style_set_pad_left(&style, 0);

    lv_style_set_x(&style, lv_pct(0));
    lv_style_set_y(&style, 0);

    /*Create an object with the new style*/
    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);

    lv_obj_t *label = lv_label_create(obj);

    if (buf[0] != 0)
        lv_label_set_text(label, buf);
    else
        lv_label_set_text(label, "确认");
    /* UI 部分 End ------------------------- */

    for (;;) {
        /* 以约 500ms 的周期闪烁 LED */
        gpio_toggle(&usr_led);

        lv_timer_handler();
        os_delay_ms(500);
    }
}
```

其中 `ASSERT_FAIL` 是一个自己写的很简单的宏：

```c
/* 失败断言宏，断言 cond 一定为假，否则执行 actions 操作 */
#define ASSERT_FAIL(cond, actions) \
    do {                           \
        if ((cond)) {              \
            actions;               \
        }                          \
    } while (0)
```

相信一看代码，老手基本就知道是什么情况了，**确实就是栈分配得不够**，不过鉴于我还是个初学者，这里就卖点小关子，先不给你们看任务创建函数，而是看我的一整套 Debug 流程，做一个抛砖引玉的作用，希望能够得到高人的指导，比如更快定位问题的方法，一些 Debug 的方法论等等，在此谢谢大家了。

## 事故现场复现

本次事故的表现是 LED 灯不闪烁，而屏幕可以正常显示。

这个名为 test.txt 的文件是存在的，因此不存在卡死在一开始的 `for` 循环的情况，而读取操作完全正常， 而 UI 界面可以显示文件内容，这就使我犯了难，至少表面上看起来不像是代码问题，不过还是得看下代码的。

## 事故原因分析

由于其他任务均为空实现，我因此最先怀疑的就是 ui_task 的代码问题，上面代码很明显可以分为两部分操作，一部分是文件 IO，另一部分是 UI 绘制，我在源代码上均做了标注，我的测试有三个步骤：

1. 注释 *文件 IO* 和 *UI 绘制*, 烧录发现 LED 正常闪烁，进入 debug 发现上下文正常切换
2. 注释 *文件 IO*, 烧录发现 LED 无法闪烁，进入 debug 发现触发 HardFault
3. 注释 *UI 绘制*, 现象同第一次尝试

到这里基本可以确定肯定是 LVGL 啥的有点问题，不过不可能怀疑到人家源代码上，~~毕竟人家水平比我高多了~~所以没办法，也只能进 Debug 看堆栈了。

我的断点设置在两句 `ASSERT_FAIL` 处，以及最后 `for` 循环的三个函数内。逐步执行，两句宏的断言均通过，而最后在 `os_delay_ms(500)` 处，按步执行之后不再正常进入此任务，而正常来说应当会从 `gpio_toggle` 再度执行循环内函数。

因此此时我按下暂停，发现居然进了 `HardFault_Handler`（我是先发现会进 HardFault 再做的三次测试），此时我开始观察栈调用情况：

```
HardFault_Handler@0x080009e2 (/home/ferne/code/self-proj/Self-DIY-Kindle/Core/Src/stm32f4xx_it.c:95)
<signal handler called>@0xfffffff1 (未知源:0)
PendSV_Handler@0x080075a0 (/home/ferne/code/self-proj/Self-DIY-Kindle/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c:435)
```

得，一个 PendSV，一个 HardFault，还有个不知道什么情况的 signal handler 返回码，准备翻手册吧。
由于有 [上一篇文章](https://aliferne.github.io/2026/02/17/build-up-zig-dev-env-on-stm32g431/) 的经验，查手册还是非常轻车熟路的。

首先看 0xE000ED2C，先确认一下 HFSR 是个什么情况，看到值又是 0x40000000，依然非常熟悉的 FORCED 置位，说明真凶另有他人，接下来让我们看到 CFSR(0xE000ED28)，发现值为 0x00008200，将该值与 (1 << 9) 到 (1 << 15) 做与运算，发现分别为 bit 9 和 bit 15 被置位，对应为 PRECIS ERR 和 BFARVALID.

由于 BFARVALID = 1，我们还需要看一下 BFAR 的值是什么，查阅 Cortex M4 手册可以得知寄存器值为 0x20020000，也就是说 PC 访问了这个地址，然后触发了 PRECIS ERR 之后立马跳到 HardFault

最后是 signal handler 返回的是 0xFFFFFFF1 （顺带说一句，正常情况下应当为 0xFFFFFFFD），对应 `EXC_RETURN` 的值为这个。

信息收集够了，开始破案吧。

### F407 内存布局

每次和内核有关的错误基本应该都要先查下内存问题了，看下是不是越界访问啥的，或者是栈错误等各种神奇但又比较常见的原因。

直接去立创商城搜 F407VET6 就能找到数据手册，然后找到内存映射相关的章节，发现如图所示：

![图片](../images/BFAR-VALUE.png)

问题很明显，肯定是内存越界访问了，但这和之前提到的栈有什么关系？

### `EXC_RETURN` 及 Cortex M4 异常处理机制讲解

在此之前我们先填一下上一篇文章没讲 `EXC_RETURN` 的坑。

TODO:

### PendSV 讲解

TODO:

```c
void xPortPendSVHandler( void )
{
	/* This is a naked function. */

	__asm volatile
	(
	"	mrs r0, psp							\n"
	"	isb									\n"
	"										\n"
	"	ldr	r3, pxCurrentTCBConst			\n" /* Get the location of the current TCB. */
	"	ldr	r2, [r3]						\n"
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, push high vfp registers. */
	"	it eq								\n"
	"	vstmdbeq r0!, {s16-s31}				\n"
	"										\n"
	"	stmdb r0!, {r4-r11, r14}			\n" /* Save the core registers. */
	"	str r0, [r2]						\n" /* Save the new top of stack into the first member of the TCB. */
	"										\n"
	"	stmdb sp!, {r0, r3}					\n"
	"	mov r0, %0 							\n"
	"	msr basepri, r0						\n"
	"	dsb									\n"
	"	isb									\n"
	"	bl vTaskSwitchContext				\n"
	"	mov r0, #0							\n"
	"	msr basepri, r0						\n"
	"	ldmia sp!, {r0, r3}					\n"
	"										\n"
	"	ldr r1, [r3]						\n" /* The first item in pxCurrentTCB is the task top of stack. */
	"	ldr r0, [r1]						\n"
	"										\n"
	"	ldmia r0!, {r4-r11, r14}			\n" /* Pop the core registers. */
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
	"	it eq								\n"
	"	vldmiaeq r0!, {s16-s31}				\n"
	"										\n"
	"	msr psp, r0							\n"
	"	isb									\n"
	"										\n"
	#ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata workaround. */
		#if WORKAROUND_PMU_CM001 == 1
	"			push { r14 }				\n"
	"			pop { pc }					\n"
		#endif
	#endif
	"										\n"
	"	bx r14								\n"
	"										\n"
	"	.align 4							\n"
	"pxCurrentTCBConst: .word pxCurrentTCB	\n"
	::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
	);
}
```

## 解决方案

TODO:

## 总结

TODO:

## 参考资料

- [STM32 Cortex M4 编程手册](https://www.st.com/content/ccc/resource/technical/document/programming_manual/6c/3a/cb/e7/e4/ea/44/9b/DM00046982.pdf/files/DM00046982.pdf/jcr:content/translations/en.DM00046982.pdf)
- [Cortex-M 异常处理的 C 实现、栈帧以及 EXC_RETURN](https://zhuanlan.zhihu.com/p/1924962149312226892)
- [ARM CM3/4 异常进入与返回的过程](https://shequ.stmicroelectronics.cn/thread-604515-1-1.html)
- [Cortex-M3 异常返回值 EXC_RETURN](https://www.cnblogs.com/utank/p/11263073.html)
