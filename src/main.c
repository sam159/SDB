#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <assert.h>
#include <time.h>

#include "InputBuffer.h"
#include "scanner.h"
#include "parser.h"
#include "random.h"

void prompt() {
    printf("SDB> ");
    fflush(stdout);
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
    Parser *parser = new_parser();

    ParserNode *node = parser_parse(parser, scanner);

    if (parser->status == PARSESTATE_ERROR) {
        printf("Parse Error: %s\n", parser->errMsg);
    }

    if (node != NULL) {
        printf("%s\n", input);
        parser_print_node_tree(node, 0);

        StatementList *list = parser_node_convert(node);
        print_statement_list(list);
        free_statement_list(list);

        free_parser_node(node);
    }

    free_scanner(scanner);
    free_parser(parser);
}

int main() {
    setbuf(stderr, 0);

    InputBuffer *buffer = input_buffer_new();
    if (!init_random()) {
        fprintf(stderr, "Error: failed to init random device\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        prompt();
        read_input(buffer);

        if (strcasecmp(buffer->buffer, ".exit") == 0) {
            break;
        } else {
            parse_input(buffer->buffer);
        }
    }

    input_buffer_free(buffer);
    deinit_random();
    return EXIT_SUCCESS;
}