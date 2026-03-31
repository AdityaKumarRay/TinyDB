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

inline void initialize_leaf_node(void* node) {
    *leaf_node_num_cells(node) = 0;
}

