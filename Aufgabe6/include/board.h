#ifndef BOARD_H
#define BOARD_H

#include <stdbool.h>

/* one connection define a snake or ladder
    * start: the start point of the snake or ladder
    * end: the end point of the snake or ladder
*/
typedef struct {
    int start;
    int end;
    bool is_ladder; // true if it's a ladder, false if it's a snake
} Connection;

typedef struct {
    int rows;
    int cols;
    int total_cells;
    int num_connections;
    Connection *connections; // array of connections (snakes and ladders)
    int die_sides; // number of sides on the die
    bool exact_finish; // true if players must land exactly on the last cell to win, false otherwise
} Board;

Board *create_board(int rows, int cols, int die_sides, bool exact_finish);
void destroy_board(Board *board);

bool board_add_connection(Board *board, int start, int end);
void board_print(const Board *board);
int board_move(const Board *board, int position, int roll);

#endif // BOARD_H
