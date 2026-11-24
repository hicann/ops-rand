# 算子调试调优

本文档介绍ops-rand项目中常见的算子调试方法。

## 概述

在算子开发过程中，可能会遇到以下问题：

- 算子功能异常，输出结果不正确
- 算子性能不达标，需要优化
- 算子运行时报错

针对这些问题，本文档提供以下调试方法：

- [日志调试](#日志调试)：通过日志定位问题

性能调优方法暂未收录。

## 日志调试

### 编译时日志

使用`-v`参数查看详细编译输出：

```bash
bash build.sh -v --run
```

### 运行时日志

CANN提供了多种日志级别，可通过环境变量配置：

```bash
# 设置日志级别
export ASCEND_GLOBAL_LOG_LEVEL=3  # 0-debug, 1-info, 2-warning, 3-error

# 设置日志路径
export ASCEND_GLOBAL_EVENT_ENABLE=0
```

### 常见错误排查

| 错误信息 | 可能原因 | 解决方案 |
| :--- | :--- | :--- |
| `Out of memory` | 内存不足 | 减小tile size或增加workspace |
| `Invalid tiling parameters` | Tiling参数错误 | 检查TilingData结构体定义 |
| `Kernel launch failed` | Kernel启动失败 | 检查核函数定义和参数 |