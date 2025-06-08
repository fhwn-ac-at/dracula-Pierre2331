#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <stdbool.h>
#include "board.h"

/* Executes a single move for a player:
  position: current square (0-based, -1 = before entering the board)
  Rolls the die and moves the player, returning the new position.
  Also returns via pointer (traversed_connection_index) the index
  of the connection used (-1 if none).
 */
int simulator_single_move(const Board *b, int position, int *roll_out, int *traversed_connection_index);

/* Simulates a single game and provides:
   - total_rolls: total number of die rolls until victory
   - path: array (buffer) of all rolled values
   - path_len: actual number of entries in path (up to path_capacity)
 
  The caller supplies path and path_capacity to record the sequence
  for later analysis (e.g. finding the shortest winning sequence).
 
  Returns true if the game is won within max_steps, false on timeout.
 */
bool simulator_play_single_game(const Board *b, int max_steps, int *total_rolls, int *path, int path_capacity, int *path_len, int *conn_path);


/**
 * Runs a batch of simulations and gathers aggregate statistics:
 *  - num_games: number of games to simulate
 *  - max_steps: maximum rolls per game before timeout
 *  - avg_rolls: output parameter for the average rolls until victory
 *  - min_rolls: output parameter for the minimum rolls needed in any win
 *  - best_path: output pointer to an array holding the roll sequence of the shortest game
 *  - best_path_len: output parameter for the length of best_path
 *  - connection_counts: output pointer to an array of length b->num_connections,
 *      where each entry counts how often that connection was traversed
 *
 * Memory for best_path and connection_counts is allocated by this function;
 * the caller is responsible for freeing both.
 *
 * Returns true if at least one game was won, false otherwise.
 */
bool simulator_run_batch(const Board *b, int num_games, int max_steps, double *avg_rolls, int *min_rolls, int **best_path, int *best_path_len, long **connection_counts);



#endif // SIMULATOR_H
