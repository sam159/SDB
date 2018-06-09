//
// Created by Sam on 05/06/2018.
//

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include "scanner.h"

char* scanner_token_type_to_str(ScannerTokenType tokenType) {
#define X(t) case t: return #t;
    switch (tokenType) {
        SCANNER_TOKEN_TYPE_LIST
        default:
            return "UNKNOWN";
    }
#undef X
}

ScannerToken *new_scanner_token() {
    ScannerToken *result = malloc(sizeof(ScannerToken));
    clear_scanner_token(result);
    return result;
}

void clear_scanner_token(ScannerToken *result) {
    result->type = T_NONE;
    result->valueInt = 0;
    if (result->valueStr != NULL) {
        free(result->valueStr);
    }
    result->valueStr = NULL;
    result->lineNo = 0;
    result->linePos = 0;
}

void free_scanner_token(ScannerToken *result) {
    if (result->valueStr != NULL) {
        free(result);
    }
    free(result);
}

char * scanner_state_to_str(ScannerState state) {
#define X(t) case t: return #t;
    switch (state) {
        SCANNER_STATE_LIST
        default:
            return "UNKNOWN";
    }
#undef X
}

Scanner *new_scanner(char *input) {
    Scanner *scanner = malloc(sizeof(Scanner));
    scanner->errMsg = NULL;
    scanner->state = SCANSTATE_START;
    scanner->buffer = NULL;
    scanner->bufferIndex = 0;
    scanner->bufferLength = 0;
    scanner->lineNo = 1;
    scanner->linePos = 1;
    scanner->input = input;
    scanner->pos = 0;
    return scanner;
}

void free_scanner(Scanner *scanner) {
    reset_scanner(scanner, NULL);
    free(scanner);
}

void reset_scanner(Scanner *scanner, char *input) {
    scanner->pos = 0;
    scanner->lineNo = 1;
    scanner->linePos = 1;
    scanner->state = SCANSTATE_START;
    if (scanner->input != NULL) {
        free(scanner->input);
    }
    scanner->input = input;
    if (scanner->errMsg != NULL) {
        free(scanner->errMsg);
    }
    scanner->errMsg = NULL;
    if (scanner->bufferIndex > 0) {
        for (size_t i = 0; i < scanner->bufferIndex; i++) {
            free_scanner_token(scanner->buffer[i]);
        }
        scanner->bufferIndex = 0;
    }
    if (scanner->bufferLength > 0) {
        free(scanner->buffer);
        scanner->buffer = NULL;
    }
}

void scanner_push_buffer(Scanner *scanner, ScannerToken *token) {
    if (scanner->bufferLength == 0) {
        scanner->bufferLength = 1;
        scanner->buffer = malloc(scanner->bufferLength * sizeof(ScannerToken*));
    } else if (scanner->bufferIndex + 1 == scanner->bufferLength) {
        scanner->bufferLength *= 2;
        scanner->buffer = realloc(scanner->buffer, scanner->bufferLength * sizeof(ScannerToken*));
    }
    scanner->buffer[scanner->bufferIndex++] = token;
}

ScannerToken *scanner_next_token(Scanner *scanner, ScannerToken *token) {
    //Return if already done
    if (scanner->state == SCANSTATE_DONE || scanner->state == SCANSTATE_ERROR) {
        if (token != NULL) free_scanner_token(token);
        return NULL;
    }

    if (scanner->bufferIndex > 0) {
        if (token != NULL) {
            free_scanner_token(token);
        }
        scanner->bufferIndex--;
        token = scanner->buffer[scanner->bufferIndex];
        scanner->buffer[scanner->bufferIndex] = NULL;
        return token;
    }

    //Setup result
    if (token == NULL) {
        token = new_scanner_token();
    } else {
        clear_scanner_token(token);
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
        free_scanner_token(token);
        return NULL;
    }

    //Record position of token in the input
    token->lineNo = scanner->lineNo;
    token->linePos = scanner->linePos;

    //Keywords
    if (scanner_match(scanner, "select", true)) token->type = T_KW_SELECT;
    else if (scanner_match(scanner, "from", true)) token->type = T_KW_FROM;
    else if (scanner_match(scanner, "insert", true)) token->type = T_KW_INSERT;
    else if (scanner_match(scanner, "into", true)) token->type = T_KW_INTO;
    else if (scanner_match(scanner, "set", true)) token->type = T_KW_SET;
    else if (scanner_match(scanner, "update", true)) token->type = T_KW_UPDATE;
    else if (scanner_match(scanner, "where", true)) token->type = T_KW_WHERE;
    else if (scanner_match(scanner, "delete", true)) token->type = T_KW_DELETE;
    else if (scanner_match(scanner, "create", true)) token->type = T_KW_CREATE;
    else if (scanner_match(scanner, "table", true)) token->type = T_KW_TABLE;
    else if (scanner_match(scanner, "drop", true)) token->type = T_KW_DROP;
    else if (scanner_match(scanner, "string", true)) token->type = T_KW_STRING;
    else if (scanner_match(scanner, "int", true)) token->type = T_KW_INT;
    else if (scanner_match(scanner, "integer", true)) token->type = T_KW_INT;
    else if (scanner_match(scanner, "index", true)) token->type = T_KW_INDEX;
    if (token->type != T_NONE) {
        return token;
    }

    //Comparators
    if (scanner_match(scanner, "=", false)) token->type = T_COMP_EQ;
    else if (scanner_match(scanner, "<>", false)) token->type = T_COMP_NEQ;
    else if (scanner_match(scanner, "and", true)) token->type = T_COMP_AND;
    if (token->type != T_NONE) {
        return token;
    }

    //Punctuation
    char nextChar = scanner_peek_char(scanner);
    switch (nextChar) {
        case ',':
            token->type = T_COMMA;
            break;
        case ';':
            token->type = T_SEMICOLON;
            break;
        case '(':
            token->type = T_PAREN_OPEN;
            break;
        case ')':
            token->type = T_PAREN_CLOSE;
            break;
        default:
            break;
    }
    if (token->type != T_NONE) {
        scanner_next_char(scanner);
        return token;
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
            free_scanner_token(token);
            return NULL;
        }
        token->type = T_NUMBER;
        //convert number
        token->valueInt = (uint64_t) strtol(intInput, NULL, 10);
        return token;
    }

    //Strings
    if (scanner_peek_char(scanner) == '"' || scanner_peek_char(scanner) == '\'') {
        if (scanner_read_string(scanner, token, scanner_next_char(scanner)) == false) {
            free_scanner_token(token);
            return NULL;
        }
        return token;
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
        token->type = T_IDENTIFIER;
        token->valueStr = strdup(ident);
        free(ident);
        return token;
    }

    //Nothing matched
    scanner_set_error(scanner, "Unexpected input");
    free_scanner_token(token);
    return NULL;
}

bool scanner_read_string(Scanner *scanner, ScannerToken *result, char quoteType) {
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
            result->type = T_STRING;
            result->valueStr = strdup(str);
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

    snprintf(errMsg, 128, "[%d:%d]: %s", scanner->lineNo, scanner->linePos, errText);

    if (scanner->errMsg != NULL) {
        free(scanner->errMsg);
    }
    scanner->state = SCANSTATE_ERROR;
    scanner->errMsg = errMsg;
}