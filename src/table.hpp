#pragma once

#include "row.hpp"
#include <vector>
#include <iostream>

struct Table {
    uint32_t num_rows;
    void* pages[TABLE_MAX_PAGES];

    Table() {
        num_rows = 0;
        for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
            pages[i] = nullptr;
        }
    }

    ~Table() {
        for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
            if (pages[i]) {
                delete[] static_cast<char*>(pages[i]);
            }
        }
    }

    void* row_slot(uint32_t row_num) {
        uint32_t page_num = row_num / ROWS_PER_PAGE;
        if (!pages[page_num]) {
            pages[page_num] = new char[PAGE_SIZE]; // Allocate if empty
        }
        uint32_t row_offset = row_num % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * ROW_SIZE;
        return static_cast<char*>(pages[page_num]) + byte_offset;
    }
};
