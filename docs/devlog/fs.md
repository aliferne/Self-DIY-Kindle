SDIO 的驱动还存在部分问题，主要是超时机制尚未加入， DMA 和 IT 还是存在问题的

不过 FatFs 则基本功能均正常，这些已经测试过了

现在存储服务也已经正常

现在架构是

storage_srv (service) -> lv_port_fs (middlewares) -> applications

说实话其实我都不确定 storage_srv 是否应该保留，我总觉得实际上直接用 lvgl 的大而美文件系统就够了，反正也不会换库，不过写都写了不用太可惜了，看后面会不会造成负担吧，会的话再重构

目前还是决定不启用 LVGL 的 FS，用自己的

在测试 SD 卡驱动时突然发现 debian 对 SD 卡不可写了，看命令：

```bash
sudo fsck -t vfat -n /dev/sda
```

发现是因为 Dirty Bit 被置位，一开始以为是 STM32 FatFs 的问题，但是查了半天查不出来，
于是换了张卡，发现问题依然存在，最后实在绷不住了去问 AI，然后发现居然是因为自己在电脑端拔 SD 卡的时候没 umount 导致的……


```bash
sudo fsck.vfat -v /dev/sda # 修复 dirty bit 问题
# 重新挂载之后恢复正常
```
