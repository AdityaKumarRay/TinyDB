#pragma once

#include "row.hpp"
#include "pager.hpp"
#include "btree.hpp"
#include <vector>
#include <iostream>
#include <memory>

struct Table {
    uint32_t root_page_num;
    Pager* pager;

    explicit Table(const std::string& filename) {
        pager = new Pager(filename);
        root_page_num = 0;

        if (pager->num_pages == 0) {
            void* root_node = pager->get_page(0);
            initialize_leaf_node(root_node);
        }
    }

    ~Table() {
        if (!pager) return;

        for (uint32_t i = 0; i < pager->num_pages; i++) {
            if (pager->pages[i]) {
                pager->flush(i);
                operator delete(pager->pages[i]);
                pager->pages[i] = nullptr;
            }
        }
        
        delete pager;
    }
};
