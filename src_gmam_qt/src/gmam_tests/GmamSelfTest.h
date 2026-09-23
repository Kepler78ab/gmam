#pragma once

/**
 * @file GmamSelfTest.h
 * @brief GMAM 自测入口（冒烟 / 多错误收集 / 旧拼装比对）
 */

/** 基础编码 + v0.0.2 多错误收集 */
bool gmamSmokeTest();

/**
 * 模仿 ExeclDataUpload 结构体拼装 vs DataContentBuilder，逐字节比对并分段打印。
 */
bool gmamCompareT9LegacyVsGmam();

/** 依次执行上述测试 */
bool gmamRunAllTests();
