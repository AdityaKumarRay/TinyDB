#include <iostream>
#include <string>
#include <sstream>
#include "row.hpp"
#include "table.hpp"
#include "cursor.hpp"

enum class MetaCommandResult {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND,
    META_COMMAND_EXIT
};

enum class PrepareResult {
    PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
};

enum class ExecuteResult {
    EXECUTE_SUCCESS,
    EXECUTE_DUPLICATE_KEY,
    EXECUTE_TABLE_FULL
};

enum class StatementType {
    STATEMENT_INSERT,
    STATEMENT_SELECT
};

struct Statement {
    StatementType type;
    Row row_to_insert;
};

MetaCommandResult do_meta_command(const std::string& command) {
    if (command == ".exit") {
        return MetaCommandResult::META_COMMAND_EXIT;
    } else {
        return MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND;
    }
}

PrepareResult prepare_insert(const std::string& input, Statement& statement) {
    statement.type = StatementType::STATEMENT_INSERT;

    std::stringstream ss(input);
    std::string keyword;
    int id;
    std::string username;
    std::string email;

    ss >> keyword >> id >> username >> email;

    if (ss.fail()) {
        return PrepareResult::PREPARE_SYNTAX_ERROR;
    }

    if (id < 0) {
        return PrepareResult::PREPARE_NEGATIVE_ID;
    }

    if (username.length() > COLUMN_USERNAME_SIZE || email.length() > COLUMN_EMAIL_SIZE) {
        return PrepareResult::PREPARE_STRING_TOO_LONG;
    }

    statement.row_to_insert.id = static_cast<uint32_t>(id);
    std::strncpy(statement.row_to_insert.username, username.c_str(), COLUMN_USERNAME_SIZE + 1);
    std::strncpy(statement.row_to_insert.email, email.c_str(), COLUMN_EMAIL_SIZE + 1);

    return PrepareResult::PREPARE_SUCCESS;
}

PrepareResult prepare_statement(const std::string& input, Statement& statement) {
    if (input.substr(0, 6) == "insert") {
        return prepare_insert(input, statement);
    }
    if (input.substr(0, 6) == "select") {
        statement.type = StatementType::STATEMENT_SELECT;
        return PrepareResult::PREPARE_SUCCESS;
    }
    return PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT;
}

ExecuteResult execute_insert(Statement& statement, Table& table) {
    void* node = table.pager->get_page(table.root_page_num);
    uint32_t num_cells = *leaf_node_num_cells(node);

    Row* row_to_insert = &statement.row_to_insert;
    uint32_t key_to_insert = row_to_insert->id;
    
    Cursor* cursor = table_find(&table, key_to_insert);

    if (cursor->cell_num < num_cells) {
        uint32_t key_at_index = *leaf_node_key(node, cursor->cell_num);
        if (key_at_index == key_to_insert) {
            delete cursor;
            return ExecuteResult::EXECUTE_DUPLICATE_KEY;
        }
    }
    
    if (num_cells >= LEAF_NODE_MAX_CELLS) {
        delete cursor;
        return ExecuteResult::EXECUTE_TABLE_FULL; // Splitting implemented in Phase 5
    }

    leaf_node_insert(cursor, row_to_insert->id, row_to_insert);

    delete cursor;
    return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult execute_select(Table& table) {
    Cursor* cursor = Cursor::table_start(&table);
    Row row;

    while (!(cursor->end_of_table)) {
        void* source = cursor->value();
        row.deserialize(static_cast<const char*>(source));
        std::cout << "(" << row.id << ", " << row.username << ", " << row.email << ")\n";
        cursor->advance();
    }
    
    delete cursor;
    return ExecuteResult::EXECUTE_SUCCESS;
}

ExecuteResult execute_statement(Statement& statement, Table& table) {
    if (statement.type == StatementType::STATEMENT_INSERT) {
        return execute_insert(statement, table);
    } else if (statement.type == StatementType::STATEMENT_SELECT) {
        return execute_select(table);
    }
    return ExecuteResult::EXECUTE_SUCCESS;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Must supply a database filename.\n";
        exit(EXIT_FAILURE);
    }

    std::string filename = argv[1];
    Table table(filename);

    std::string input_buffer;
    while (true) {
        std::cout << "db > ";
        
        if (!std::getline(std::cin, input_buffer)) {
            break;
        }

        if (input_buffer.empty()) {
            continue;
        }

        if (input_buffer[0] == '.') {
            switch (do_meta_command(input_buffer)) {
                case (MetaCommandResult::META_COMMAND_EXIT):
                    return 0;
                case (MetaCommandResult::META_COMMAND_SUCCESS):
                    continue;
                case (MetaCommandResult::META_COMMAND_UNRECOGNIZED_COMMAND):
                    std::cout << "Unrecognized command '" << input_buffer << "'.\n";
                    continue;
            }
        }

        Statement statement;
        switch (prepare_statement(input_buffer, statement)) {
            case (PrepareResult::PREPARE_SUCCESS):
                break;
            case (PrepareResult::PREPARE_SYNTAX_ERROR):
                std::cout << "Syntax error. Could not parse statement.\n";
                continue;
            case (PrepareResult::PREPARE_STRING_TOO_LONG):
                std::cout << "String is too long.\n";
                continue;
            case (PrepareResult::PREPARE_NEGATIVE_ID):
                std::cout << "ID must be positive.\n";
                continue;
            case (PrepareResult::PREPARE_UNRECOGNIZED_STATEMENT):
                std::cout << "Unrecognized keyword at start of '" << input_buffer << "'.\n";
                continue;
        }

        switch (execute_statement(statement, table)) {
            case (ExecuteResult::EXECUTE_SUCCESS):
                std::cout << "Executed.\n";
                break;
            case (ExecuteResult::EXECUTE_DUPLICATE_KEY):
                std::cout << "Error: Duplicate key.\n";
                break;
            case (ExecuteResult::EXECUTE_TABLE_FULL):
                std::cout << "Error: Table full.\n";
                break;
        }
    }

    return 0;
}
