//
// Created by Sam on 19/05/2018.
//

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "SQL.h"
#include "MurmurHash3.h"
#include "random.h"

Value *new_value() {
    Value *value = malloc(sizeof(Value));
    value->type = VALUE_NONE;
    value->integer = 0;
    value->string = NULL;
    return value;
}

void free_value(Value *value) {
    if (value->string != NULL) {
        free(value->string);
    }
    free(value);
}

Comparison *new_comparison() {
    Comparison *comparison = malloc(sizeof(Comparison));
    comparison->identifier = NULL;
    comparison->value = NULL;
    comparison->comp = COMP_NONE;
    return comparison;
}

void free_comparison(Comparison *comparison) {
    if (comparison->value != NULL) {
        free_value(comparison->value);
    }
    if (comparison->identifier != NULL) {
        free(comparison->identifier);
    }
    free(comparison);
}

ComparisonGroup *new_comparision_group() {
    ComparisonGroup *group = malloc(sizeof(ComparisonGroup));
    group->comparisons = NULL;
    group->length = 0;
    return group;
}

void free_comparison_group(ComparisonGroup *group) {
    if (group->length > 0) {
        for (size_t i = 0; i < group->length; i++) {
            free_comparison(group->comparisons[i]);
        }
        free(group->comparisons);
        group->length = 0;
    }
    free(group);
}

void append_comparison_group(ComparisonGroup *group, Comparison *comparison) {
    group->length++;
    group->comparisons = realloc(group->comparisons, sizeof(Assignment *) * group->length);
    group->comparisons[group->length - 1] = comparison;
}

Assignment *new_assignment() {
    Assignment *assignment = malloc(sizeof(Assignment));
    assignment->value = NULL;
    assignment->identifier = NULL;
}

void free_assignment(Assignment *assignment) {
    if (assignment->identifier != NULL) {
        free(assignment->identifier);
    }
    if (assignment->value != NULL) {
        free(assignment->value);
    }
    free(assignment);
}

AssignmentList *new_assignment_list() {
    AssignmentList *assignmentList = malloc(sizeof(AssignmentList));
    assignmentList->assignments = NULL;
    assignmentList->length = 0;
    return assignmentList;
}

void free_assignment_list(AssignmentList *assignmentList) {
    if (assignmentList->length > 0) {
        for (size_t i = 0; i < assignmentList->length; i++) {
            free_assignment(assignmentList->assignments[i]);
        }
        free(assignmentList->assignments);
        assignmentList->length = 0;
    }
    free(assignmentList);
}

void append_assignment_list(AssignmentList *list, Assignment *assignment) {
    list->length++;
    list->assignments = realloc(list->assignments, sizeof(Assignment *) * list->length);
    list->assignments[list->length - 1] = assignment;
}

FieldList *new_field_list() {
    FieldList *list = malloc(sizeof(FieldList));
    list->length = 0;
    list->fields = NULL;
    return list;
}

void free_field_list(FieldList *list) {
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

void append_field_list(FieldList *list, char *field) {
    list->length++;
    list->fields = realloc(list->fields, sizeof(char *) * list->length);
    list->fields[list->length - 1] = field;
}

ColumnSpec *new_column_spec() {
    ColumnSpec *spec = malloc(sizeof(ColumnSpec));
    spec->id = 0;
    spec->rowOffset = 0;
    spec->identifier = NULL;
    spec->option = COLOPT_NONE;
    spec->size = 0;
    spec->type = COLTYPE_NONE;
    return spec;
}

void free_column_spec(ColumnSpec *spec) {
    if (spec->identifier != NULL) {
        free(spec->identifier);
    }
    free(spec);
}

size_t column_spec_data_size(ColumnSpec *spec) {
    switch (spec->type) {
        case COLTYPE_CHAR:
            return (sizeof(char) * spec->size) + 1;
        case COLTYPE_INT:
            return sizeof(uint64_t);
        default:
            return 0;
    }
}

ColumnSpecList *new_column_spec_list() {
    ColumnSpecList *list = malloc(sizeof(ColumnSpecList));
    list->length = 0;
    list->columns = NULL;
    return list;
}

void free_column_spec_list(ColumnSpecList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free_column_spec(list->columns[i]);
        }
        free(list->columns);
        list->columns = NULL;
        list->length = 0;
    }
    free(list);
}

void append_column_spec_list(ColumnSpecList *list, ColumnSpec *spec) {
    list->length++;
    list->columns = realloc(list->columns, sizeof(ColumnSpec *) * list->length);
    list->columns[list->length - 1] = spec;
}

SelectStmt *new_select_stmt() {
    SelectStmt *stmt = malloc(sizeof(SelectStmt));
    stmt->fields = NULL;
    stmt->tableName = NULL;
    stmt->where = NULL;
    return stmt;
}

void free_select_stmt(SelectStmt *stmt) {
    if (stmt->where != NULL) {
        free_comparison_group(stmt->where);
    }
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->fields != NULL) {
        free_field_list(stmt->fields);
    }
    free(stmt);
}

InsertStmt *new_insert_stmt() {
    InsertStmt *stmt = malloc(sizeof(InsertStmt));
    stmt->tableName = NULL;
    stmt->values = NULL;
    return stmt;
}

void free_insert_stmt(InsertStmt *stmt) {
    if (stmt->values != NULL) {
        free_assignment_list(stmt->values);
    }
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    free(stmt);
}

UpdateStmt *new_update_stmt() {
    UpdateStmt *stmt = malloc(sizeof(UpdateStmt));
    stmt->tableName = NULL;
    stmt->values = NULL;
    stmt->where = NULL;
    return stmt;
}

void free_update_stmt(UpdateStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->where != NULL) {
        free_comparison_group(stmt->where);
    }
    if (stmt->values != NULL) {
        free_assignment_list(stmt->values);
    }
    free(stmt);
}

DeleteStmt *new_delete_stmt() {
    DeleteStmt *stmt = malloc(sizeof(DeleteStmt));
    stmt->tableName = NULL;
    stmt->where = NULL;
}

void free_delete_stmt(DeleteStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    if (stmt->where != NULL) {
        free_comparison_group(stmt->where);
    }
    free(stmt);
}

CreateStmt *new_create_stmt() {
    CreateStmt *spec = malloc(sizeof(CreateStmt));
    spec->tableName = NULL;
    spec->columns = NULL;
    return spec;
}

void free_create_stmt(CreateStmt *spec) {
    if (spec->tableName != NULL) {
        free(spec->tableName);
    }
    if (spec->columns != NULL) {
        free_column_spec_list(spec->columns);
    }
    free(spec);
}

DropStmt *new_drop_stmt() {
    DropStmt *stmt = malloc(sizeof(DropStmt));
    stmt->tableName = NULL;
    return stmt;
}

void free_drop_stmt(DropStmt *stmt) {
    if (stmt->tableName != NULL) {
        free(stmt->tableName);
    }
    free(stmt);
}

Statement *new_statement(StatementType type, void *statement) {
    Statement *stmt = malloc(sizeof(Statement));
    stmt->type = type;
    stmt->stmt = statement;
    return stmt;
}

void free_statement(Statement *stmt) {
    switch (stmt->type) {
        case STMT_SELECT:
            free_select_stmt(stmt->stmt);
            break;
        case STMT_INSERT:
            free_insert_stmt(stmt->stmt);
            break;
        case STMT_UPDATE:
            free_update_stmt(stmt->stmt);
            break;
        case STMT_DELETE:
            free_delete_stmt(stmt->stmt);
            break;
        case STMT_CREATE:
            free_create_stmt(stmt->stmt);
            break;
        case STMT_DROP:
            free_drop_stmt(stmt->stmt);
            break;
        default:
            break;
    }
    stmt->stmt = NULL;
    stmt->type = STMT_NONE;
    free(stmt);
}

StatementList *new_statement_list() {
    StatementList *list = malloc(sizeof(StatementList));
    list->length = 0;
    list->statements = NULL;
    return list;
}

void free_statement_list(StatementList *list) {
    if (list->length > 0) {
        for (size_t i = 0; i < list->length; i++) {
            free_statement(list->statements[i]);
        }
        free(list->statements);
        list->statements = NULL;
        list->length = 0;
    }
    free(list);
}

void append_statement_list(StatementList *list, Statement *statement) {
    list->length++;
    list->statements = realloc(list->statements, sizeof(Statement *) * list->length);
    list->statements[list->length - 1] = statement;
}

void print_statement_list(StatementList *list) {

    for (size_t i = 0; i < list->length; i++) {
        Statement *stmt = list->statements[i];
        switch (stmt->type) {
            case STMT_SELECT: {
                SelectStmt *select = stmt->stmt;
                printf("SELECT ");
                print_field_list(select->fields);
                printf(" FROM %s", select->tableName);
                if (select->where->length > 0) {
                    printf(" WHERE ");
                    print_comparison_group(select->where);
                }
            }
                break;
            case STMT_INSERT: {
                InsertStmt *insert = stmt->stmt;
                printf("INSERT INTO %s SET ", insert->tableName);
                print_assignment_list(insert->values);
            }
                break;
            case STMT_UPDATE: {
                UpdateStmt *update = stmt->stmt;
                printf("UPDATE %s SET ", update->tableName);
                print_assignment_list(update->values);
                if (update->where->length > 0) {
                    printf(" WHERE ");
                    print_comparison_group(update->where);
                }
            }
                break;
            case STMT_DELETE: {
                DeleteStmt *delete = stmt->stmt;
                printf("DELETE FROM %s", delete->tableName);
                if (delete->where->length > 0) {
                    printf(" ");
                    print_comparison_group(delete->where);
                }
            }
                break;
            case STMT_CREATE: {
                CreateStmt *create = stmt->stmt;
                printf("CREATE TABLE %s (", create->tableName);
                print_column_spec_list(create->columns);
                printf(")");
            }
                break;
            case STMT_DROP: {
                DropStmt *drop = stmt->stmt;
                printf("DELETE TABLE %s", drop->tableName);
            }
                break;
            default:
                break;
        }
        printf(";\n");
    }

}

void print_field_list(FieldList *list) {
    for (size_t i = 0; i < list->length; i++) {
        printf("%s%s", list->fields[i], i + 1 < list->length ? ", " : "");
    }
}

void print_assignment_list(AssignmentList *list) {
    for (size_t i = 0; i < list->length; i++) {
        Assignment *assignment = list->assignments[i];
        printf("%s=", assignment->identifier);
        print_value(assignment->value);
        if (i + 1 < list->length) {
            printf(", ");
        }
    }
}

void print_comparison_group(ComparisonGroup *group) {
    for (size_t i = 0; i < group->length; i++) {
        Comparison *comparison = group->comparisons[i];
        printf("%s=", comparison->identifier);
        print_value(comparison->value);
        if (i + 1 < group->length) {
            printf(" AND ");
        }
    }
}

void print_column_spec_list(ColumnSpecList *list) {
    for (size_t i = 0; i < list->length; i++) {
        ColumnSpec *spec = list->columns[i];
        if (spec->type == COLTYPE_CHAR) {
            printf("%s CHAR(%ld)", spec->identifier, spec->size);
        } else if (spec->type == COLTYPE_INT) {
            printf("%s INTEGER", spec->identifier);
        }
        if (spec->option == COLOPT_INDEX) {
            printf(" INDEX");
        }
        if (i + 1 < list->length) {
            printf(", ");
        }
    }
}

void print_value(Value *value) {
    if (value->type == VALUE_STRING) {
        printf("'");
        size_t len = strlen(value->string);
        for (size_t i = 0; i < len; i++) {
            char next = value->string[i];
            switch (next) {
                case '\'':
                    printf("\\'");
                    break;
                case '\n':
                    printf("\\n");
                    break;
                case '\r':
                    printf("\\r");
                    break;
                case '\t':
                    printf("\\t");
                    break;
                case '\\':
                    printf("\\\\");
                    break;
                default:
                    printf("%c", next);
                    break;
            }
        }
        printf("'");
    } else if (value->type == VALUE_INTEGER) {
        printf("%ld", value->integer);
    }
}