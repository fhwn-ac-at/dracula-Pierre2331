#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "board.h"


Board *create_board(int rows, int cols, int die_sides, bool exact_finish) {
    Board *board = malloc(sizeof(Board));
    if (!board) {
        return NULL; // Memory allocation failed
    }
    
    board->rows = rows;
    board->cols = cols;
    board->total_cells = rows * cols;
    board->num_connections = 0;
    board->connections = NULL; // No connections initially
    board->die_sides = die_sides;
    board->exact_finish = exact_finish;

    return board;
}

void destroy_board(Board *board) {
    if (board) {
        free(board->connections); // Free the connections array if it was allocated
        free(board); // Free the board structure itself
    }
}

int board_move(const Board *b, int position, int roll) {
    if (!b) return position;
    int target = position + roll;
    if (b->exact_finish) {
        if (target > b->total_cells - 1) {
            // no turn if overthrows
            return position;
        }
    } else {
        if (target > b->total_cells - 1) {
            // wins if overthrows
            return b->total_cells - 1;
        }
    }
    
    return target;
}

void board_print(const Board *b) {
    if (!b) return;
    printf("Board: %d x %d, Diesites: %d, exact_finish: %s\n",
           b->rows, b->cols, b->die_sides,
           b->exact_finish ? "ja" : "nein");
    printf("Count Snakes/Ladders: %d\n", b->num_connections);
    for (int i = 0; i < b->num_connections; ++i) {
        Connection *c = &b->connections[i];
        printf("  %s from %d to %d\n",
               c->is_ladder ? "Ladder" : "Snakes",
               c->start, c->end);
    }
}

// check if there is a connection given by 
static bool connection_exists(const Board *b, int start, int end) {
    //if (!b || !b->connections) return false;
    for (int i = 0; i < b->num_connections; ++i) {
        if (b->connections[i].start == start && b->connections[i].end == end) {
            return true;
        }
    }
    return false;
}

bool board_add_connection(Board *b, int start, int end) {
    if (!b) return false;
    int last_square = b->total_cells - 1;

    // 1. Validity check: start and end within range?
    if (start < 0 || start > last_square ||
        end   < 0 || end   > last_square) {
        fprintf(stderr,
                "board_add_connection: Invalid square number (start=%d, end=%d)\n",
                start, end);
        return false;
    }

    // 2. No duplicate, no self-loop
    if (start == end) {
        fprintf(stderr,
                "board_add_connection: Start and end are the same (%d)\n",
                start);
        return false;
    }
    if (start == last_square) {
        fprintf(stderr,
                "board_add_connection: Cannot start on the last square (%d)\n",
                start);
        return false;
    }
    if (connection_exists(b, start, end)) {
        fprintf(stderr,
                "board_add_connection: Connection already exists (start=%d, end=%d)\n",
                start, end);
        return false;
    }

    // 3. Ensure no overlap with other snakes/ladders at the same square
    for (int i = 0; i < b->num_connections; ++i) {
        if (b->connections[i].start == start) {
            fprintf(stderr,
                    "board_add_connection: A connection already starts at square %d\n",
                    start);
            return false;
        }
        if (b->connections[i].end == start) {
            // Conservatively disallow landing on another connection’s end
            fprintf(stderr,
                    "board_add_connection: Another connection ends at square %d\n",
                    start);
            return false;
        }
    }

    // 4. Grow the array by one entry
    Connection *new_array = realloc(
        b->connections,
        sizeof(Connection) * (b->num_connections + 1)
    );
    if (!new_array) {
        perror("realloc");
        return false;
    }
    b->connections = new_array;

    // 5. Add the new connection
    Connection c;
    c.start      = start;
    c.end        = end;
    c.is_ladder  = (end > start);
    b->connections[b->num_connections] = c;
    b->num_connections++;

    return true;
}