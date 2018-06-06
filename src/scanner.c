//
// Created by Sam on 05/06/2018.
//

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include "scanner.h"

Scanner_Result *new_scanner_result() {
    Scanner_Result *result = malloc(sizeof(Scanner_Result));
    clear_scanner_result(result);
    return result;
}

void clear_scanner_result(Scanner_Result *result) {
    result->token = T_NONE;
    result->value_int = 0;
    if (result->value_str != NULL) {
        free(result->value_str);
    }
    result->value_str = NULL;
}

void free_scanner_result(Scanner_Result *result) {
    if (result->value_str != NULL) {
        free(result);
    }
    free(result);
}

Scanner *new_scanner(char *input) {
    Scanner *scanner = malloc(sizeof(Scanner));
    scanner->input = input;
    scanner->pos = 0;
    scanner->lineNo = 1;
    scanner->linePos = 1;
    scanner->state = SCANSTATE_START;
    scanner->errMsg = NULL;
    return scanner;
}

void free_scanner(Scanner *scanner) {
    if (scanner->input != NULL) {
        free(scanner->input);
    }
    free(scanner);
}

void reuse_scanner(Scanner *scanner, char *input) {
    if (scanner->input != NULL) {
        free(scanner->input);
    }
    scanner->input = input;
    if (scanner->errMsg != NULL) {
        free(scanner->input);
        scanner->errMsg = NULL;
    }
    scanner->pos = 0;
    scanner->lineNo = 1;
    scanner->linePos = 1;
    scanner->state = SCANSTATE_START;
    scanner->errMsg = NULL;
}

Scanner_Result *scanner_next_token(Scanner *scanner, Scanner_Result *result) {
    if (scanner->state == SCANSTATE_DONE || scanner->state == SCANSTATE_ERROR) {
        if (result != NULL) free_scanner_result(result);
        return NULL;
    }
    if (result == NULL) {
        result = new_scanner_result();
    } else {
        clear_scanner_result(result);
    }
    //Consume white space
    while (scanner_peek_char(scanner) != 0 && isspace(scanner_peek_char(scanner))) {
        if (scanner_next_char(scanner) == '\n') {
            scanner->lineNo++;
            scanner->linePos = 1;
        }
    }
    //Check for end of string
    if (scanner_peek_char(scanner) == 0) {
        scanner->state = SCANSTATE_DONE;
        free_scanner_result(result);
        return NULL;
    }

    //Keywords
    if (scanner_match(scanner, "select", true)) result->token = K_SELECT;
    else if (scanner_match(scanner, "from", true)) result->token = K_FROM;
    else if (scanner_match(scanner, "insert", true)) result->token = K_INSERT;
    else if (scanner_match(scanner, "into", true)) result->token = K_INTO;
    else if (scanner_match(scanner, "set", true)) result->token = K_SET;
    else if (scanner_match(scanner, "update", true)) result->token = K_UPDATE;
    else if (scanner_match(scanner, "where", true)) result->token = K_WHERE;
    else if (scanner_match(scanner, "delete", true)) result->token = K_DELETE;
    else if (scanner_match(scanner, "create", true)) result->token = K_CREATE;
    else if (scanner_match(scanner, "table", true)) result->token = K_TABLE;
    else if (scanner_match(scanner, "drop", true)) result->token = K_DROP;
    else if (scanner_match(scanner, "string", true)) result->token = K_STRING;
    else if (scanner_match(scanner, "int", true)) result->token = K_INT;
    else if (scanner_match(scanner, "index", true)) result->token = K_INDEX;
    if (result->token != T_NONE) {
        return result;
    }

    //Comparators
    if (scanner_match(scanner, "=", false)) result->token = T_COMP_EQ;
    if (scanner_match(scanner, "<>", false)) result->token = T_COMP_NEQ;
    if (scanner_match(scanner, "and", true)) result->token = T_COMP_AND;
    if (result->token != T_NONE) {
        return result;
    }

    //Punctuation
    if (scanner_peek_char(scanner) == ',') {
        scanner_next_char(scanner);
        result->token = T_COMMA;
    }
    if (scanner_peek_char(scanner) == ';') {
        scanner_next_char(scanner);
        result->token = T_SEMICOLON;
    }
    if (result->token != T_NONE) {
        return result;
    }

    //Numbers
    if (isdigit(scanner_peek_char(scanner))) {
        char intInput[32] = {0};
        size_t intIndex = 0;
        while (scanner_peek_char(scanner) != 0
               && isdigit(scanner_peek_char(scanner))
               && intIndex + 1 < 32) {
            intInput[intIndex++] = scanner_next_char(scanner);
        }
        if (isdigit(scanner_peek_char(scanner))) {
            //Exceed length of int array!
            scanner_set_error(scanner, "Number exceeded allowed length");
            free_scanner_result(result);
            return NULL;
        }
        result->token = T_NUMBER;
        //convert number
        result->value_int = (uint64_t) strtol(intInput, NULL, 10);
        return result;
    }

    //Strings
    if (scanner_peek_char(scanner) == '"' || scanner_peek_char(scanner) == '\'') {
        if (scanner_read_string(scanner, result, scanner_next_char(scanner)) == false) {
            free_scanner_result(result);
            return NULL;
        }
        return result;
    }

    //Identifiers
    if (isalpha(scanner_peek_char(scanner)) || scanner_peek_char(scanner) == '_') {
        size_t ident_size = 8;
        char *ident = calloc(ident_size, sizeof(char));
        size_t ident_i = 0;
        do {
            ident[ident_i++] = scanner_next_char(scanner);
            ident[ident_i] = 0;

            if (ident_i + 1 == ident_size) {
                ident_size = ident_size * 2;
                ident = realloc(ident, sizeof(char) * ident_size);
            }
        } while ((isalnum(scanner_peek_char(scanner)) || scanner_peek_char(scanner) == '_'));
        result->token = T_IDENTIFIER;
        result->value_str = strdup(ident);
        free(ident);
        return result;
    }

    //Nothing matched
    scanner_set_error(scanner, "Unknown text");
    free_scanner_result(result);
    return NULL;
}

bool scanner_read_string(Scanner *scanner, Scanner_Result *result, char quoteType) {
    char next = 0;

    size_t length = 8;
    size_t i = 0;
    char *str = calloc(length, sizeof(char));

    while ((next = scanner_next_char(scanner)) != 0) {
        if (next == '\\') {
            //Capture escapes
            char escaped = scanner_next_char(scanner);
            if (escaped == 0) {
                //end of input!
                scanner_set_error(scanner, "Unterminated String");
                return false;
            }
            switch (escaped) {
                case 'n':
                    next = '\n';
                    break;
                case 'r':
                    next = '\r';
                    break;
                case 't':
                    next = '\t';
                    break;
                case '\\':
                    next = '\\';
                    break;
                case '\'':
                    next = '\'';
                    break;
                case '"':
                    next = '"';
                    break;
                default:
                    next = 0;
                    break;
            }
            if (next == 0) {
                //ignore unknown escape sequence
                continue;
            }
        } else if (next == quoteType) {
            //end of string
            result->token = T_STRING;
            result->value_str = strdup(str);
            free(str);
            return true;
        }
        if (next == '\n') {
            scanner->lineNo++;
        }

        str[i++] = next;
        str[i] = 0;
        if (i + 1 == length) {
            //expand string
            length = length * 2;
            str = realloc(str, sizeof(char) * length);
        }
    }
    //reached end of input
    scanner_set_error(scanner, "Unterminated String");
    return false;
}

char scanner_next_char(Scanner *scanner) {
    char next = scanner->input[scanner->pos];
    if (next != 0) {
        scanner->pos++;
        scanner->linePos++;
    }
    return next;
}

char scanner_peek_char(Scanner *scanner) {
    return scanner->input[scanner->pos];
}

bool scanner_match(Scanner *scanner, const char *text, bool keyword) {
    char a = 0;
    char b = 0;
    size_t ai = scanner->pos;
    size_t bi = 0;

    do {
        b = text[bi++];
        //Match if we reach the end of text
        if (b == 0) {
            //Check that the next char is not an alphanumeric char (ie do not match hello against helloworld)
            if (keyword && isalnum(scanner->input[ai])) {
                break;
            }
            scanner->pos = ai; //consume text from scanner input
            scanner->linePos += strlen(text);
            return true;
        }
        a = scanner->input[ai++];
    } while (a != 0 && tolower(a) == tolower(b));

    return false;
}

void scanner_set_error(Scanner *scanner, const char *errText) {
    scanner->state = SCANSTATE_ERROR;

    //Create error msg
    char *errMsg = malloc(sizeof(char) * 128);

    snprintf(errMsg, 128, "Error [%d:%d]: %s", scanner->lineNo, scanner->linePos, errText);

    if (scanner->errMsg != NULL) {
        free(scanner->errMsg);
    }
    scanner->state = SCANSTATE_ERROR;
    scanner->errMsg = errMsg;
    printf(scanner->errMsg);
}