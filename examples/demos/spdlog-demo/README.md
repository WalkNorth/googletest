# spdlog-demo

演示如何在本仓库中使用 `spdlog`。示例演示了控制台日志与基本文件日志。

构建方式（推荐从仓库根目录运行）：

1. 使用仓库内的 `lib/spdlog`（若你已将 `spdlog` 源码放在仓库的 `lib/spdlog`）：

```bash
mkdir -p build/spdlog-demo
cmake -S examples/demos/spdlog-demo -B build/spdlog-demo -DSPDLOG_DIR=${PWD}/lib/spdlog
cmake --build build/spdlog-demo
./build/spdlog-demo/spdlog_demo
```

2. 如果系统已有 `spdlog`（通过包管理器或 vcpkg/conan 提供）：

```bash
mkdir -p build/spdlog-demo
cmake -S examples/demos/spdlog-demo -B build/spdlog-demo
cmake --build build/spdlog-demo
./build/spdlog-demo/spdlog_demo
```

运行后会在当前目录生成 `spdlog_demo.log`，并在控制台输出示例日志。

异步与多 sink 演示
-----------------
示例同时包含一个异步多 sink 演示，会向彩色控制台和 `spdlog_demo_rotating.log` 写入大量并发日志：

```bash
# 构建（同上）
./build/spdlog-demo/spdlog_demo
```

运行后会生成 `spdlog_demo_rotating.log`（轮转）以及 `spdlog_demo.log`（basic demo 的文件输出）。程序会在完成后退出。
