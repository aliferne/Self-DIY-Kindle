# FreeRTOS 下的栈溢出 —— 从 PendSV 到 HardFault

## 前言

打完比赛之后感到无事可做，因此最近正在自己 DIY 一个掌上阅读器，为了更好地~~折磨~~提升自己，决定引入一堆以前自己几乎从来没学过的东西，并且还尝试学 Linux 驱动代码的风格去做一个芯片无关的 BSP 层，不过这些不会是这篇文章的重点，等这个项目完工之后应该会整理文档并发博客，不过那都是后话了。

那么就先介绍下这个问题的背景吧，由于掌上阅读器肯定需要 UI 和 SD 卡，所以我就**引入了 LVGL 和 FatFs**，而因为我雄心相对较大（这不完全只是一个掌上阅读器），所以还**引入了 FreeRTOS**，不过我的水平只局限于能创建一些任务，这个项目也顺带附带着我学习高级用法的想法。

之后我为了测试方便就索性一股脑全写到 ui_task.c 文件中，下面是源代码：

```c
void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();

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

相信一看代码，老手基本就知道是什么情况了，确实就是栈分配得不够，不过鉴于我还是个初学者，这里就卖点小关子，先不给你们看任务创建函数，而是看我的一整套 Debug 流程，做一个抛砖引玉的作用，希望能够得到高人的指导，比如更快定位问题的方法，一些 Debug 的方法论等等，在此谢谢大家了。

## 事故现场复现

本次事故的表现是 LED 灯不闪烁，而屏幕可以正常显示。

这个名为 test.txt 的文件是存在的，因此不存在卡死在一开始的 `for` 循环的情况，而读取操作完全正常， UI 界面可以显示文件内容，这就使我犯了难，因为至少表面上看起来不像是代码问题。没办法，进 Debug 看堆栈吧。

## 事故原因分析

我的断点设置在两句 `ASSERT_FAIL` 处，以及最后 `for` 循环的三个函数内。逐步执行，两句宏的断言均通过，而最后在 `os_delay_ms(500)` 处，按步执行之后不再正常进入此任务，而正常来说应当会从 `gpio_toggle` 再度执行循环内函数。

因此此时我按下暂停，发现居然进了 `HardFault_Handler`，此时我开始观察栈调用情况：

```
HardFault_Handler@0x080009e2 (/home/ferne/code/self-proj/Self-DIY-Kindle/Core/Src/stm32f4xx_it.c:95)
<signal handler called>@0xfffffff1 (未知源:0)
PendSV_Handler@0x080075a0 (/home/ferne/code/self-proj/Self-DIY-Kindle/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F/port.c:435)
```

得，一个 PendSV，一个 HardFault，还有个不知道什么情况的错误码，准备翻手册吧。
由于有 [上一篇文章](https://aliferne.github.io/2026/02/17/build-up-zig-dev-env-on-stm32g431/) 的经验，查手册还是非常轻车熟路的。

首先看 0xE000ED2C，先确认一下 HFSR 是个什么情况，看到值又是 0x40000000，依然非常熟悉的 FORCED 置位，说明真凶另有他人，接下来让我们看到 CFSR(0xE000ED28)，发现值为 0x00008200，将该值与 (1 << 8) 到 (1 << 15) 做与运算，发现分别为 bit 9 和 bit 15 被置位，对应为 PRECIS ERR 和 BFARVALID.

然后是 signal handler 返回的是 0xFFFFFFF1，对应 `EXC_RETURN` 的值为这个。

信息收集够了，开始破案吧。

### PendSV 讲解

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

### `EXC_RETURN` 讲解

## 解决方案

## 总结
