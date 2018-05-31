//
// Created by Sam on 15/05/2018.
//

#ifndef SDB_INPUTBUFFER_H
#define SDB_INPUTBUFFER_H

#include <stddef.h>
#include <sys/types.h>
#include <malloc.h>

struct InputBuffer_t {
    char* buffer;
    size_t bufferLength;
    ssize_t inputLength;
};
typedef struct InputBuffer_t InputBuffer;

InputBuffer* input_buffer_new();
void input_buffer_free(InputBuffer* buffer);

#endif //SDB_INPUTBUFFER_H
