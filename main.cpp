#include "skiplist/skiplist.h"
#include "Ring-Log/src/rlog.h"

#include <string>
#include <chrono>
#include <iostream>

int main()
{
    LOG_INIT("./log", "skiplist_bench", INFO);

    const int max_level = 21;
    const std::string file_name = "./dumpfile_skiplist_1m";
    const int N = 1'000'000;

    Skiplist<int, std::string> skiplist(max_level, file_name);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; ++i)
    {
        skiplist.insert(i, "v");
    }
    auto end = std::chrono::steady_clock::now();
    auto diff_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Skiplist 插入 " << N << " 条数据耗时: " << diff_ms << " ms" << std::endl;
    return 0;
}