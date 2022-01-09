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
    *  `blob [const char*, blob_size]`
* tested on 
    * MacOS (x86-64), compiler Apple clang version 13.0.0 (clang-1300.0.29.3, Mac OS Big Sur v11.5.2)
    * Windows (x86|x86-64), Visual Studio 2019 Version 16.5.0 (cl v19.25.28610.4, Windows 10 Pro)
    * Linux (x86-64), compiler GNU version 10.3.0 (Ubuntu v20.04)

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

```cpp
    Storage<int32_t, int32_t> s;
    
    auto path = "../a.txt";
    int tree_order = 2;
    
    const int val = 65;
    {
        auto v = s.open_volume(path, tree_order);
        v.set(0, val);
        assert(v.get(0) == val);
    }
    {
        auto v = s.open_volume(path, tree_order);
        assert(v.get(0) == val);
    }
```

#### MultiThreading usage
* Support **coarse-grained synchronization**:
   * Thread safety is guaranteed for SET|GET|REMOVE operations on the same "VOLUME"
```cpp
    StorageMT<int32_t, int32_t> s_mt;
    
    auto path = "../a.txt";
    int tree_order = 2;
    
    auto v_mt = s_mt.open_volume(path, tree_order);

```
