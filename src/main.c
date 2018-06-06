#include <stdio.h>
#include <stdbool.h>
#include <mem.h>

#include "InputBuffer.h"
#include "scanner.h"

void prompt() {
    printf("SDB> ");
}

void read_input(InputBuffer *buffer) {
    ssize_t read = getline(&buffer->buffer, &buffer->bufferLength, stdin);

    if (read <= 0) {
        printf("Failed to read input\n");
        exit(EXIT_FAILURE);
    }

    //Remove newline
    buffer->inputLength = read - 1;
    buffer->buffer[read - 1] = 0;
}

void parse_input(char *input) {
    Scanner *scanner = new_scanner(strdup(input));
    Scanner_Result *result = NULL;

    while ((result = scanner_next_token(scanner, result)) != NULL) {
        if (result->token == T_STRING) {
            printf("Found String: '%s'\n", result->valueStr);
        } else if (result->token == T_IDENTIFIER) {
            printf("Found Identifier: %s\n", result->valueStr);
        } else if (result->token == T_NUMBER) {
            printf("Found Number %lld\n", result->valueInt);
        } else {
            printf("Found Token: %d\n", result->token);
        }
    }

    if (scanner->state == SCANSTATE_ERROR) {
        if (scanner->errMsg != NULL) {
            fprintf(stderr, "%s\n", scanner->errMsg);
        } else {
            fprintf(stderr, "Parse Error!\n");
        }
    }

    free_scanner(scanner);
}

int main() {
#if defined(_WIN32) || defined(WIN32)
    //Fix issues with debugging on windows
    setbuf(stdout, 0);
    setbuf(stderr, 0);
#endif
    InputBuffer *buffer = input_buffer_new();

    while (true) {
        prompt();
        read_input(buffer);

        if (strcmpi(buffer->buffer, ".exit") == 0) {
            break;
        } else {
            parse_input(buffer->buffer);
        }
    }

    input_buffer_free(buffer);
    return EXIT_SUCCESS;
}