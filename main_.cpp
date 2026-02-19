#include <iostream>
#include <map>
#include <string>
#include <chrono>

#include "Ring-Log/src/rlog.h"

int main()
{
    LOG_INIT("./log", "map_bench", INFO);
    const int N = 1'000'000;
    std::map<int, std::string> mp;
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i)
    {
        mp.emplace(i, "v");
    }
    auto end = std::chrono::steady_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "std::map 插入 " << N << " 条数据耗时: " << diff_ms << " ms" << std::endl;
    return 0;
}