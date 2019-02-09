//
// Created by sam on 12/07/18.
//

#ifndef SDB_RANDOM_H
#define SDB_RANDOM_H

#include <stdint.h>
#include <stdbool.h>

bool init_random();
bool deinit_random();
uint32_t random_32(bool *success);

#endif //SDB_RANDOM_H
