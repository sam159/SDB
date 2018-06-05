//
// Created by Sam on 05/06/2018.
//

#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include "scanner.h"

Scanner_Result *new_scanner_result() {
    Scanner_Result *result = malloc(sizeof(Scanner_Result));
    result->token = T_NONE;
    result->value_int = 0;
    result->value_str = NULL;
    return result;
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
    scanner->state = SCANSTATE_START;
    return scanner;
}

void free_scanner(Scanner *scanner) {
    if (scanner->input != NULL) {
        free(scanner->input);
    }
    free(scanner);
}

void reuse_scanner(Scanner *scanner, char *input) {
    scanner->state = SCANSTATE_START;
    if (scanner->input != NULL) {
        free(scanner->input);
    }
    scanner->input = input;
    scanner->pos = 0;
}

Scanner_Result *scanner_next_token(Scanner *scanner, Scanner_Result *result) {
    if (scanner->state == SCANSTATE_DONE) {
        return NULL;
    }
    //Consume white space
    while (scanner_peek_char(scanner) != NULL && isblank(scanner_peek_char(scanner))) {
        scanner->pos++;
    }
    //Check for end of string
    if (scanner_peek_char(scanner) == NULL) {
        scanner->state = SCANSTATE_DONE;
        return NULL;
    }
}

char scanner_next_char(Scanner *scanner) {
    char next = scanner->input[scanner->pos];
    if (next != NULL) {
        scanner->pos++;
    }
    return next;
}

char scanner_peek_char(Scanner *scanner) {
    return scanner->input[scanner->pos];
}

bool scanner_match(Scanner *scanner, const char *text) {
    char a = NULL;
    char b = NULL;
    size_t ai = scanner->pos;
    size_t bi = 0;

    do {
        b = text[bi++];
        //Match if we reach the end of text
        if (b == NULL) {
            scanner->pos = ai; //consume text from scanner input
            return true;
        }
        a = scanner->input[ai++];
    } while (a != NULL && tolower(a) == tolower(b));

    return false;
}