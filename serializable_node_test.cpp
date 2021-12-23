#include "serializable_node.h"

int main() {
    SerializableNode old_node(8);
    std::ofstream ofs("../output.txt");
    std::ifstream ifs("../output.txt");
    {
        old_node.used_keys = 6;
        old_node.serialize(ofs);
    }

    SerializableNode new_node;
    {
        new_node.deserialize(ifs);
    }


    assert(old_node == new_node);
    return 0;
}