#pragma once

#include "row.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>

struct Pager {
    std::fstream file;
    uint32_t file_length;
    uint32_t num_pages;
    std::array<void*, TABLE_MAX_PAGES> pages;

    explicit Pager(const std::string& filename) {
        // Try opening existing file
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        
        // If it doesn't exist, create it
        if (!file.is_open()) {
            file.clear(); // Clear error flags
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
            if (!file.is_open()) {
                std::cout << "Unable to open or create file\n";
                exit(EXIT_FAILURE);
            }
        }

        file.seekg(0, std::ios::end);
        file_length = file.tellg();
        file.seekg(0, std::ios::beg);

        num_pages = file_length / PAGE_SIZE;

        if (file_length % PAGE_SIZE != 0) {
            std::cout << "Db file is not a whole number of pages. Corrupt file.\n";
            exit(EXIT_FAILURE);
        }

        pages.fill(nullptr);
    }

    ~Pager() {
        if (file.is_open()) {
            file.close();
        }
    }

    void* get_page(uint32_t page_num) {
        if (page_num >= TABLE_MAX_PAGES) {
            std::cout << "Tried to fetch page number out of bounds. " << page_num << "\n";
            exit(EXIT_FAILURE);
        }

        if (pages[page_num] == nullptr) {
            void* page = operator new(PAGE_SIZE);
            uint32_t num_pages_file = file_length / PAGE_SIZE;

            if (file_length % PAGE_SIZE) {
                num_pages_file += 1;
            }

            if (page_num < num_pages_file) {
                file.seekg(page_num * PAGE_SIZE, std::ios::beg);
                file.read(static_cast<char*>(page), PAGE_SIZE);
                if (file.fail() && !file.eof()) {
                    std::cout << "Error reading file.\n";
                    exit(EXIT_FAILURE);
                }
            }

            pages[page_num] = page;
            
            if (page_num >= num_pages) {
                num_pages = page_num + 1;
            }
        }

        return pages[page_num];
    }

    void flush(uint32_t page_num) {
        if (pages[page_num] == nullptr) {
            std::cout << "Tried to flush null page\n";
            exit(EXIT_FAILURE);
        }

        file.seekp(page_num * PAGE_SIZE, std::ios::beg);
        if (file.fail()) {
            std::cout << "Error seeking.\n";
            exit(EXIT_FAILURE);
        }

        file.write(static_cast<char*>(pages[page_num]), PAGE_SIZE);
        if (file.fail()) {
            std::cout << "Error writing.\n";
            exit(EXIT_FAILURE);
        }
        file.flush();
    }
};

inline uint32_t get_unused_page_num(Pager* pager) {
    return pager->num_pages;
}
