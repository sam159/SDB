//
// Created by Sam on 15/05/2018.
//

#include "InputBuffer.h"

InputBuffer* input_buffer_new() {
    InputBuffer* buffer = malloc(sizeof(InputBuffer));
    buffer->buffer = NULL;
    buffer->bufferLength = 0;
    buffer->inputLength = 0;

    return buffer;
}

void input_buffer_free(InputBuffer* buffer) {
    if (buffer->buffer != NULL) {
        free(buffer->buffer);
    }
    buffer->buffer = NULL;
    buffer->inputLength = 0;
    buffer->bufferLength = 0;
    free(buffer);
}