## Skiplist + Ring-Log 性能测试

本仓库是一个 **C++ 跳表（Skiplist）实现**，并对比了自实现 Skiplist 与 STL `std::map` 在 **插入 100 万条数据** 时的性能。同时集成了 **开源日志库 Ring-Log**，用于记录基准测试日志。

### 项目结构

- **`skiplist/skiplist.h`**：模板化 Skiplist 实现（插入 / 删除 / 查找 / 持久化等）
- **`Ring-Log/`**：开源 Ring-Log 日志库源码,https://github.com/LeechanX/Ring-Log
- **`main.cpp`**：使用 Skiplist 插入 100 万条数据并计时（带 Ring-Log）
- **`main_.cpp`**：使用 `std::map` 插入 100 万条数据并计时（带 Ring-Log）

### 构建方式

项目既可以用 CMake 构建，也可以用 `g++` 直接编译基准程序。

#### 使用 g++（推荐用于简单对比）

在仓库根目录：

```bash
mkdir -p log

g++ -std=c++17 -O2 -DNDEBUG main.cpp Ring-Log/src/rlog.cc -I. -pthread -o skiplist_bench
g++ -std=c++17 -O2 -DNDEBUG main_.cpp Ring-Log/src/rlog.cc -I. -pthread -o map_bench
```

运行：

```bash
./skiplist_bench
./map_bench
```

终端会打印插入 100 万条数据的耗时，Ring-Log 的日志会写入 `./log` 目录。

#### 使用 CMake（如需集成到更大工程）

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

> 注：如需用 CMake 生成 `skiplist_bench` / `map_bench`，可以在 `CMakeLists.txt` 中增加相应的 `add_executable`，并链接 `Ring-Log/src/rlog.cc` 以及 `pthread`。

### 当前测试结果（本机实测）

环境：Linux / g++ / `-O2 -DNDEBUG`

```text
std::map 插入 1000000 条数据耗时: 220 ms
Skiplist 插入 1000000 条数据耗时: 178 ms
```

> 不同机器、编译器版本、优化选项会导致结果不同，这里仅作一个参考数值。

### Ring-Log 使用说明

本项目直接使用 `Ring-Log/src/rlog.h` / `rlog.cc`，在 `main.cpp` / `main_.cpp` 中通过：

```cpp
LOG_INIT("./log", "skiplist_bench", INFO);   // 或 "map_bench"
```

完成日志系统初始化，并在运行时异步将日志写入 `./log` 目录。

Ring-Log 为第三方开源项目，本仓库仅作学习和性能测试使用。  
如需了解 Ring-Log 的更多细节或在生产环境中使用，请参考其上游开源仓库(https://github.com/LeechanX/Ring-Log)。

### 许可证

本仓库仅作为学习与实验用途，你可以根据自己的需要选择合适的开源协议（例如 MIT / Apache-2.0 等）并在此处补充说明；同时请遵守 Ring-Log 原项目的开源协议要求。

