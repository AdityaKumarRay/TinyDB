# TinyDB

A minimal, portable, SQLite-like relational database engine written in C++ from scratch.

This project was built as an educational exploration into database internals. It strips away complex features (like SQL parsing, networking, concurrency, and transactions) to focus purely on the core mechanics of storing and retrieving data using a B-Tree structure on disk.

## ✨ Features

- **Interactive REPL**: A command-line interface for executing meta-commands and basic SQL-like statements.
- **On-Disk Persistence**: Data is saved to a local `.db` file and loaded on startup using a custom Pager.
- **B-Tree Storage Engine**: Rows are serialized and stored inside dynamically split memory pages using B-Tree Node layouts.
- **Cursor Abstraction**: Cursor-based table scanning and sequential row retrieval.
- **Modern C++**: Written strictly using the C++ Standard Library (`<fstream>`, `std::unique_ptr`, `<array>`) with no external dependencies for maximum portability and safety.

## 🏗️ Architecture

TinyDB is conceptually divided into several components mimicking a real RDBMS:

1. **Frontend / REPL (`main.cpp`)**: Parses standard input into statements and executes meta-commands.
2. **Virtual Machine (`cursor.hpp`)**: Executes the parsed statements using Cursors to traverse data. 
3. **B-Tree (`btree.hpp`)**: Maintains the node layout directly in memory (Leaf Nodes / Internal Nodes), calculates offsets natively, and maintains key ordering.
4. **Pager (`pager.hpp`)**: Translates 4KB page requests from the B-Tree into standard `<fstream>` disk I/O operations. Serves as a transparent disk-caching layer.
5. **Row Serialization (`row.hpp`)**: Handles converting tabular C++ structs into flat byte sequences natively.

## 🚀 Getting Started

### Prerequisites
- A C++17/C++20 compatible compiler (GCC, Clang, or MSVC)
- CMake (3.10+)

### Building
```bash
mkdir build && cd build
cmake ..
make
```

### Usage
Run the database by providing a file path to persist your data (if the file does not exist, it will be automatically created):

```bash
./tinydb mydata.db
```

Once inside the REPL, you can run basic queries. The schema is strictly fixed to: `id (int)`, `username (string32)`, `email (string255)`.

```text
db > insert 1 alice alice@example.com
Executed.
db > insert 2 bob bob@example.com
Executed.
db > select
(1, alice, alice@example.com)
(2, bob, bob@example.com)
Executed.
db > .exit
```

Data is transparently saved to `mydata.db`. You can safely exit and re-run the database, and `select` will accurately scan the previously populated B-Tree.

## 🎯 Project Scope & Non-Goals

This database intentionally lacks production features for the sake of simplicity and educational value:
- **No SQL Parser**: Commands are strictly formatted (`insert [id] [user] [email]`).
- **Fixed Schema**: Supporting dynamic typing and varying column sizes requires a metadata catalog that distracts from core paging logic.
- **Single Threaded**: No concurrency, locks, or transaction handling (ACID properties).
