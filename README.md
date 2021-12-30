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
* tested on 
    * MacOS (x86-64), compiler Apple clang version 13.0.0 (clang-1300.0.29.3)
    * Windows (x86|x86-64), Visual Studio 2019 Version 16.5.0 (cl v19.25.28610.4)

#### Current problems:
* file resizing on Windows

#### Todo-list:
* to add **blob**-value support
* to add **coarse-grained synchronization**
* to test on Linux


### Storage structures:

 #### Header (6 bytes):
     - T                        |=> takes 2 bytes (tree degree)
     - KEY_SIZE                 |=> takes 1 byte
     - VALUE_TYPE               |=> takes 1 byte 
        - VALUE_TYPE = 0 for primitives: (u)int32_t, (u)int64_t, float, double
        - VALUE_TYPE = 1 for container of values: (w)string, vectors<T>
        - VALUE_TYPE = 2 for blob

     - ELEMENT_SIZE             |=> takes 1 byte 
        - ELEMENT_SIZE = sizeof(VALUE_TYPE) for primitives
        - ELEMENT_SIZE = mask from the 8 bits (max 256 bytes) for non-primitives
  
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
```
cmake . 
```

### Usage exapmle 

```cpp
    const int val = 65;
    {
        BTree<int, int> b("../a.txt", 2);
        b.set(0, val);
    }
    {
        BTree<int, int> b("../a.txt", 2);
        auto value = b.get(0);
        assert(value == val);
    }
```

