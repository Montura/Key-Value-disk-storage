## Key-Value storage C++ library based on BTree structure

This is the C++17 template based header library under Windows/Linux/MacOs to store KEY|VALUES on disk.


#### Verified:
* tested on value types:
    *  `int32_t`
    *  `int64_t`
    *  `float`
    *  `double`
    *  `std::string`
    *  `std::wstring`
    *  `blob {const char*, size}`
* tested on 
    * MacOS (x86-64), compiler Apple clang version 13.0.0 (clang-1300.0.29.3, Mac OS Big Sur v11.5.2)
    * Windows (x86|x86-64), Visual Studio 2019 Version 16.5.0 (cl v19.25.28610.4, Windows 10 Pro)
    * Linux (x86-64), compiler GNU version 10.3.0 (Ubuntu v20.04)

#### TODO:
   * Implement automatic remove for expiring keys (see [Redis impl](https://github.com/redis/redis/blob/a92921da135e38eedd89138e15fe9fd1ffdd9b48/src/expire.c#L98))
   * Implement recovery technique [Write-Ahead-Log](https://people.eecs.berkeley.edu/~kubitron/cs262/handouts/papers/a1-graefe.pdf) 
      * To provide failure atomicity and durability
      * The recovery log describes changes before any in-place updates of the B-tree data structure.
      * *For now all modifications are written to the end of the same file -> file size accordingly grows (drawback)*
   * Specify mapped region usage [behavior](https://github.com/steinwurf/boost/blob/master/boost/interprocess/mapped_region.hpp#L199) to reduce [overhead in memory mapped file I/O](https://www.usenix.org/sites/default/files/conference/protected-files/hotstorage17_slides_choi.pdf)

### Volume structure:

 #### Header (6 bytes):
     - T                        |=> takes 2 bytes (tree degree)
     - KEY_SIZE                 |=> takes 1 byte
     - VALUE_TYPE               |=> takes 1 byte 
        - VALUE_TYPE = 0 for primitives: (u)int32_t, (u)int64_t, float, double
        - VALUE_TYPE = 1 for container of values: (w)string, vectors<T>
        - VALUE_TYPE = 2 for blob [char*]

     - ELEMENT_SIZE             |=> takes 1 byte 
        - ELEMENT_SIZE = sizeof(VALUE_TYPE) for primitives
        - ELEMENT_SIZE = sizeof(VALUE_SUBTYPE) for containers or blob
  
     - ROOT POS                 |=> takes 8 bytes (pos in file)

 #### Node (N bytes):
     - FLAG                     |=> takes 1 byte                 (for "is_leaf")
     - USED_KEYS                |=> takes 2 bytes                (for the number of "active" keys in the node)
     - KEY_POS                  |=> takes (2 * t - 1) * KEY_SIZE (for key positions in file)
     - CHILD_POS                |=> takes (2 * t) * KEY_SIZE     (for key positions in file)

 #### Entry (M bytes):
     - KEY                      |=> takes KEY_SIZE bytes (4 bytes is enough for 10^8 different keys)
     ----------–-----
        - VALUE                 |=> takes ELEMENT_SIZE bytes for primitive VALUE_TYPE
     or
        - NUMBER_OF_ELEMENTS    |=> takes 4 bytes
        - VALUES                |=> takes (ELEMENT_SIZE * NUMBER_OF_ELEMENTS) bytes
     ----------–-----

### Build

#### Requirements:
   - [Boost Iostreams Library](https://www.boost.org/doc/libs/1_76_0/libs/iostreams/doc/index.html)
   - [Boost Thread Library](https://www.boost.org/doc/libs/1_78_0/doc/html/thread.html)
```
cmake . 
```

### Usage exapmle
* an exapmle is in test/test.cpp

```cpp
{
  btree::Storage<int, int> int_storage;
  auto volume = int_storage.open_volume("../int_storage.txt", 2);
  int val = -1;
  volume.set(0, val);
  std::optional<int> opt = volume.get(0);
  assert(opt.value() == val);
}
{
  btree::Storage<int, std::string> str_storage;
  auto volume = str_storage.open_volume("../str_storage.txt", 2);
  std::string val = "abacaba";
  volume.set(0, val);
  std::optional<std::string> opt = volume.get(0);
  assert(opt.value() == val);
}
{
  btree::Storage<int, const char*> blob_storage;
  auto volume = blob_storage.open_volume("../blob_storage.txt", 2);
  int len = 10;
  auto blob = std::make_unique<char*>(new char[len + 1]);
  for (int i = 0; i < len; ++i) {
      (*blob)[i] = (char)(i + 1);
  }
  volume.set(0, *blob, len);

  std::optional<const char*> opt = volume.get(0);
  auto ptr = opt.value();
  for (int i = 0; i < len; ++i) {
      assert(ptr[i] == (*blob)[i]);
  }
}
```

#### MultiThreading usage
* Support **coarse-grained synchronization**:
   * Thread safety is guaranteed for SET|GET|REMOVE operations on the same "VOLUME"
* an exapmle is in test/test.cpp
```cpp
btree::StorageMT<int, int> int_storage;
auto volume = int_storage.open_volume("../mt_int_storage.txt", 100);

int n = 100000;
std::vector<int> keys(n), values(n);
for (int i = 0; i < n; ++i) {
  keys[i] = i; values[i] = -i;
}

ThreadPool tp { 10 };
auto ranges = generate_ranges(n, 10); // ten not-overlapped intervals
// pass volume to ThreadPool

std::vector<std::future<bool>> futures;
for (auto& range : ranges) {
  futures.emplace_back(
          tp.submit([&volume, &keys, &values, &range]() -> bool {
              for (int i = range.first; i < range.second; ++i)
                  volume.set(keys[i], values[i]);
              return true;
          })
  );
}
for (auto& future : futures) {
  future.get();
}
futures.clear();

// check
for (int i = 0; i < n; ++i)
  assert(volume.get(i).value() == -i);

tp.post([&volume, &keys, &n]() {
  for (int i = 0; i < n; ++i) {
      volume.set(keys[i], 0);
  }
});
tp.join();

// check
for (int i = 0; i < n; ++i)
  assert(volume.get(i).value() == 0);

```

#### Links:
   * Overview of data structures for *Key|Value* storage:
      * [МФТИ. Липовский Р.Г. Теория отказоустойчивых распределенных систем](https://mipt.ru/online/algoritmov-i-tekhnologiy/teoriya-ORS.php)
         * TFTDS 0. Модель распределенной системы
            * [примеры систем](https://youtu.be/HJaI4lCgPCs?t=1106)
         * TFTDS 1. Линеаризуемость. Репликация атомарного регистра, алгоритм ABD
            *  [k/v storage](https://youtu.be/FWQ37wvq1OI?t=619)
            *  [реализация k/v storage](https://youtu.be/FWQ37wvq1OI?t=2441)
         * TFTDS. Семинар 2. Локальное хранилище
            *  [B-tree](https://youtu.be/wXoQIh6mvwE?t=2806)
            *  [LSM-tree](https://youtu.be/wXoQIh6mvwE?t=3447)
      *  [Блеск и нищета key-value базы данных LMDB в приложениях для iOS](https://habr.com/ru/company/vk/blog/480850/)
      *  [Understanding Key-Value Store’s Internal Working](https://medium.com/swlh/key-value-pair-database-internals-18f52c36bb70)
      *  [The State of the Storage Engine](https://dzone.com/articles/state-storage-engine)
      *  [B-Tree vs Log-Structured-Merge-Tree](https://tikv.github.io/deep-dive-tikv/key-value-engine/B-Tree-vs-Log-Structured-Merge-Tree.html)
      *  [Btree vs LSM (WiredTiger bench)](https://github.com/wiredtiger/wiredtiger/wiki/Btree-vs-LSM)
      *  [Closing the B-tree vs. LSM-tree Write Amplification Gap on Modern Storage Hardware with Built-in Transparent Compression (WiredTiger article)](https://arxiv.org/pdf/2107.13987.pdf)
   * Mapped files:
      * [Introduction to Memory Mapped IO]( https://towardsdatascience.com/introduction-to-memory-mapped-io-3540454770f7)
      * [Efficient Memory Mapped File I/O for In-Memory File Systems](https://www.usenix.org/sites/default/files/conference/protected-files/hotstorage17_slides_choi.pdf)
   * Overview of BTree impls:
      *  [B-tree library for eventual proposal to Boost](https://github.com/Beman/btree)
      *  [B-tree based on Google's B-tree implementation](https://github.com/Kronuz/cpp-btree)
      *  [Fine-Grained-Locked-B-Tree](https://github.com/MentallyCramped/Fine-Grained-Locked-B-Tree)
      *  [BPlusTree](https://github.com/skyzh/BPlusTree)
      *  [B-tree ИМТО конспект](https://neerc.ifmo.ru/wiki/index.php?title=B-%D0%B4%D0%B5%D1%80%D0%B5%D0%B2%D0%BE)
      *  [Implement Key-Value Store by B-Tree on Linux OS environment](https://medium.com/@pthtantai97/implement-key-value-store-by-btree-5a100a03da3a)
         * [B-Tree impl on Linux OS environment](https://github.com/phamtai97/key-value-store)
         * [Key Value Store using B-Tree](https://github.com/billhcmus/key-value-store)
