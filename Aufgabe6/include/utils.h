#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

// Function to generate a random integer between min and max (inclusive)
static inline int roll_die(int sides) {
    return (rand() % sides) + 1; // rand() % sides gives 0 to sides-1, so we add 1
}

#endif // UTILS_H
