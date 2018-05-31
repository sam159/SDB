#include <stdio.h>
#include <stdbool.h>
#include <mem.h>

#include "InputBuffer.h"

void prompt() {
    printf("SDB> ");
}

void readInput(InputBuffer *buffer) {
    ssize_t read = getline(&buffer->buffer, &buffer->bufferLength, stdin);

    if (read <= 0) {
        printf("Failed to read input\n");
        exit(EXIT_FAILURE);
    }

    //Remove newline
    buffer->inputLength = read - 1;
    buffer->buffer[read - 1] = 0;
}

int main() {
    InputBuffer *buffer = input_buffer_new();

    while (true) {
        prompt();
        readInput(buffer);

        if (strcmpi(buffer->buffer, ".exit") == 0) {
            break;
        } else {
            printf("Unknown Command or Query\n");
        }
    }

    input_buffer_free(buffer);
    return EXIT_SUCCESS;
}