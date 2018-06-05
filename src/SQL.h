//
// Created by Sam on 19/05/2018.
//

#ifndef SDB_SQL_H
#define SDB_SQL_H

#include <stddef.h>

union Value_t {
    char *string;
    int number;
};
typedef union Value_t Value;

Value *new_value();

void free_value(Value *value);

enum Comparator_t {
    COMP_NONE,
    COMP_EQ,
    COMP_NEQ
};
typedef enum Comparator_t Comparator;

struct Comparison_t {
    char *identifier;
    Comparator comp;
    Value *value;
};
typedef struct Comparison_t Comparison;

Comparison *new_comparison();
void free_comparison(Comparison *comparison);

struct ComparisonGroup_t {
    Comparison **comparisons;
    size_t length;
};
typedef struct ComparisonGroup_t ComparisonGroup;

ComparisonGroup *new_comparision_group();
void free_comparison_group(ComparisonGroup* group);
void append_comparison_group(ComparisonGroup *group, Comparison *comparison);

struct Assignment_t {
    char *identifier;
    Value *value;
};
typedef struct Assignment_t Assignment;

Assignment *new_assignment();

void free_assignment(Assignment *assignment);

struct AssignmentList_t {
    Assignment **assignments;
    size_t length;
};
typedef struct AssignmentList_t AssignmentList;

AssignmentList *new_assignment_list();

void free_assignment_list(AssignmentList *assignmentList);

void append_assignment_list(AssignmentList *list, Assignment *assignment);

struct FieldList_t {
    char **fields;
    size_t length;
};
typedef struct FieldList_t FieldList;

FieldList *new_field_list();

void free_field_list(FieldList *list);

void append_field_list(FieldList *list, char *field);

enum ColumnOption_t {
    COLOPT_NONE = 0,
    COLOPT_INDEX = 1
};
typedef enum ColumnOption_t ColumnOption;

enum ColumnType_t {
    COLTYPE_NONE,
    COLTYPE_STRING,
    COLTYPE_INT
};
typedef enum ColumnType_t ColumnType;

struct ColumnSpec_t {
    char *identifier;
    ColumnType type;
    size_t size;
    ColumnOption option;
};
typedef struct ColumnSpec_t ColumnSpec;

ColumnSpec *new_column_spec();

void free_column_spec(ColumnSpec *spec);

struct ColumnSpecList_t {
    ColumnSpec **columns;
    size_t length;
};
typedef struct ColumnSpecList_t ColumnSpecList;

ColumnSpecList *new_column_spec_list();

void free_column_spec_list(ColumnSpecList *list);

void append_column_spec_list(ColumnSpecList *list, ColumnSpec *spec);

struct SelectStmt_t {
    FieldList *fields;
    char *tableName;
    AssignmentList *where;
};
typedef struct SelectStmt_t SelectStmt;

SelectStmt *new_select_stmt();

void free_select_stmt(SelectStmt *stmt);

struct InsertStmt_t {
    char *tableName;
    AssignmentList *values;
};
typedef struct InsertStmt_t InsertStmt;

InsertStmt *new_insert_stmt();

void free_insert_stmt(InsertStmt *stmt);

struct UpdateStmt_t {
    char *tableName;
    AssignmentList *values;
    AssignmentList *where;
};
typedef struct UpdateStmt_t UpdateStmt;

UpdateStmt *new_update_stmt();

void free_update_stmt(UpdateStmt *stmt);

struct DeleteStmt_t {
    char *tableName;
    AssignmentList *where;
};
typedef struct DeleteStmt_t DeleteStmt;

DeleteStmt *new_delete_stmt();

void free_delete_stmt(DeleteStmt *stmt);

struct CreateStmt_t {
    char *tableName;
    ColumnSpecList *columns;
};
typedef struct CreateStmt_t CreateStmt;

CreateStmt *new_create_stmt();

void free_create_stmt(CreateStmt *spec);

struct DropStmt_t {
    char *tableName;
};
typedef struct DropStmt_t DropStmt;

DropStmt *new_drop_stmt();

void free_drop_stmt(DropStmt *stmt);

enum StatementType_t {
    STMT_NONE = 0,
    STMT_SELECT,
    STMT_INSERT,
    STMT_UPDATE,
    STMT_DELETE,
    STMT_CREATE,
    STMT_DROP
};
typedef enum StatementType_t StatementType;

struct Statement_t {
    StatementType type;
    void *stmt;
};
typedef struct Statement_t Statement;

Statement *new_statement();

void free_statement(Statement *stmt);

struct StatementList_t {
    Statement **statements;
    size_t length;
};
typedef struct StatementList_t StatementList;

StatementList *new_statement_list();

void free_statement_list(StatementList *list);

void append_statement_list(StatementList *list, Statement *statement);

#endif //SDB_SQL_H
