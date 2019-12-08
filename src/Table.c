//
// Created by sam on 07/12/2019.
//

#include <malloc.h>
#include <assert.h>
#include "Table.h"

TableIndex *new_table_index() {
    TableIndex *index = malloc(sizeof(TableIndex));
    index->name = NULL;
    index->columnId = 0;
    index->tree = NULL;
    return index;
}

void free_table_index(TableIndex *index) {
    if (index->name != NULL) {
        free(index->name);
    }
    if (index->tree != NULL) {
        free_bplus_tree(index->tree);
    }
    free(index);
}

TableIndexList *new_table_index_list() {
    TableIndexList *list = malloc(sizeof(TableIndexList));
    list->indexes = NULL;
    list->length = 0;
    return list;
}

void free_table_index_list(TableIndexList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free_table_index(list->indexes[i]);
        }
        free(list->indexes);
    }
    free(list);
}

void append_table_index_list(TableIndexList *list, TableIndex *index) {
    list->length++;
    list->indexes = realloc(list->indexes, sizeof(TableIndex *) * list->length);
    list->indexes[list->length - 1] = index;
}

void table_index_list_remove(TableIndexList *list, TableIndex *index) {
    if (list->length == 0) {
        return;
    }
    for (size_t i = 0; i < list->length; i++) {
        if (list->indexes[i] == index) {
            while (i + 1 < list->length) {
                list->indexes[i] = list->indexes[i + 1];
                i++;
            }
            list->length--;
            list->indexes = realloc(list->indexes, sizeof(TableIndex *) * list->length);
            return;
        }
    }
}

TableIndex *table_index_list_get(TableIndexList *list, uint16_t columnId) {
    if (list->length == 0) {
        return NULL;
    }
    for (size_t i = 0; i < list->length; i++) {
        if (list->indexes[i]->columnId == columnId) {
            return list->indexes[i];
        }
    }
}

TableRow *new_table_row() {
    TableRow *row = malloc(sizeof(TableRow));
    row->rowId = 0;
    row->data = NULL;
    row->next = NULL;
    return row;
}

void free_table_row(TableRow *row) {
    if (row->data != NULL) {
        free(row->data);
    }
    free(row);
}

Table *new_table() {
    Table *table = malloc(sizeof(Table));
    table->name = NULL;
    table->columns = NULL;
    table->columnLength = 0;
    table->indexes = new_table_index_list();
    table->rowCount = 0;
    table->rowSize = 0;
    table->rowFirst = NULL;
    return table;
}

void free_table(Table *table) {
    if (table->name != NULL) {
        free(table->name);
    }
    if (table->columns != NULL) {
        for (size_t i = 0; i < table->columnLength; i++) {
            free_column_spec(table->columns[i]);
        }
        free(table->columns);
        table->columnLength = 0;
    }
    if (table->indexes != NULL) {
        free_table_index_list(table->indexes);
    }
    if (table->rowFirst != NULL) {
        TableRow *row = table->rowFirst;
        do {
            TableRow *temp = row;
            row = row->next;
            free_table_row(temp);
        } while (row == NULL);
    }
    free(table);
}

size_t table_determine_row_size(Table *table) {
    size_t minSize = 0;
    for (size_t i = 0; i < table->columnLength; i++) {
        minSize += column_spec_data_size(table->columns[i]);
    }
    size_t rowSize = 0;
    while (minSize > rowSize) rowSize += TABLE_PAGE_SIZE;
    return rowSize;
}

TableRow *table_new_row(Table *table) {
    TableRow *row = new_table_row();
    row->data = calloc(1, table->rowSize);
    if (table->rowFirst == NULL) {
        table->rowFirst = row;
        return row;
    }
    TableRow *lastRow = table->rowFirst;
    while (lastRow->next != NULL) {
        lastRow = lastRow->next;
    }
    lastRow->next = row;
    return row;
}

void table_add_column(Table *table, ColumnSpec *spec) {
    table->columnLength++;
    table->columns = realloc(table->columns, sizeof(ColumnSpec) * table->columnLength);
    table->columns[table->columnLength - 1] = spec;
    table->rowSize = table_determine_row_size(table);
    spec->id = table->columnLength - 1;
    if (spec->id > 0) {
        ColumnSpec *previousColumn = table->columns[spec->id - 1];
        spec->rowOffset = previousColumn->rowOffset + column_spec_data_size(previousColumn);
    } else {
        spec->rowOffset = 0;
    }
}

void* table_row_column_data(Table *table, TableRow *row, uint16_t columnId) {
    assert(columnId < table->columnLength);
    ColumnSpec *columnSpec = table->columns[columnId];
    return row->data + columnSpec->rowOffset;
}
