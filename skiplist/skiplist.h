#pragma once

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <mutex>
#include <fstream>
#include <string>

// ===================== Node =====================

template <typename K, typename V>
class Node
{
public:
    Node() : forward(nullptr), level(0) {}
    Node(K k, V v, int level);
    ~Node();

    K get_key() const;
    V get_val() const;
    void set_val(V v);

    Node<K, V>** forward; // 每一层的前向指针数组
    int level;            // 该节点的实际层高

private:
    K key;
    V val;

    template <typename, typename>
    friend class Skiplist;
};

template <typename K, typename V>
Node<K, V>::Node(K k, V v, int level)
{
    this->key = k;
    this->val = v;
    this->level = level;
    this->forward = new Node<K, V>*[level + 1];
    std::memset(this->forward, 0, sizeof(Node<K, V>*) * (level + 1));
}

template <typename K, typename V>
Node<K, V>::~Node()
{
    delete[] forward;
}

template <typename K, typename V>
K Node<K, V>::get_key() const
{
    return this->key;
}

template <typename K, typename V>
V Node<K, V>::get_val() const
{
    return this->val;
}

template <typename K, typename V>
void Node<K, V>::set_val(V v)
{
    this->val = v;
}


template <typename K, typename V>
class Skiplist
{
public:
    Skiplist(int max_level, const std::string file_name);
    ~Skiplist();

    int get_random_level();                 // 决定新节点高度的“掷硬币”算法
    Node<K, V>* create_node(K, V, int);     // 创建新节点
    int insert(K, V);                       // 插入
    void display_list();                    // 打印
    bool serach(K);                         // 查询
    bool del(K);                            // 删除
    void dump_file();                       // 持久化：从内存写到磁盘
    void load_file();                       // 加载：从磁盘读回内存
    void clear(Node<K, V>*);                // 递归释放内存
    int size();                             // 获取元素总数

private:
    void get_key_val_from_string(std::string& str,
                                 std::string* key,
                                 std::string* val);   // 内部辅助函数：处理字符串解析
    bool is_valid_string(std::string& str);           // 判断一行是否是合法的 key: val 格式

    int max_level;              // 允许的最大高度
    int _skip_list_level;       // 当前跳表索引的最高有效层级
    Node<K, V>* header;         // 跳表的头节点（不存数据，作为查找起点）

    std::ofstream file_write;   // 文件写操作
    std::ifstream file_read;    // 文件读操作

    int _element_count;         // 当前跳表中的元素个数
    std::mutex mutex_;          // 互斥锁
    std::string file_name;      // 文件名
};

template <typename K, typename V>
Skiplist<K, V>::Skiplist(int max_level, const std::string file_name)
{
    this->max_level = max_level;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    this->file_name = file_name;

    // 头节点不存真实 key / value
    K k{};
    V v{};
    this->header = new Node<K, V>(k, v, max_level);
}

template <typename K, typename V>
Skiplist<K, V>::~Skiplist()
{
    // 释放所有节点
    if (header != nullptr)
    {
        clear(header->forward[0]);
        delete header;
        header = nullptr;
    }
}

template <typename K, typename V>
Node<K, V>* Skiplist<K, V>::create_node(K key, V val, int level)
{
    return new Node<K, V>(key, val, level);
}

template <typename K, typename V>
int Skiplist<K, V>::get_random_level()
{
    int k = 1;
    while (std::rand() & 1)
    {
        ++k;
    }
    k = (k > max_level) ? max_level : k;
    return k;
}

template <typename K, typename V>
int Skiplist<K, V>::insert(K key, V val)
{
    // std::lock_guard<std::mutex> lock(mutex_);

    Node<K, V>* now = this->header;
    Node<K, V>* update[max_level + 1];
    std::memset(update, 0, sizeof(Node<K, V>*) * (max_level + 1));

    // 从最高层往下找插入位置
    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (now->forward[i] != nullptr && now->forward[i]->get_key() < key)
        {
            now = now->forward[i];
        }
        update[i] = now;
    }

    now = now->forward[0];

    // 若 key 已存在，更新或返回错误
    if (now != nullptr && now->get_key() == key)
    {
        std::cout << "已经存在" << std::endl;
        return -1;
    }

    // 生成随机层高
    int rand_level = get_random_level();

    if (rand_level > _skip_list_level)
    {
        for (int i = _skip_list_level + 1; i <= rand_level; ++i)
        {
            update[i] = header;
        }
        _skip_list_level = rand_level;
    }

    Node<K, V>* insert_node = create_node(key, val, rand_level);

    for (int i = 0; i <= rand_level; ++i)
    {
        insert_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = insert_node;
    }

    ++_element_count;
    // std::cout << "成功插入" << std::endl;
    return 0;
}

template <typename K, typename V>
bool Skiplist<K, V>::serach(K key)
{
    Node<K, V>* now = this->header;

    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (now->forward[i] != nullptr && now->forward[i]->get_key() < key)
        {
            now = now->forward[i];
        }
    }

    now = now->forward[0];

    if (now != nullptr && now->get_key() == key)
    {
        return true;
    }
    return false;
}

template <typename K, typename V>
bool Skiplist<K, V>::del(K key)
{
    std::lock_guard<std::mutex> lock(mutex_);

    Node<K, V>* now = this->header;
    Node<K, V>* update[max_level + 1];
    std::memset(update, 0, sizeof(Node<K, V>*) * (max_level + 1));

    for (int i = _skip_list_level; i >= 0; --i)
    {
        while (now->forward[i] != nullptr && now->forward[i]->get_key() < key)
        {
            now = now->forward[i];
        }
        update[i] = now;
    }

    now = now->forward[0];

    if (now == nullptr || now->get_key() != key)
    {
        std::cout << "找不到" << std::endl;
        return false;
    }

    for (int i = 0; i <= _skip_list_level; ++i)
    {
        if (update[i]->forward[i] != now)
            continue;
        update[i]->forward[i] = now->forward[i];
    }

    delete now;
    --_element_count;
    while (_skip_list_level > 0 && header->forward[_skip_list_level] == nullptr)
    {
        --_skip_list_level;
    }

    std::cout << "OK" << std::endl;
    return true;
}

template <typename K, typename V>
void Skiplist<K, V>::dump_file()
{
    file_write.open(file_name);
    if (!file_write.is_open())
    {
        std::cerr << "open file failed: " << file_name << std::endl;
        return;
    }

    Node<K, V>* now = header->forward[0];
    while (now != nullptr)
    {
        file_write << now->get_key() << ": " << now->get_val() << "\n";
        now = now->forward[0];
    }

    file_write.flush();
    file_write.close();
}

template <typename K, typename V>
void Skiplist<K, V>::get_key_val_from_string(std::string& str,
                                             std::string* key,
                                             std::string* val)
{
    if (!is_valid_string(str))
    {
        key->clear();
        val->clear();
        return;
    }
    std::size_t p = str.find(':');
    *key = str.substr(0, p);
    if (p + 2 <= str.size())
        *val = str.substr(p + 2);
    else
        val->clear();
}

template <typename K, typename V>
bool Skiplist<K, V>::is_valid_string(std::string& str)
{
    if (str.empty())
        return false;

    std::size_t p = str.find(':');
    if (p == std::string::npos)
        return false;

    if (p + 1 >= str.size())
        return false;

    return true;
}

template <typename K, typename V>
void Skiplist<K, V>::load_file()
{
    file_read.open(file_name);
    if (!file_read.is_open())
    {
        return;
    }

    std::string line;
    while (std::getline(file_read, line))
    {
        std::string key_str;
        std::string val_str;
        get_key_val_from_string(line, &key_str, &val_str);
        if (key_str.empty())
            continue;

        K key{};
        V value{};

        if constexpr (std::is_same<K, int>::value)
        {
            key = std::stoi(key_str);
        }
        else
        {
            // 其他类型你可以按需自己扩展
        }

        if constexpr (std::is_same<V, std::string>::value)
        {
            value = val_str;
        }
        else
        {
            // 其他类型自己扩展
        }

        insert(key, value);
    }

    file_read.close();
}

template <typename K, typename V>
void Skiplist<K, V>::clear(Node<K, V>* node)
{
    Node<K, V>* cur = node;
    while (cur != nullptr)
    {
        Node<K, V>* next = cur->forward[0];
        delete cur;
        cur = next;
    }
}

template <typename K, typename V>
int Skiplist<K, V>::size()
{
    return _element_count;
}

template <typename K, typename V>
void Skiplist<K, V>::display_list()
{
    Node<K, V>* now = this->header->forward[0];
    while (now != nullptr)
    {
        std::cout << now->get_key() << ": " << now->get_val() << std::endl;
        now = now->forward[0];
    }
}