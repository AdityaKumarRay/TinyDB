#pragma once

#include "row.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <vector>

struct Pager {
    int file_descriptor;
    uint32_t file_length;
    uint32_t num_pages;
    void* pages[TABLE_MAX_PAGES];

    explicit Pager(const std::string& filename) {
        file_descriptor = open(filename.c_str(),
                               O_RDWR | O_CREAT,
                               S_IWUSR | S_IRUSR);

        if (file_descriptor == -1) {
            std::cout << "Unable to open file\n";
            exit(EXIT_FAILURE);
        }

        struct stat file_stat;
        if (fstat(file_descriptor, &file_stat) == -1) {
            std::cout << "Unable to get file stats\n";
            exit(EXIT_FAILURE);
        }
        
        file_length = static_cast<uint32_t>(file_stat.st_size);
        num_pages = file_length / PAGE_SIZE;

        if (file_length % PAGE_SIZE != 0) {
            std::cout << "Db file is not a whole number of pages. Corrupt file.\n";
            exit(EXIT_FAILURE);
        }

        for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
            pages[i] = nullptr;
        }
    }

    ~Pager() {
        close(file_descriptor);
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
                lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
                ssize_t bytes_read = read(file_descriptor, page, PAGE_SIZE);
                if (bytes_read == -1) {
                    std::cout << "Error reading file: " << errno << "\n";
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

        off_t offset = lseek(file_descriptor, page_num * PAGE_SIZE, SEEK_SET);
        if (offset == -1) {
            std::cout << "Error seeking: " << errno << "\n";
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_written = write(file_descriptor, pages[page_num], PAGE_SIZE);
        if (bytes_written == -1) {
            std::cout << "Error writing: " << errno << "\n";
            exit(EXIT_FAILURE);
        }
    }
};

inline uint32_t get_unused_page_num(Pager* pager) {
    return pager->num_pages;
}
