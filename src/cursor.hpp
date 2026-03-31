#pragma once

#include "table.hpp"
#include "btree.hpp"
#include <memory>

struct Cursor {
    Table* table;
    uint32_t page_num;
    uint32_t cell_num;
    bool end_of_table;

    static std::unique_ptr<Cursor> table_start(Table* tbl) {
        auto cursor = std::make_unique<Cursor>();
        cursor->table = tbl;
        cursor->page_num = tbl->root_page_num;
        cursor->cell_num = 0;

        void* root_node = tbl->pager->get_page(tbl->root_page_num);
        uint32_t num_cells = *leaf_node_num_cells(root_node);
        cursor->end_of_table = (num_cells == 0);

        return cursor;
    }

    static std::unique_ptr<Cursor> table_end(Table* tbl) {
        auto cursor = std::make_unique<Cursor>();
        cursor->table = tbl;
        cursor->page_num = tbl->root_page_num;

        void* root_node = tbl->pager->get_page(tbl->root_page_num);
        uint32_t num_cells = *leaf_node_num_cells(root_node);
        cursor->cell_num = num_cells;
        cursor->end_of_table = true;

        return cursor;
    }

    void advance() {
        void* node = table->pager->get_page(page_num);
        cell_num += 1;
        if (cell_num >= (*leaf_node_num_cells(node))) {
            end_of_table = true;
        }
    }

    void* value() {
        void* page = table->pager->get_page(page_num);
        return leaf_node_value(page, cell_num);
    }
};

inline void leaf_node_insert(Cursor* cursor, uint32_t key, Row* value) {
    void* node = cursor->table->pager->get_page(cursor->page_num);

    uint32_t num_cells = *leaf_node_num_cells(node);
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        // Node full
        std::cout << "Need to implement splitting an internal node.\n";
        exit(EXIT_FAILURE);
    }

    if (cursor->cell_num < num_cells) {
        // Make room for new cell
        for (uint32_t i = num_cells; i > cursor->cell_num; i--) {
            std::memcpy(leaf_node_cell(node, i),
                        leaf_node_cell(node, i - 1),
                        LEAF_NODE_CELL_SIZE);
        }
    }

    *(leaf_node_num_cells(node)) += 1;
    *(leaf_node_key(node, cursor->cell_num)) = key;
    value->serialize(static_cast<char*>(leaf_node_value(node, cursor->cell_num)));
}

inline std::unique_ptr<Cursor> table_find(Table* table, uint32_t key) {
    uint32_t root_page_num = table->root_page_num;
    void* root_node = table->pager->get_page(root_page_num);

    // Assume root is a leaf for now
    uint32_t num_cells = *leaf_node_num_cells(root_node);
    
    auto cursor = std::make_unique<Cursor>();
    cursor->table = table;
    cursor->page_num = root_page_num;

    // Binary search
    uint32_t min_index = 0;
    uint32_t max_index = num_cells;

    while (min_index != max_index) {
        uint32_t index = (min_index + max_index) / 2;
        uint32_t key_at_index = *leaf_node_key(root_node, index);
        
        if (key == key_at_index) {
            cursor->cell_num = index;
            return cursor;
        }
        
        if (key < key_at_index) {
            max_index = index;
        } else {
            min_index = index + 1;
        }
    }

    cursor->cell_num = min_index;
    return cursor;
}
