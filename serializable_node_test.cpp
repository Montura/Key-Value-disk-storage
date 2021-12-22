#include "serializable_node.h"

int main() {
    SerializableNode old_node(10);
    {
        std::ofstream ofs("../output.txt");
        old_node.serialize(ofs);
    }

    SerializableNode new_node;
    {
        std::ifstream ifs("../output.txt");
        new_node.deserialize(ifs);
    }


    assert(old_node == new_node);
    return 0;
}