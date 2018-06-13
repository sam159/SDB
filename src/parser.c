//
// Created by Sam on 07/06/2018.
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "parser.h"

char *parser_status_to_str(ParseStatus status) {
#define X(t) case t: return #t;

    switch (status) {
        PARSE_STATUS_LIST
        default:
            return "UNKNOWN";
    }

#undef X
}

char *parser_node_type_to_str(ParserNodeType nodeType) {
#define X(t) case t: return #t;

    switch (nodeType) {
        PARSE_NODE_TYPE_LIST
        default:
            return "UNKNOWN";
    }

#undef X
}

ParserNode *new_parser_node(ParserNodeType type, ScannerToken *token) {
    ParserNode *node = malloc(sizeof(ParserNode));
    node->parent = NULL;
    node->type = type;
    node->phase = 0;
    node->token = token;
    node->children = NULL;
    node->childrenLength = 0;
    return node;
}

void free_parser_node(ParserNode *node) {
    if (node->token != NULL) {
        free_scanner_token(node->token);
    }
    if (node->childrenLength > 0) {
        for (size_t i = 0; i < node->childrenLength; i++) {
            free_parser_node(node->children[i]);
        }
        free(node->children);
    }
    free(node);
}

void append_parser_node(ParserNode *node, ParserNode *child) {
    node->childrenLength++;
    node->children = realloc(node->children, sizeof(ParserNode *) * node->childrenLength);
    node->children[node->childrenLength - 1] = child;
    child->parent = node;
}

Parser *new_parser() {
    Parser *result = malloc(sizeof(Parser));
    result->status = PARSESTATE_NONE;
    result->errMsg = NULL;
    return result;
}

void free_parser(Parser *parser) {
    if (parser->errMsg != NULL) {
        free(parser->errMsg);
    }
    free(parser);
}

StatementList *parser_node_convert(ParserNode *node) {
    if (node->type != NODE_STATEMENT_LIST) {
        return NULL;
    }

    StatementList *list = new_statement_list();

    size_t i, k;

    for (i = 0; i < node->childrenLength; i++) {
        ParserNode *child = node->children[i];
        switch (child->type) {
            case NODE_SELECT_STMT: {
                SelectStmt *select = new_select_stmt();
                append_statement_list(list, new_statement(STMT_SELECT, select));
                select->fields = new_field_list();
                select->tableName = strdup(child->children[1]->token->valueStr);
                for (k = 0; k < child->children[0]->childrenLength; k++) {
                    ParserNode *fieldNode = child->children[0]->children[k];
                    append_field_list(select->fields, strdup(fieldNode->token->valueStr));
                }
                if (child->childrenLength == 3) {
                    select->where = parser_node_convert_comparison_group(child->children[2]);
                } else {
                    select->where = new_comparision_group();
                }
            }
                break;
            case NODE_INSERT_STMT: {
                InsertStmt *insert = new_insert_stmt();
                append_statement_list(list, new_statement(STMT_INSERT, insert));
                insert->tableName = strdup(child->children[0]->token->valueStr);
                insert->values = parser_node_convert_assignment_list(child->children[1]);
            }
                break;
            case NODE_UPDATE_STMT: {
                UpdateStmt *update = new_update_stmt();
                append_statement_list(list, new_statement(STMT_UPDATE, update));
                update->tableName = strdup(child->children[0]->token->valueStr);
                update->values = parser_node_convert_assignment_list(child->children[1]);
                if (child->childrenLength == 3) {
                    update->where = parser_node_convert_comparison_group(child->children[2]);
                } else {
                    update->where = new_comparision_group();
                }
            }
                break;
            case NODE_DELETE_STMT: {
                DeleteStmt *delete = new_delete_stmt();
                append_statement_list(list, new_statement(STMT_DELETE, delete));
                delete->tableName = strdup(child->children[0]->token->valueStr);
                if (child->childrenLength == 2) {
                    delete->where = parser_node_convert_comparison_group(child->children[1]);
                } else {
                    delete->where = new_comparision_group();
                }
            }
                break;
            case NODE_CREATE_STMT: {
                CreateStmt *create = new_create_stmt();
                append_statement_list(list, new_statement(STMT_CREATE, create));
                create->tableName = strdup(child->children[0]->token->valueStr);
                create->columns = parser_node_convert_column_spec_list(child->children[1]);
            }
                break;
            case NODE_DROP_STMT: {
                DropStmt *drop= new_drop_stmt();
                append_statement_list(list, new_statement(STMT_DROP, drop));
                drop->tableName = strdup(child->children[0]->token->valueStr);
            }
                break;
            default:
                break;
        }
    }

    return list;
}

ColumnSpecList *parser_node_convert_column_spec_list(ParserNode *node) {
    ColumnSpecList *list = new_column_spec_list();
    for (size_t i = 0; i < node->childrenLength; i++) {
        ParserNode* specNode = node->children[i];
        ColumnSpec *spec = new_column_spec();
        append_column_spec_list(list, spec);
        spec->identifier = strdup(specNode->token->valueStr);
        ParserNode *colTypeNode = specNode->children[0];
        if (colTypeNode->token->type == T_KW_STRING) {
            spec->type = COLTYPE_STRING;
            spec->size = (size_t)colTypeNode->children[0]->token->valueInt;
        } else if (colTypeNode->token->type == T_KW_INT) {
            spec->type = COLTYPE_INT;
        }
        if (specNode->childrenLength == 2) {
            if (specNode->children[1]->token->type == T_KW_INDEX) {
                spec->option = COLOPT_INDEX;
            }
        }
    }
    return list;
}

ComparisonGroup *parser_node_convert_comparison_group(ParserNode *node) {
    ComparisonGroup *group = new_comparision_group();
    for (size_t i = 0; i < node->childrenLength; i++) {
        ParserNode *compNode = node->children[i];
        Comparison *comp = new_comparison();
        append_comparison_group(group, comp);
        comp->identifier = strdup(compNode->children[0]->token->valueStr);
        if (compNode->children[1]->token->type == T_COMP_EQ) {
            comp->comp = COMP_EQ;
        } else if (compNode->children[1]->token->type == T_COMP_NEQ) {
            comp->comp = COMP_NEQ;
        }
        comp->value = parser_node_convert_value(compNode->children[2]);
    }
    return group;
}

AssignmentList *parser_node_convert_assignment_list(ParserNode *node) {
    AssignmentList *list = new_assignment_list();
    for (size_t i = 0; i < node->childrenLength; i++) {
        ParserNode *assignmentNode = node->children[i];
        Assignment *assignment = new_assignment();
        append_assignment_list(list, assignment);
        assignment->identifier = strdup(assignmentNode->token->valueStr);
        assignment->value = parser_node_convert_value(assignmentNode->children[0]);
    }
    return list;
}

Value *parser_node_convert_value(ParserNode *node) {
    Value *value = new_value();
    if (node->token->type == T_STRING) {
        value->type = VALUE_STRING;
        value->string = strdup(node->token->valueStr);
    } else if (node->token->type == T_NUMBER) {
        value->type = VALUE_NUMBER;
        value->number = node->token->valueInt;
    }
    return value;
}

#define NEXT_TOKEN() {\
        token = scanner_next_token(scanner, token);\
    }

#define EXPECT(token_type) {\
        NEXT_TOKEN();\
        if (token == NULL || token->type != (token_type)) {\
            parser_set_error(parser, "Unexpected input, expected "#token_type, token);\
        }\
    }
#define DECEND_NODE(type) { \
        ParserNode* newNode = new_parser_node(type, token); \
        append_parser_node(node, newNode);\
        node = newNode;\
        token = NULL;\
    }
#define ASCEND_NODE() {\
        node = node->parent;\
        if (node == NULL) { parser_set_error(parser, "Parent node was null", NULL); }\
    }

ParserNode *parser_parse(Parser *parser, Scanner *scanner) {
    if (scanner->state != SCANSTATE_START) {
        parser_set_error(parser, "Scanner not ready", NULL);
        return NULL;
    }

    if (parser->status != PARSESTATE_NONE) {
        parser_set_error(parser, "Parser was not reset before use", NULL);
        return NULL;
    }

    ScannerToken *token = NULL;
    ParserNode *root = new_parser_node(NODE_STATEMENT_LIST, NULL);
    ParserNode *node = root;

    parser->status = PARSESTATE_RUNNING;

    while (parser->status == PARSESTATE_RUNNING) {
        switch (node->type) {
            case NODE_STATEMENT_LIST: {
                NEXT_TOKEN();
                if (token == NULL) {
                    //End of input
                    parser->status = PARSESTATE_DONE;
                    break;
                }
                switch (token->type) {
                    case T_KW_SELECT: {
                        DECEND_NODE(NODE_SELECT_STMT);
                    }
                        break;
                    case T_KW_INSERT: {
                        DECEND_NODE(NODE_INSERT_STMT);
                        EXPECT(T_KW_INTO);
                    }
                        break;
                    case T_KW_UPDATE: {
                        DECEND_NODE(NODE_UPDATE_STMT);
                    }
                        break;
                    case T_KW_DELETE: {
                        DECEND_NODE(NODE_DELETE_STMT);
                        EXPECT(T_KW_FROM);
                    }
                        break;
                    case T_KW_CREATE: {
                        DECEND_NODE(NODE_CREATE_STMT);
                        EXPECT(T_KW_TABLE);
                    }
                        break;
                    case T_KW_DROP: {
                        DECEND_NODE(NODE_DROP_STMT);
                        EXPECT(T_KW_TABLE);
                    }
                        break;
                    case T_SEMICOLON: {
                        parser_set_error(parser, "Unexpected end of statement", token);
                    }
                        break;
                    default:
                        parser_set_error(parser, "Unexpected node type", token);
                        break;
                }
            }
                break;
            case NODE_SELECT_STMT:
                switch (node->phase) {
                    case 0: {//field list
                        ParserNode *fieldList = new_parser_node(NODE_FIELD_LIST, NULL);
                        append_parser_node(node, fieldList);
                        while (parser->status != PARSESTATE_ERROR && node->phase == 0) {
                            NEXT_TOKEN();
                            if (token == NULL) {
                                parser_set_error(parser, "Expected table name", token);
                                break;
                            }
                            switch (token->type) {
                                case T_IDENTIFIER:
                                    append_parser_node(fieldList, new_parser_node(NODE_IDENTIFIER, token));
                                    token = NULL;
                                    break;
                                case T_COMMA:
                                    break;
                                case T_KW_FROM:
                                    if (fieldList->childrenLength == 0) {
                                        parser_set_error(parser, "Field list empty", token);
                                    }
                                    node->phase++;
                                    break;
                                default:
                                    parser_set_error(parser, "Unexpected token, expecting , or identifier", token);
                                    break;
                            }
                        }
                    }
                        break;
                    case 1: {//table name
                        NEXT_TOKEN();
                        if (token != NULL && token->type == T_IDENTIFIER) {
                            append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                            token = NULL;
                            node->phase++;
                            NEXT_TOKEN();
                            //Expect where or end of command
                            if (token == NULL || (token->type != T_KW_WHERE && token->type != T_SEMICOLON)) {
                                parser_set_error(parser, "Expected WHERE or ;", token);
                            } else {
                                if (token->type == T_KW_WHERE) {
                                    ParserNode *group = new_parser_node(NODE_COMPARISON_GROUP, NULL);
                                    append_parser_node(node, group);
                                    node = group;
                                    break;
                                } else if (token->type == T_SEMICOLON) {
                                    node->phase = -1;
                                    //Finished reading select
                                    ASCEND_NODE();
                                    break;
                                }
                            }
                        } else {
                            parser_set_error(parser, "Expected table name", token);
                        }
                    }
                        break;
                    case 2: //after assignment list
                    EXPECT(T_SEMICOLON);
                        ASCEND_NODE();
                        break;
                    default:
                        parser_set_error(parser, "Unknown phase reading select", token);
                        break;
                }
                break;
            case NODE_INSERT_STMT: {
                switch (node->phase) {
                    case 0: { //table name
                        NEXT_TOKEN();
                        if (token == NULL || token->type != T_IDENTIFIER) {
                            parser_set_error(parser, "Expecting identifier", token);
                            break;
                        }
                        append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                        token = NULL;
                        node->phase++;
                        EXPECT(T_KW_SET);
                        ParserNode *assignments = new_parser_node(NODE_ASSIGNMENT_LIST, NULL);
                        append_parser_node(node, assignments);
                        node = assignments;

                    }
                        break;
                    case 1: { // end of assignment list
                        EXPECT(T_SEMICOLON);
                        ASCEND_NODE();
                    }
                        break;
                    default:
                        parser_set_error(parser, "Unknown phase reading insert", token);
                        break;
                }
            }
                break;
            case NODE_UPDATE_STMT: {
                switch (node->phase) {
                    case 0: {
                        NEXT_TOKEN();
                        if (token == NULL || token->type != T_IDENTIFIER) {
                            parser_set_error(parser, "Expected identifier", token);
                            break;
                        }
                        append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                        token = NULL;
                        node->phase++;
                        EXPECT(T_KW_SET);
                        ParserNode *assignments = new_parser_node(NODE_ASSIGNMENT_LIST, NULL);
                        append_parser_node(node, assignments);
                        node = assignments;
                    }
                        break;
                    case 1: { //end of command or where
                        NEXT_TOKEN();
                        if (token == NULL || (token->type != T_SEMICOLON && token->type != T_KW_WHERE)) {
                            parser_set_error(parser, "Expected ; or WHERE", token);
                            break;
                        }
                        if (token->type == T_SEMICOLON) {
                            ASCEND_NODE();
                        } else if (token->type == T_KW_WHERE) {
                            node->phase++;
                            ParserNode *comparisons = new_parser_node(NODE_COMPARISON_GROUP, NULL);
                            append_parser_node(node, comparisons);
                            node = comparisons;
                        }
                    }
                        break;
                    case 2: {
                        EXPECT(T_SEMICOLON);
                        ASCEND_NODE();
                    }
                        break;
                    default:
                        parser_set_error(parser, "Unknown phase reading update", token);
                        break;
                }
            }
                break;
            case NODE_DELETE_STMT: {
                switch (node->phase) {
                    case 0: {
                        NEXT_TOKEN();
                        if (token == NULL || token->type != T_IDENTIFIER) {
                            parser_set_error(parser, "Expected identifier", token);
                            break;
                        }
                        append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                        token = NULL;
                        NEXT_TOKEN();
                        //Where or end of command
                        if (token == NULL || (token->type != T_KW_WHERE && token->type != T_SEMICOLON)) {
                            parser_set_error(parser, "Expected ; or WHERE", token);
                            break;
                        }
                        if (token->type == T_KW_WHERE) {
                            node->phase++;
                            ParserNode *comparison = new_parser_node(NODE_COMPARISON_GROUP, NULL);
                            append_parser_node(node, comparison);
                            node = comparison;
                        } else if (token->type == T_SEMICOLON) {
                            ASCEND_NODE();
                            break;
                        }
                    }
                        break;
                    case 1: {
                        EXPECT(T_SEMICOLON);
                        ASCEND_NODE();
                    }
                        break;
                    default:
                        parser_set_error(parser, "Unknown phase reading delete", token);
                        break;
                }
            }
                break;
            case NODE_CREATE_STMT: {
                switch (node->phase) {
                    case 0: {
                        NEXT_TOKEN();
                        if (token == NULL || token->type != T_IDENTIFIER) {
                            parser_set_error(parser, "Expected identifier", token);
                            break;
                        }
                        append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                        token = NULL;
                        EXPECT(T_PAREN_OPEN);
                        node->phase++;
                        ParserNode *columnSpecs = new_parser_node(NODE_COLUMN_SPEC_LIST, NULL);
                        append_parser_node(node, columnSpecs);
                        node = columnSpecs;
                    }
                        break;
                    case 1: {
                        EXPECT(T_PAREN_CLOSE);
                        EXPECT(T_SEMICOLON);
                        ASCEND_NODE();
                    }
                        break;
                    default:
                        parser_set_error(parser, "Unknown phase reading create", token);
                        break;
                }
            }
                break;
            case NODE_DROP_STMT: {
                NEXT_TOKEN();
                if (token == NULL || token->type != T_IDENTIFIER) {
                    parser_set_error(parser, "Expected identifier", token);
                    break;
                }
                append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                token = NULL;
                EXPECT(T_SEMICOLON);
                ASCEND_NODE();
            }
                break;
            case NODE_COLUMN_SPEC_LIST: {
                while (parser->status != PARSESTATE_ERROR) {
                    NEXT_TOKEN();
                    if (token == NULL || token->type != T_IDENTIFIER) {
                        parser_set_error(parser, "Expected identifier", token);
                        break;
                    }
                    ParserNode *columnSpec = new_parser_node(NODE_COLUMN_SPEC, token);
                    token = NULL;
                    append_parser_node(node, columnSpec);
                    NEXT_TOKEN();
                    if (token == NULL || (token->type != T_KW_STRING && token->type != T_KW_INT)) {
                        parser_set_error(parser, "Expected one of STRING, INT, INTEGER", token);
                        break;
                    }
                    if (token->type == T_KW_STRING) {
                        ParserNode *columnType = new_parser_node(NODE_COLUMN_TYPE, token);
                        token = NULL;
                        append_parser_node(columnSpec, columnType);
                        EXPECT(T_PAREN_OPEN);
                        NEXT_TOKEN();
                        if (token == NULL || token->type != T_NUMBER) {
                            parser_set_error(parser, "Expected number", token);
                            break;
                        }
                        if (token->valueInt < 1) {
                            parser_set_error(parser, "Expected positive number", token);
                            break;
                        }
                        append_parser_node(columnType, new_parser_node(NODE_COLUMN_TYPE_SPECIFIER, token));
                        token = NULL;
                        EXPECT(T_PAREN_CLOSE);
                    } else if (token->type == T_KW_INT) {
                        append_parser_node(columnSpec, new_parser_node(NODE_COLUMN_TYPE, token));
                        token = NULL;
                    }
                    NEXT_TOKEN();
                    if (token == NULL) {
                        ASCEND_NODE();
                        break;
                    }
                    //Look for options
                    if (token->type == T_KW_INDEX) {
                        append_parser_node(node, new_parser_node(NODE_COLUMN_OPTION, token));
                        token = NULL;
                        NEXT_TOKEN();
                        if (token == NULL) {
                            ASCEND_NODE();
                            break;
                        }
                    }
                    //Comma or ascend
                    if (token->type != T_COMMA) {
                        scanner_push_buffer(scanner, token);
                        token = NULL;
                        ASCEND_NODE();
                        break;
                    }
                }
            }
                break;
            case NODE_ASSIGNMENT_LIST: {
                //Read assignments
                while (parser->status != PARSESTATE_ERROR) {
                    NEXT_TOKEN();
                    if (token == NULL || token->type != T_IDENTIFIER) {
                        parser_set_error(parser, "Expected identifier", token);
                        break;
                    }
                    ParserNode *assignment = new_parser_node(NODE_ASSIGNMENT, token);
                    token = NULL;
                    append_parser_node(node, assignment);
                    EXPECT(T_COMP_EQ);
                    NEXT_TOKEN();
                    if (token == NULL || (token->type != T_STRING && token->type != T_NUMBER)) {
                        parser_set_error(parser, "Expected value", token);
                        break;
                    }
                    append_parser_node(assignment, new_parser_node(NODE_VALUE, token));
                    token = NULL;
                    //Check for comma
                    NEXT_TOKEN();
                    if (token == NULL) {
                        ASCEND_NODE();
                        break;
                    }
                    if (token->type != T_COMMA) {
                        scanner_push_buffer(scanner, token);
                        token = NULL;
                        ASCEND_NODE();
                        break;
                    }
                }
            }
                break;
            case NODE_COMPARISON_GROUP: {
                NEXT_TOKEN();
                if (token == NULL) {
                    parser_set_error(parser, "Expected identifier", NULL);
                    break;
                }
                switch (token->type) {
                    case T_IDENTIFIER:
                        scanner_push_buffer(scanner, token);
                        token = NULL;
                        DECEND_NODE(NODE_COMPARISON);
                        break;
                    case T_COMP_AND:
                        break;
                    default:
                        scanner_push_buffer(scanner, token);
                        token = NULL;
                        ASCEND_NODE();
                        break;
                }
            }
                break;
            case NODE_COMPARISON:
                while (parser->status != PARSESTATE_ERROR) {
                    NEXT_TOKEN();
                    if (token == NULL) {
                        parser_set_error(parser, "Unexpected end of input", NULL);
                        break;
                    }
                    switch (node->phase) {
                        case 0:
                            if (token->type == T_IDENTIFIER) {
                                append_parser_node(node, new_parser_node(NODE_IDENTIFIER, token));
                                token = NULL;
                                node->phase++;
                            } else {
                                parser_set_error(parser, "Expected identifier", token);
                            }
                            break;
                        case 1:
                            if (token->type == T_COMP_EQ || token->type == T_COMP_NEQ) {
                                append_parser_node(node, new_parser_node(NODE_COMPARISON, token));
                                token = NULL;
                                node->phase++;
                            } else {
                                parser_set_error(parser, "Expected comparator", token);
                            }
                            break;
                        case 2:
                            if (token->type == T_STRING || token->type == T_NUMBER) {
                                append_parser_node(node, new_parser_node(NODE_VALUE, token));
                                token = NULL;
                                node->phase = -1;
                            } else {
                                parser_set_error(parser, "Expected value", token);
                            }
                            break;
                        default:
                            parser_set_error(parser, "Unknown phase reading comparison", token);
                            break;
                    }
                    if (node->phase == -1) {
                        ASCEND_NODE();
                        break;
                    }
                }
                break;
            default:
                parser_set_error(parser, "Parser in unknown state", token);
                break;
        }
    }
    if (token != NULL) {
        free_scanner_token(token);
    }

    if (scanner->state == SCANSTATE_ERROR) {
        parser_set_error(parser, scanner->errMsg, NULL);
    }

    if (parser->status != PARSESTATE_ERROR) {
        return root;
    }

    free_parser_node(root);
    return NULL;
}

#undef NEXT_TOKEN
#undef EXPECT
#undef DECEND_NODE
#undef ASCEND_NODE

void parser_set_error(Parser *parser, char *err, ScannerToken *token) {
    parser->status = PARSESTATE_ERROR;
    if (token != NULL) {
        char *errMsg = calloc(128, sizeof(char));
        snprintf(errMsg, 128, "[%ld:%ld] %s", token->lineNo, token->linePos, err);
        parser->errMsg = strdup(errMsg);
        free(errMsg);
    } else {
        parser->errMsg = strdup(err);
    }
}

void parser_print_node_tree(ParserNode *node, size_t indent) {
    char *indentStr = calloc(indent + 1, sizeof(char));
    memset(indentStr, ' ', sizeof(char) * indent);

    //Type
    printf("%s%s", indentStr, parser_node_type_to_str(node->type));

    free(indentStr);

    //Token
    if (node->token != NULL) {
        printf(": %s", scanner_token_type_to_str(node->token->type));
        switch (node->token->type) {
            case T_IDENTIFIER:
            case T_STRING:
                printf("<%s>", node->token->valueStr);
                break;
            case T_NUMBER:
                printf("<%ld>", node->token->valueInt);
                break;
            default:
                break;
        }
    }
    printf("\n");
    for (size_t i = 0; i < node->childrenLength; i++) {
        parser_print_node_tree(node->children[i], indent + 2);
    }
}