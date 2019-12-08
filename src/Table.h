//
// Created by sam on 07/12/2019.
//

#ifndef SDB_TABLE_H
#define SDB_TABLE_H

#include "SQL.h"
#include "bplus_tree.h"

#define TABLE_PAGE_SIZE 4096

struct TableIndex_t {
    char *name;
    uint16_t columnId;
    BPlusTree *tree;
};
typedef struct TableIndex_t TableIndex;

TableIndex *new_table_index();

void free_table_index(TableIndex *index);

struct TableIndexList_t {
    TableIndex **indexes;
    size_t length;
};
typedef struct TableIndexList_t TableIndexList;

TableIndexList *new_table_index_list();

void free_table_index_list(TableIndexList *list);

void append_table_index_list(TableIndexList *list, TableIndex *index);

void table_index_list_remove(TableIndexList *list, TableIndex *index);

TableIndex *table_index_list_get(TableIndexList *list, uint16_t columnId);

struct TableRow_t {
    uint64_t rowId;
    struct TableRow_t *next;
    void *data;
};
typedef struct TableRow_t TableRow;

TableRow *new_table_row();

void free_table_row(TableRow *row);

struct Table_t {
    char *name;
    ColumnSpec **columns;
    size_t columnLength;
    TableIndexList *indexes;
    size_t rowSize;
    size_t rowCount;
    TableRow *rowFirst;
};
typedef struct Table_t Table;

Table *new_table();

void free_table(Table *table);

void table_add_column(Table *table, ColumnSpec *spec);

size_t table_determine_row_size(Table *table);

TableRow *table_new_row(Table *table);

void* table_row_column_data(Table *table, TableRow *row, uint16_t columnId);

#endif //SDB_TABLE_H
