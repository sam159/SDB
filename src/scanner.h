//
// Created by Sam on 05/06/2018.
//

#ifndef SDB_SCANNER_H
#define SDB_SCANNER_H

#include <stdbool.h>
#include <stdint.h>

enum Scanner_Token_t {
    T_NONE,

    //Values
    T_STRING,
    T_NUMBER,
    T_IDENTIFIER,

    //Punctuation
    T_COMMA,
    T_SEMICOLON,

    //Comparators
    T_COMP_EQ,
    T_COMP_NEQ,
    T_COMP_AND,

    //Keywords
    K_SELECT,
    K_FROM,
    K_INSERT,
    K_INTO,
    K_SET,
    K_UPDATE,
    K_WHERE,
    K_DELETE,
    K_CREATE,
    K_TABLE,
    K_DROP,
    K_STRING,
    K_INT,
    K_INDEX
};
typedef enum Scanner_Token_t Scanner_Token;

struct Scanner_Result_t {
    Scanner_Token token;
    char *value_str;
    uint64_t value_int;
};
typedef struct Scanner_Result_t Scanner_Result;

Scanner_Result *new_scanner_result();

void clear_scanner_result(Scanner_Result *result);

void free_scanner_result(Scanner_Result *result);

enum Scanner_State_t {
    SCANSTATE_START,
    SCANSTATE_ERROR,
    SCANSTATE_DONE
};
typedef enum Scanner_State_t Scanner_State;

struct Scanner_t {
    char *input;
    char *errMsg;
    size_t lineNo;
    size_t linePos;
    size_t pos;
    Scanner_State state;
};
typedef struct Scanner_t Scanner;

Scanner *new_scanner(char *input);

void free_scanner(Scanner *scanner);

void reuse_scanner(Scanner *scanner, char *input);

Scanner_Result *scanner_next_token(Scanner *scanner, Scanner_Result *result);

bool scanner_read_string(Scanner *scanner, Scanner_Result *result, char quoteType);

char scanner_next_char(Scanner *scanner);

char scanner_peek_char(Scanner *scanner);

bool scanner_match(Scanner *scanner, const char *text, bool keyword);

void scanner_set_error(Scanner *scanner, const char *errText);

#endif //SDB_SCANNER_H
