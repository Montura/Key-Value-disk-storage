## Description

### Volume
  * one or more files on a disk 
  * describes hierarchical structure of `Nodes` that contain key-value pairs
   * the total number of KEYs in all volume nodes can reach ~ 10^8
  * `volume format` has to be **_platform-|architecture- independent_**
  * `sizeof(volume)` >= 10^12

#### Outline:
* [memcache](https://github.com/memcached/memcached) & [Redis](https://github.com/redis/redis/tree/unstable) -> hash table
* [LevelDB](https://github.com/google/leveldb) & [RocksDB](https://github.com/facebook/rocksdb) -> LSM-tree
* [LMDB](https://github.com/LMDB/lmdb/tree/mdb.master/libraries/liblmdb) & [BerkeleyDB](https://github.com/berkeleydb/libdb) -> B+Tree

#### Questions:
1. Volume == B+Tree ?
2. https://github.com/erthink/libfpta

### Storage
* virtual hierarchy structure of `mount nodes`
* arbitrary parts of hierarchical structures from one or several `Volume's` are mounted in the `storage mount nodes`
* the same `volume node` can be `mounted` to multiple `storage mount nodes`
* when several `volume nodes` from one or several `volumes` hit one `storage mount node`:
  * `volume nodes` data is merged
  * if the `volume node arrayPosKey` match, then `volume node data` from the highest priority `volume` is taken
* the total number of `mount nodes` in the `Storage` can reach ~ `10^5` 
* `Storage` should work optimally with a variety of data types: `uint32`, `uint64`, `float`, `double`, `string`, `blob`, ...

#### Questions:
1. Keys are comparable, so the values are -> uint32, uint64, float, double, string, blob, ... ?

### KEYs:
* `add|remove|search|iterate?` (`or put|get`)
* `automatic erasure when the KEY lifetime has expired`

### Implementation details:
* One OS process can simultaneously and independently work with several `storages`, whose `volumes` do not overlap
* Support `32-bit`, `64-bit` architectures
* Support multithreading
  * particularly, providing parallel execution of most operations, including concurrent access to `volume files`
 

#### Questions:
1. multithreading: `SWMR` -> Single Writer Multiple Reader (sync only write ops)