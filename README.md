# TinyDB

A minimal SQLite-like relational database in C++ inspired by "Let's Build a Simple Database".

## Features
- B-Tree backed storage (leaf nodes).
- Cursors for scanning and inserting.
- On-disk persistence.

## Build
```bash
mkdir build && cd build
cmake ..
make
```

## Run
```bash
./tinydb file.db
```
