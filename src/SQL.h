//
// Created by Sam on 19/05/2018.
//

#ifndef SDB_SQL_H
#define SDB_SQL_H

#include <stddef.h>

union T_Value_t {
    char *string;
    int number;
};
typedef union T_Value_t T_Value;

T_Value *new_t_value();

void free_t_value(T_Value *value);

struct T_Assignment_t {
    char *identifier;
    T_Value *value;
};
typedef struct T_Assignment_t T_Assignment;

T_Assignment *new_t_assignment();

void free_t_assignment(T_Assignment *assignment);

struct T_AssignmentList_t {
    T_Assignment **assignments;
    size_t length;
};
typedef struct T_AssignmentList_t T_AssignmentList;

T_AssignmentList *new_t_assignment_list();

void free_t_assignment_list(T_AssignmentList *assignmentList);

void append_t_assignment_list(T_AssignmentList *list, T_Assignment *assignment);

struct T_FieldList_t {
    char **fields;
    size_t length;
};
typedef struct T_FieldList_t T_FieldList;

T_FieldList *new_t_field_list();

void free_t_field_list(T_FieldList *list);

void append_t_field_list(T_FieldList *list, char *field);

enum T_ColumnOption_t {
    COLOPT_NONE = 0,
    COLOPT_INDEX = 1
};
typedef enum T_ColumnOption_t T_ColumnOption;

enum T_ColumnType_t {
    COLTYPE_NONE,
    COLTYPE_STRING,
    COLTYPE_INT
};
typedef enum T_ColumnType_t T_ColumnType;

struct T_ColumnSpec_t {
    char *identifier;
    T_ColumnType type;
    size_t size;
    T_ColumnOption option;
};
typedef struct T_ColumnSpec_t T_ColumnSpec;

T_ColumnSpec *new_t_column_spec();

void free_t_column_spec(T_ColumnSpec *spec);

struct T_ColumnSpecList_t {
    T_ColumnSpec **columns;
    size_t length;
};
typedef struct T_ColumnSpecList_t T_ColumnSpecList;

T_ColumnSpecList *new_t_column_spec_list();

void free_t_column_spec_list(T_ColumnSpecList *list);

void append_t_column_spec_list(T_ColumnSpecList *list, T_ColumnSpec *spec);

struct T_SelectStmt_t {
    T_FieldList *fields;
    char *tableName;
    T_AssignmentList *where;
};
typedef struct T_SelectStmt_t T_SelectStmt;

T_SelectStmt *new_t_select_stmt();

void free_t_select_stmt(T_SelectStmt *stmt);

struct T_InsertStmt_t {
    char *tableName;
    T_AssignmentList *values;
};
typedef struct T_InsertStmt_t T_InsertStmt;

T_InsertStmt *new_t_insert_stmt();

void free_t_insert_stmt(T_InsertStmt *stmt);

struct T_UpdateStmt_t {
    char *tableName;
    T_AssignmentList *values;
    T_AssignmentList *where;
};
typedef struct T_UpdateStmt_t T_UpdateStmt;

T_UpdateStmt *new_t_update_stmt();

void free_t_update_stmt(T_UpdateStmt *stmt);

struct T_DeleteStmt_t {
    char *tableName;
    T_AssignmentList *where;
};
typedef struct T_DeleteStmt_t T_DeleteStmt;

T_DeleteStmt *new_t_delete_stmt();

void free_t_delete_stmt(T_DeleteStmt *stmt);

struct T_CreateStmt_t {
    char *tableName;
    T_ColumnSpecList *columns;
};
typedef struct T_CreateStmt_t T_CreateStmt;

T_CreateStmt *new_t_create_stmt();

void free_t_create_stmt(T_CreateStmt *spec);

struct T_DropStmt_t {
    char *tableName;
};
typedef struct T_DropStmt_t T_DropStmt;

T_DropStmt *new_t_drop_stmt();

void free_t_drop_stmt(T_DropStmt *stmt);

enum T_StatementType_t {
    STMT_NONE = 0,
    STMT_SELECT,
    STMT_INSERT,
    STMT_UPDATE,
    STMT_DELETE,
    STMT_CREATE,
    STMT_DROP
};
typedef enum T_StatementType_t T_StatementType;

struct T_Statement_t {
    T_StatementType type;
    void *stmt;
};
typedef struct T_Statement_t T_Statement;

T_Statement *new_t_statement();

void free_t_statement(T_Statement *stmt);

struct T_StatementList_t {
    T_Statement **statements;
    size_t length;
};
typedef struct T_StatementList_t T_StatementList;

T_StatementList *new_t_statement_list();

void free_t_statement_list(T_StatementList *list);

void append_t_statement_list(T_StatementList *list, T_Statement *statement);

#endif //SDB_SQL_H
