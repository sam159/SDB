//
// Created by Sam on 19/05/2018.
//

#include <malloc.h>
#include "SQL.h"

T_Value *new_t_value() {
    T_Value *value = malloc(sizeof(T_Value));
    value->number = 0;
    value->string = NULL;
    return value;
}

void free_t_value(T_Value *value) {
    if (value->string != NULL) {
        free(value->string);
    }
    free(value);
}

T_Assignment *new_t_assignment() {
    T_Assignment *assignment = malloc(sizeof(T_Assignment));
    assignment->value = NULL;
    assignment->identifier = NULL;
}

void free_t_assignment(T_Assignment *assignment) {
    if (assignment->identifier != NULL) {
        free(assignment->identifier);
    }
    if (assignment->value != NULL) {
        free(assignment->value);
    }
    free(assignment);
}

T_AssignmentList *new_t_assignment_list() {
    T_AssignmentList *assignmentList = malloc(sizeof(T_AssignmentList));
    assignmentList->assignments = NULL;
    assignmentList->length = 0;
    return assignmentList;
}

void free_t_assignment_list(T_AssignmentList *assignmentList) {
    if (assignmentList->length > 0) {
        for (size_t i = 0; i < assignmentList->length; i++) {
            free_t_assignment(assignmentList->assignments[i]);
        }
        free(assignmentList->assignments);
        assignmentList->length = 0;
    }
    free(assignmentList);
}

void append_t_assignment_list(T_AssignmentList *list, T_Assignment *assignment) {
    list->length++;
    list->assignments = realloc(list->assignments, sizeof(T_Assignment) * list->length);
    list->assignments[list->length - 1] = assignment;
}

T_FieldList *new_t_field_list() {
    T_FieldList *list = malloc(sizeof(T_FieldList));
    list->length = 0;
    list->fields = NULL;
    return list;
}

void free_t_field_list(T_FieldList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free(list->fields[i]);
        }
        free(list->fields);
        list->length = 0;
        list->fields = NULL;
    }
    free(list);
}

void append_t_field_list(T_FieldList *list, char *field) {
    list->length++;
    list->fields = realloc(list->fields, sizeof(char *) * list->length);
    list->fields[list->length - 1] = field;
}

T_ColumnSpec *new_t_column_spec() {
    T_ColumnSpec *spec = malloc(sizeof(T_ColumnSpec));
    spec->identifier = NULL;
    spec->option = COLOPT_NONE;
    spec->size = 0;
    spec->type = COLTYPE_NONE;
    return spec;
}

void free_t_column_spec(T_ColumnSpec *spec) {
    if (spec->identifier != NULL) {
        free(spec->identifier);
    }
    free(spec);
}

T_ColumnSpecList *new_t_column_spec_list() {
    T_ColumnSpecList *list = malloc(sizeof(T_ColumnSpecList));
    list->length = 0;
    list->columns = NULL;
    return list;
}

void free_t_column_spec_list(T_ColumnSpecList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free_t_column_spec(list->columns[i]);
        }
        free(list->columns);
        list->columns = NULL;
        list->length = 0;
    }
    free(list);
}

void append_t_column_spec_list(T_ColumnSpecList *list, T_ColumnSpec *spec) {
    list->length++;
    list->columns = realloc(list->columns, sizeof(T_ColumnSpec) * list->length);
    list->columns[list->length - 1] = spec;
}

T_SelectStmt *new_t_select_stmt() {
    T_SelectStmt *stmt = malloc(sizeof(T_SelectStmt));
    stmt->fields = NULL;
    stmt->tableName = NULL;
    stmt->where = NULL;
    return stmt;
}

void free_t_select_stmt(T_SelectStmt *stmt) {
    if (stmt->where != NULL) {
        free_t_assignment_list(stmt->where);
    }
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->fields != NULL) {
        free_t_field_list(stmt->fields);
    }
    free(stmt);
}

T_InsertStmt *new_t_insert_stmt() {
    T_InsertStmt *stmt = malloc(sizeof(T_InsertStmt));
    stmt->tableName = NULL;
    stmt->values = NULL;
    return stmt;
}

void free_t_insert_stmt(T_InsertStmt *stmt) {
    if (stmt->values != NULL) {
        free_t_assignment_list(stmt->values);
    }
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    free(stmt);
}

T_UpdateStmt *new_t_update_stmt() {
    T_UpdateStmt *stmt = malloc(sizeof(T_UpdateStmt));
    stmt->tableName = NULL;
    stmt->values = NULL;
    stmt->where = NULL;
    return stmt;
}

void free_t_update_stmt(T_UpdateStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->where != NULL) {
        free_t_assignment_list(stmt->where);
    }
    if (stmt->values != NULL) {
        free_t_assignment_list(stmt->values);
    }
    free(stmt);
}

T_DeleteStmt *new_t_delete_stmt() {
    T_DeleteStmt *stmt = malloc(sizeof(T_DeleteStmt));
    stmt->tableName = NULL;
    stmt->where = NULL;
}

void free_t_delete_stmt(T_DeleteStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->where != NULL) {
        free_t_assignment_list(stmt->where);
    }
    free(stmt);
}

T_CreateStmt *new_t_create_stmt() {
    T_CreateStmt *spec = malloc(sizeof(T_CreateStmt));
    spec->tableName = NULL;
    spec->columns = NULL;
    return spec;
}

void free_t_create_stmt(T_CreateStmt *spec) {
    if (spec->tableName != NULL) {
        free(spec->tableName);
    }
    if (spec->columns != NULL) {
        free_t_column_spec_list(spec->columns);
    }
    free(spec);
}

T_DropStmt *new_t_drop_stmt() {
    T_DropStmt *stmt = malloc(sizeof(T_DropStmt));
    stmt->tableName = NULL;
    return stmt;
}

void free_t_drop_stmt(T_DropStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    free(stmt);
}

T_Statement *new_t_statement() {
    T_Statement *stmt = malloc(sizeof(T_Statement));
    stmt->type = STMT_NONE;
    stmt->stmt = NULL;
    return stmt;
}

void free_t_statement(T_Statement *stmt) {
    switch (stmt->type) {
        case STMT_SELECT:
            free_t_select_stmt(stmt->stmt);
            break;
        case STMT_INSERT:
            free_t_insert_stmt(stmt->stmt);
            break;
        case STMT_UPDATE:
            free_t_update_stmt(stmt->stmt);
            break;
        case STMT_DELETE:
            free_t_delete_stmt(stmt->stmt);
            break;
        case STMT_CREATE:
            free_t_create_stmt(stmt->stmt);
            break;
        case STMT_DROP:
            free_t_drop_stmt(stmt->stmt);
            break;
        default:
            break;
    }
    stmt->stmt = NULL;
    stmt->type = STMT_NONE;
    free(stmt);
}

T_StatementList *new_t_statement_list() {
    T_StatementList *list = malloc(sizeof(T_StatementList));
    list->length = 0;
    list->statements = NULL;
    return list;
}

void free_t_statement_list(T_StatementList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free_t_statement(list->statements[i]);
        }
        free(list->statements);
        list->statements = NULL;
        list->length = 0;
    }
    free(list);
}

void append_t_statement_list(T_StatementList *list, T_Statement *statement) {
    list->length++;
    list->statements = realloc(list->statements, sizeof(T_Statement) * list->length);
    list->statements[list->length - 1] = statement;
}