# 内核的艺术 —— 探索 Arm Cortex-M4 内核的架构设计与异常处理机制

## 前言

本文主要是为了自己理解 Arm Cortex-M4 内核的一些东西而写的，主要参考书籍为：

- 《Arm Cortex-M3 与 Cortex-M4 权威指南》
- 《STM32 Cortex-M4 MCUs and MPUs programming manual》

由于本人才疏学浅，势必会有一些地方会文不达意，还请多多海涵。

以下均用 M 代指 Cortex-M， M3/M4 代指 Cortex-M3/Cortex-M4.

## M 内核的架构设计


