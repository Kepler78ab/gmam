#ifndef CONFIG_H
#define CONFIG_H

#define PACKAGE_VERSION "1.6.2"

/* 解决 ICONV_CONST 未定义问题 */
#define ICONV_CONST

/* 声明支持的功能 */
#define HAVE_STRDUP 1
#define HAVE_STRTOL 1
#define HAVE_ICONV 1

/* 如果是在 MinGW 环境下，屏蔽掉一些 Linux 特有的定义 */
#ifdef __MINGW32__
#include <stdint.h>
#endif

#endif
