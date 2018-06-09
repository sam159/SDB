//
// Created by Sam on 05/06/2018.
//

#ifndef SDB_SCANNER_H
#define SDB_SCANNER_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define SCANNER_TOKEN_TYPE_LIST \
    X(T_NONE) \
    X(T_STRING) \
    X(T_NUMBER) \
    X(T_IDENTIFIER) \
    X(T_COMMA) \
    X(T_SEMICOLON) \
    X(T_PAREN_OPEN) \
    X(T_PAREN_CLOSE) \
    X(T_COMP_EQ) \
    X(T_COMP_NEQ) \
    X(T_COMP_AND) \
    X(T_KW_SELECT) \
    X(T_KW_FROM) \
    X(T_KW_INSERT) \
    X(T_KW_INTO) \
    X(T_KW_SET) \
    X(T_KW_UPDATE) \
    X(T_KW_WHERE) \
    X(T_KW_DELETE) \
    X(T_KW_CREATE) \
    X(T_KW_TABLE) \
    X(T_KW_DROP) \
    X(T_KW_STRING) \
    X(T_KW_INT) \
    X(T_KW_INDEX)

#define X(t) t,
enum ScannerTokenType_t {
    SCANNER_TOKEN_TYPE_LIST
};
#undef X
typedef enum ScannerTokenType_t ScannerTokenType;

char* scanner_token_type_to_str(ScannerTokenType tokenType);

struct ScannerToken_t {
    ScannerTokenType type;
    char *valueStr;
    uint64_t valueInt;
    size_t lineNo;
    size_t linePos;
};
typedef struct ScannerToken_t ScannerToken;

ScannerToken *new_scanner_token();

void clear_scanner_token(ScannerToken *result);

void free_scanner_token(ScannerToken *result);

#define SCANNER_STATE_LIST \
    X(SCANSTATE_START) \
    X(SCANSTATE_ERROR) \
    X(SCANSTATE_DONE)

#define X(t) t,
enum ScannerState_t {
    SCANNER_STATE_LIST
};
#undef X
typedef enum ScannerState_t ScannerState;

char * scanner_state_to_str(ScannerState state);

struct Scanner_t {
    char *input;
    char *errMsg;
    size_t lineNo;
    size_t linePos;
    size_t pos;
    ScannerState state;
    ScannerToken **buffer;
    size_t bufferLength;
    size_t bufferIndex;
};
typedef struct Scanner_t Scanner;

Scanner *new_scanner(char *input);

void free_scanner(Scanner *scanner);

void reset_scanner(Scanner *scanner, char *input);

void scanner_push_buffer(Scanner *scanner, ScannerToken *token);

ScannerToken *scanner_next_token(Scanner *scanner, ScannerToken *token);

bool scanner_read_string(Scanner *scanner, ScannerToken *result, char quoteType);

char scanner_next_char(Scanner *scanner);

char scanner_peek_char(Scanner *scanner);

bool scanner_match(Scanner *scanner, const char *text, bool keyword);

void scanner_set_error(Scanner *scanner, const char *errText);

#endif //SDB_SCANNER_H
