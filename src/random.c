//
// Created by sam on 12/07/18.
//

#include <stdio.h>
#include <assert.h>
#include "random.h"

FILE * randomFile = NULL;
bool init_random() {
    randomFile = fopen("/dev/urandom", "r");
    return randomFile != NULL;
}
bool deinit_random() {
    int result = 0;
    if (randomFile != NULL) {
        result = fclose(randomFile);
        randomFile = NULL;
    }
    return result == 0;
}
uint32_t random_32(bool *success) {
    assert(randomFile != NULL);
    uint32_t output = 0;

    if (fread(&output, sizeof(uint32_t), 1, randomFile) < 1) {
        if (success != NULL) {
            *success = false;
        }
        return 0;
    }
    if (success != NULL) {
        *success = true;
    }
    return output;
}