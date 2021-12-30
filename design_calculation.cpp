#include <iostream>
#include <string>
#include <sstream>
#include <cmath>

using std::cout;
using std::endl;
/**
 ИЕРАРХИЯ - под огромным вопросом (что это за структура)
    * том - это один или несколько файлов на диске, в которых хранится иерархическая структура узлов,
            содержащих в себе пары ключ/значение.
    * хранилище - это виртуальная иерархическая структура, в узлы которой монтируются произвольные части
                  иерархических структур из одного или нескольких томов.
 ФОРМАТ
    * формат тома должен быть не завязан на архитектуру и платформу, а также описан понятным языком.

   * Если использовать B+ TREE (IndexNode - диапазоны ключей, LeafNode - отмапленные 4kb страницы на файлы фикс. размера)
     * IndexNode -> IN
     * LeafNode  -> LN

     Level 0    :                              Root
                                  /       |      ...       \      \
     Level 1    :             [ IN_l             ...             IN_r ]
                            /  ...  \                           /  ...  \
     Level 2    :        [IN_l ... IN_r]   ... [...] ...    [IN_l ... IN_r]
     ..............................................................................................................
                      /                                                        \
     Level N-1  : [IN_l ... IN_r]          ... [...] ...           [IN_l ... IN_r]
                    ||       ||                                      ||       ||
     Level N    : [LN_l ... LN_r]                                 [LN_l ... LN_R]

   * УДОВЛЕТВОРЯЕТ:
     * размер одного тома может достигать нескольких терабайт.
     * суммарное количество узлов монтирования в хранилище может достигать нескольких тысяч.
     * суммарное количество ключей во всех узлах тома может превышать нескольких сотен миллионов.
     * хранилище должно оптимально работать с различными типами данных, включая:
          uint32, uint64, float, double, string, blob, ...

   * НЕ УДОВЛЕТВОРЯЕТ:
     * один и тот же узел тома может быть примонтирован к нескольким узлам в хранилище.
     * при попадании нескольких узлов из одного или нескольких томов на один узел хранилища их данные объединяются,
        а в случае совпадения ключей берутся данные из наиболее приоритетного тома.

 ОПЕРАЦИИ НАД КЛЮЧАМИ:
    * нужно реализовывать все основные операции над ключами.
    * нужно обеспечить автоматическое удаление ключей, если установлено время жизни ключа.

 МНОГОПОТОЧКА:
    * библиотека должна нормально работать в 32-х и в 64-х битных процессах.
    * библиотека должна поддерживать полноценную многопоточную работу и
        обеспечивать параллельное выполнение большинства операций, в том числе и одновременный доступ к файлам томов.
    * один процесс может одновременно и независимо работать с несколькими хранилищами, у которых тома не пересекаются.

 * */

namespace common {
    constexpr int page_size = 4096; // virtual page size
    constexpr int total_keys = 1E9; // ~ 10^9 different keys
    constexpr long long TB = 1E12;  // ~ 10^12 bytes (~ 1TB)

    constexpr double to_TB_percentage(long long bytes) {
        return double(bytes) / TB * 100;
    }

    constexpr int tree_order = 50; // a number of keys in IndexNode

    int log100(size_t x) {
        return std::ceil(std::log(x) / std::log(tree_order));
    }

    template <typename ValueType, int total_elements = 0, int element_size = 0>
    std::stringstream dump_key_value_info(const char* value_type_name, int buffer_size = 1) {
        std::stringstream ss;
        size_t pair_size;
        if constexpr (std::is_fundamental_v<ValueType>) {
            static_assert ((total_elements == 0) && (element_size == 0));
            constexpr size_t size = sizeof(int) + sizeof(ValueType);
            static_assert (size <= page_size);
            pair_size = size;
        } else {
            static_assert ((total_elements != 0) && (element_size != 0));
            constexpr size_t size = sizeof(int) + element_size * total_elements;
            static_assert (size <= page_size);
            pair_size = size;
        }
        size_t max_pair_count = page_size / pair_size;
        size_t leaf_node_count = total_keys / max_pair_count;
        if constexpr (std::is_fundamental_v<ValueType>) {
            ss << "<int, " << value_type_name << "> occupies " << pair_size << " bytes (or "
               << max_pair_count << " pairs fit into 4kb page):\n"
               << "\t" << sizeof(int) << " bytes for KEY\n"
               << "\t" << sizeof(ValueType) << " bytes for VALUE\n"
               << "\t" << leaf_node_count << " leaf nodes, each is a 4kb block on a disk\n"
               << "\t" << log100(leaf_node_count) << " is the height of tree";
        } else {
            ss << "<int, " << value_type_name << "> occupies " << pair_size << " bytes (or "
               << max_pair_count << " pairs fit into 4kb page):\n"
               << "\t" << sizeof(int) << " bytes for int\n"
               << "\t" << total_elements * element_size << " bytes for VALUE (ignore type wrapper): "
               << total_elements << " elements per " << element_size << " bytes\n"
               << "\t" << leaf_node_count << " leaf nodes, each is a 4kb block on a disk\n"
               << "\t" << log100(leaf_node_count) << " is the height of tree";
        }
        return ss;
    }
}

using namespace common;

int main() {
    cout << endl;
    cout << "For storage that have about 10^8 different keys and occupies ~ 1TB disk space required:" << endl;
    cout << "\t- key type: int, sizeof(int) = " << sizeof(int) << endl;
    cout << "\t\t1. It can fit ~ 10^8 different keys" << endl;
    cout << "\t\t2. The total size for all keys is about 1GB (is " << to_TB_percentage(sizeof(int) * total_keys) << "% of 1 TB)\n" << endl;

    cout << dump_key_value_info<uint32_t>("uint32_t").str() << endl;
    cout << dump_key_value_info<uint64_t>("uint64_t").str() << endl;
    cout << dump_key_value_info<float>("float").str() << endl;
    cout << dump_key_value_info<double>("double").str() << endl;
    cout << dump_key_value_info<std::string, 30, sizeof(uint32_t)>("std::string").str() << endl;
    cout << dump_key_value_info<uint8_t*, 1000, sizeof(uint8_t)>("uint8_t*").str() << endl;
}