#pragma once

#include "row.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>

enum class NodeType {
    NODE_INTERNAL,
    NODE_LEAF
};

/* Common Node Header */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE = NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/* Leaf Node Header */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + LEAF_NODE_NUM_CELLS_SIZE;

/* Leaf Node Body */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET = LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS = LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;

/* Internal Node Header */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET = INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE + INTERNAL_NODE_NUM_KEYS_SIZE + INTERNAL_NODE_RIGHT_CHILD_SIZE;

/* Internal Node Body */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE = INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;

/* Accessors */
inline uint32_t* leaf_node_num_cells(void* node) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(node) + LEAF_NODE_NUM_CELLS_OFFSET);
}

inline void* leaf_node_cell(void* node, uint32_t cell_num) {
    return static_cast<char*>(node) + LEAF_NODE_HEADER_SIZE + cell_num * LEAF_NODE_CELL_SIZE;
}

inline uint32_t* leaf_node_key(void* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t*>(leaf_node_cell(node, cell_num));
}

inline void* leaf_node_value(void* node, uint32_t cell_num) {
    return static_cast<char*>(leaf_node_cell(node, cell_num)) + LEAF_NODE_KEY_SIZE;
}

inline NodeType get_node_type(void* node) {
    uint8_t value = *static_cast<uint8_t*>(node);
    return static_cast<NodeType>(value);
}

inline void set_node_type(void* node, NodeType type) {
    uint8_t value = static_cast<uint8_t>(type);
    *static_cast<uint8_t*>(node) = value;
}

inline bool is_node_root(void* node) {
    uint8_t value = *(static_cast<uint8_t*>(node) + IS_ROOT_OFFSET);
    return static_cast<bool>(value);
}

inline void set_node_root(void* node, bool is_root) {
    uint8_t value = static_cast<uint8_t>(is_root);
    *(static_cast<uint8_t*>(node) + IS_ROOT_OFFSET) = value;
}

inline uint32_t* node_parent(void* node) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(node) + PARENT_POINTER_OFFSET);
}

inline void initialize_leaf_node(void* node) {
    set_node_type(node, NodeType::NODE_LEAF);
    set_node_root(node, false);
    *leaf_node_num_cells(node) = 0;
}

inline uint32_t* internal_node_num_keys(void* node) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(node) + INTERNAL_NODE_NUM_KEYS_OFFSET);
}

inline uint32_t* internal_node_right_child(void* node) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(node) + INTERNAL_NODE_RIGHT_CHILD_OFFSET);
}

inline uint32_t* internal_node_cell(void* node, uint32_t cell_num) {
    return reinterpret_cast<uint32_t*>(static_cast<char*>(node) + INTERNAL_NODE_HEADER_SIZE + cell_num * INTERNAL_NODE_CELL_SIZE);
}

inline uint32_t* internal_node_child(void* node, uint32_t child_num) {
    uint32_t num_keys = *internal_node_num_keys(node);
    if (child_num > num_keys) {
        std::cout << "Tried to access child_num " << child_num << " > num_keys " << num_keys << "\n";
        exit(EXIT_FAILURE);
    } else if (child_num == num_keys) {
        return internal_node_right_child(node);
    } else {
        return internal_node_cell(node, child_num);
    }
}

inline uint32_t* internal_node_key(void* node, uint32_t key_num) {
    return reinterpret_cast<uint32_t*>(reinterpret_cast<char*>(internal_node_cell(node, key_num)) + INTERNAL_NODE_CHILD_SIZE);
}

inline void initialize_internal_node(void* node) {
    set_node_type(node, NodeType::NODE_INTERNAL);
    set_node_root(node, false);
    *internal_node_num_keys(node) = 0;
}


const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

inline uint32_t get_node_max_key(void* node) {
    switch (get_node_type(node)) {
        case NodeType::NODE_INTERNAL:
            return *internal_node_key(node, *internal_node_num_keys(node) - 1);
        case NodeType::NODE_LEAF:
            return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
    }
    return 0; 
}
