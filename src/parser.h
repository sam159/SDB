//
// Created by Sam on 07/06/2018.
//

#ifndef SDB_PARSER_H
#define SDB_PARSER_H

#include <stddef.h>
#include <stdbool.h>
#include "scanner.h"
#include "sql.h"

#define PARSE_STATUS_LIST \
    X(PARSESTATE_NONE) \
    X(PARSESTATE_RUNNING) \
    X(PARSESTATE_DONE) \
    X(PARSESTATE_ERROR) \

#define X(t) t,
enum ParseStatus_t {
    PARSE_STATUS_LIST
};
#undef X
typedef enum ParseStatus_t ParseStatus;

char *parser_status_to_str(ParseStatus status);

#define PARSE_NODE_TYPE_LIST \
    X(NODE_NONE) \
    X(NODE_STATEMENT_LIST) \
    X(NODE_SELECT_STMT) \
    X(NODE_INSERT_STMT) \
    X(NODE_UPDATE_STMT) \
    X(NODE_DELETE_STMT) \
    X(NODE_CREATE_STMT) \
    X(NODE_DROP_STMT) \
    X(NODE_COLUMN_SPEC_LIST) \
    X(NODE_COLUMN_SPEC) \
    X(NODE_COLUMN_TYPE) \
    X(NODE_COLUMN_TYPE_SPECIFIER) \
    X(NODE_COLUMN_OPTION) \
    X(NODE_FIELD_LIST) \
    X(NODE_ASSIGNMENT_LIST) \
    X(NODE_ASSIGNMENT) \
    X(NODE_COMPARISON_GROUP) \
    X(NODE_COMPARISON) \
    X(NODE_COMPARATOR) \
    X(NODE_IDENTIFIER) \
    X(NODE_VALUE)

#define X(t) t,
enum ParserNodeType_t {
    PARSE_NODE_TYPE_LIST
};
typedef enum ParserNodeType_t ParserNodeType;
#undef X

char* parser_node_type_to_str(ParserNodeType nodeType);

struct ParserNode_t {
    struct ParserNode_t *parent;
    ParserNodeType type;
    ssize_t phase;
    ScannerToken *token;
    struct ParserNode_t **children;
    size_t childrenLength;
};
typedef struct ParserNode_t ParserNode;

ParserNode *new_parser_node(ParserNodeType type, ScannerToken *token);

void free_parser_node(ParserNode *node);

void append_parser_node(ParserNode *node, ParserNode *child);

struct Parser_t {
    ParseStatus status;
    char *errMsg;
};
typedef struct Parser_t Parser;

Parser *new_parser();

void free_parser(Parser *parser);

ParserNode *parser_parse(Parser *parser, Scanner *scanner);

void parser_set_error(Parser *parser, char *err, ScannerToken *token);

void parser_print_node_tree(ParserNode *node, size_t indent);

#endif //SDB_PARSER_H
