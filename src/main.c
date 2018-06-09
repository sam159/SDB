#include <stdio.h>
#include <stdbool.h>
#include <mem.h>

#include "InputBuffer.h"
#include "scanner.h"
#include "parser.h"

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
    Parser *parser = new_parser();

    ParserNode *node = parser_parse(parser, scanner);

    if (parser->status == PARSESTATE_ERROR) {
        printf("Parse Error: %s\n", parser->errMsg);
    }

    if (node != NULL) {
        printf("%s\n", input);
        parser_print_node_tree(node, 0);
        free_parser_node(node);
    }

    free_scanner(scanner);
    free_parser(parser);

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