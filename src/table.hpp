#pragma once

#include "row.hpp"
#include "pager.hpp"
#include <vector>
#include <iostream>
#include <memory>

struct Table {
    uint32_t num_rows;
    Pager* pager;

    explicit Table(const std::string& filename) {
        pager = new Pager(filename);
        num_rows = pager->file_length / ROW_SIZE;
    }

    ~Table() {
        if (!pager) return;

        uint32_t num_full_pages = num_rows / ROWS_PER_PAGE;
        
        for (uint32_t i = 0; i < num_full_pages; i++) {
            if (pager->pages[i]) {
                pager->flush(i);
                operator delete(pager->pages[i]);
                pager->pages[i] = nullptr;
            }
        }

        uint32_t num_additional_rows = num_rows % ROWS_PER_PAGE;
        if (num_additional_rows > 0) {
            uint32_t page_num = num_full_pages;
            if (pager->pages[page_num]) {
                pager->flush(page_num);
                operator delete(pager->pages[page_num]);
                pager->pages[page_num] = nullptr;
            }
        }
        
        delete pager;
    }

    void* row_slot(uint32_t row_num) {
        uint32_t page_num = row_num / ROWS_PER_PAGE;
        void* page = pager->get_page(page_num);
        uint32_t row_offset = row_num % ROWS_PER_PAGE;
        uint32_t byte_offset = row_offset * ROW_SIZE;
        return static_cast<char*>(page) + byte_offset;
    }
};
