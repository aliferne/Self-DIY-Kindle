# Miniz 移植情况

目前已经添加到了编译文件中并编译通过了，但尚未测试基础 API

Miniz 借助各种 `xx_NO_xx` 的宏定义来判断启用哪些功能（这是个可扩展的设计，应当学习）

目前启用的宏为：

```h
/* Defines to completely disable specific portions of miniz.c:
   If all macros here are defined the only functionality remaining will be CRC-32 and adler-32. */

/* Define MINIZ_NO_STDIO to disable all usage and any functions which rely on stdio for file I/O. */
#define MINIZ_NO_STDIO

/* If MINIZ_NO_TIME is specified then the ZIP archive functions will not be able to get the current time, or */
/* get/set file times, and the C run-time funcs that get/set times won't be called. */
/* The current downside is the times written to your archives will be from 1979. */
#define MINIZ_NO_TIME

/* Define MINIZ_NO_DEFLATE_APIS to disable all compression API's. */
#define MINIZ_NO_DEFLATE_APIS

/* Define MINIZ_NO_INFLATE_APIS to disable all decompression API's. */
/*#define MINIZ_NO_INFLATE_APIS */

/* Define MINIZ_NO_ARCHIVE_APIS to disable all ZIP archive API's. */
/*#define MINIZ_NO_ARCHIVE_APIS */

/* Define MINIZ_NO_ARCHIVE_WRITING_APIS to disable all writing related ZIP archive API's. */
#define MINIZ_NO_ARCHIVE_WRITING_APIS

/* Define MINIZ_NO_ZLIB_APIS to remove all ZLIB-style compression/decompression API's. */
/*#define MINIZ_NO_ZLIB_APIS */

/* Define MINIZ_NO_ZLIB_COMPATIBLE_NAME to disable zlib names, to prevent conflicts against stock zlib. */
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES

/* Define MINIZ_NO_MALLOC to disable all calls to malloc, free, and realloc.
   Note if MINIZ_NO_MALLOC is defined then the user must always provide custom user alloc/free/realloc
   callbacks to the zlib and archive API's, and a few stand-alone helper API's which don't provide custom user
   functions (such as tdefl_compress_mem_to_heap() and tinfl_decompress_mem_to_heap()) won't work. */
#define MINIZ_NO_MALLOC
```

即我们移植 miniz 所达到的最终可用功能是：

- 不需要标准库（单片机环境）
- 不需要时间戳
- 仅解压
- 静态分配

# Miniz 使用情况

TODO: 合理的抽象

# 参考链接

[Miniz 官方 GitHub 链接]: https://github.com/richgel999/miniz
[合宙资料中心——Miniz 压缩解压缩]: https://docs.openluat.com/air780epm/luatos/app/common/miniz/
