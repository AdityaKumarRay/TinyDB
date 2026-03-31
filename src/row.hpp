#pragma once

#include <cstdint>
#include <cstring>
#include <string>

const uint32_t COLUMN_USERNAME_SIZE = 32;
const uint32_t COLUMN_EMAIL_SIZE = 255;

struct Row {
    uint32_t id;
    char username[COLUMN_USERNAME_SIZE + 1];
    char email[COLUMN_EMAIL_SIZE + 1];

    void serialize(char* destination) const {
        std::memcpy(destination, &id, sizeof(id));
        std::memcpy(destination + sizeof(id), username, COLUMN_USERNAME_SIZE);
        std::memcpy(destination + sizeof(id) + COLUMN_USERNAME_SIZE, email, COLUMN_EMAIL_SIZE);
    }

    void deserialize(const char* source) {
        std::memcpy(&id, source, sizeof(id));
        std::memcpy(username, source + sizeof(id), COLUMN_USERNAME_SIZE);
        std::memcpy(email, source + sizeof(id) + COLUMN_USERNAME_SIZE, COLUMN_EMAIL_SIZE);
        // Ensure null termination safely:
        username[COLUMN_USERNAME_SIZE] = '\0';
        email[COLUMN_EMAIL_SIZE] = '\0';
    }
};

const uint32_t ID_SIZE = sizeof(uint32_t);
const uint32_t USERNAME_SIZE = COLUMN_USERNAME_SIZE;
const uint32_t EMAIL_SIZE = COLUMN_EMAIL_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
const uint32_t ROWS_PER_PAGE = PAGE_SIZE / ROW_SIZE;
const uint32_t TABLE_MAX_PAGES = 100;
const uint32_t TABLE_MAX_ROWS = ROWS_PER_PAGE * TABLE_MAX_PAGES;
