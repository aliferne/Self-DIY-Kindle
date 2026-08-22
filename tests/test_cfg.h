#pragma once

#define configUSE_TEST_FUNCTIONS

#ifdef configUSE_TEST_FUNCTIONS

#define configRUN_EBOOK_TEST   1
#define configRUN_INPUT_TEST   1
#define configRUN_MUSIC_TEST   1
#define configRUN_NET_TEST     1
#define configRUN_STORAGE_TEST 1
#define configRUN_TIME_TEST    1

extern void ebook_test();

#endif /* configUSE_TEST_FUNCTIONS */

