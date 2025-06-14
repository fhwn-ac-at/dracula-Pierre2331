#include <stdio.h>
#include <stdbool.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "simulator.h"
#include "utils.h"

int simulator_single_move(const Board *b, int position, int *roll_out, int *traversed_connection_index) {
    if (!b || !traversed_connection_index || !roll_out) return position;

    // roll the die and compute tentative new position
    int roll = roll_die(b->die_sides);
    *roll_out = roll; // output the rolled value

    //int new_pos = board_move(b, position, roll);
    int new_pos;
    if (position >= 0 && position < b->total_cells) {
        new_pos = b->adj[position][roll - 1];
    } else {
        new_pos = board_move(b, position, roll);
    }

    // check for a snake or ladder at new_pos
    for (int i = 0; i < b->num_connections; ++i) {
        if (b->connections[i].start == new_pos) {
            *traversed_connection_index = i;
            return b->connections[i].end;
        }
    }

    *traversed_connection_index = -1;
    return new_pos;
}

bool simulator_play_single_game(const Board *b, int max_steps, int *total_rolls, int *path, int path_capacity, int *path_len, int *conn_path) {
    if (!b || !total_rolls || !path || !path_len || !conn_path) return false;

    int position = -1;          // start off the board
    int rolls = 0;
    int idx_sequence = 0;

    while (rolls < max_steps) {
        int roll_value = 0; // to capture the roll value

        int connection_index = -1;
        // perform one move (does not return roll value here)
        int next_pos = simulator_single_move(b, position, &roll_value, &connection_index);
        rolls++;

        // (hint: store roll values here if you extend the code)
        if (idx_sequence < path_capacity) {
            path[idx_sequence] = roll_value; 
            conn_path[idx_sequence] = connection_index; // store connection index
        }
        idx_sequence++;
        position = next_pos;
        if (position == b->total_cells - 1) {
            *total_rolls = rolls;
            *path_len = idx_sequence;     // no path recorded in this version
            return true;
        }
    }

    // reached max_steps without winning
    return false;
}

bool simulator_run_batch(const Board *b, int num_games, int max_steps, double *avg_rolls, int *min_rolls, int **best_path, int *best_path_len, long **connection_counts) {
    if (!b || num_games <= 0 || !avg_rolls || !min_rolls || !best_path || !best_path_len || !connection_counts) {
        return false;
    }

    int num_conn = b->num_connections;

    // Allocate array to count how often each connection is used
    long *conn_counts = calloc(num_conn, sizeof(long));
    if (!conn_counts) return false;

    int wins = 0;
    long sum_rolls = 0;
    int best_rolls = INT_MAX;
    int *best_p = NULL;
    int best_len = 0;

    // Temporary buffer to store the roll sequence of each game
    int *path_buffer = malloc(sizeof(int) * max_steps);
    int *conn_buffer = malloc(sizeof(int) * max_steps);
    if (!path_buffer || !conn_buffer) {
        free(conn_counts);
        free(path_buffer);
        free(conn_buffer);
        return false;
    }

    for (int g = 0; g < num_games; ++g) {
        int rolls = 0;
        int path_len = 0;
        bool won = simulator_play_single_game(b, max_steps, &rolls, path_buffer, max_steps, &path_len, conn_buffer);

        if (!won) continue;
        wins++;
        sum_rolls += rolls;

        for (int i = 0; i < path_len; ++i) {
            int conn_index = conn_buffer[i];
            if (conn_index >= 0 && conn_index < num_conn) {
                conn_counts[conn_index]++;
            }
        }

        // Save the shortest winning sequence
        if (rolls < best_rolls) {
            best_rolls = rolls;
            best_len = path_len;
            free(best_p);
            best_p = malloc(sizeof(int) * path_len);
            if (best_p) {
                memcpy(best_p, path_buffer, sizeof(int) * path_len);
            }
        }
    }

    free(path_buffer);
    free(conn_buffer);

    if (wins == 0) {
        free(conn_counts);
        return false;
    }

    // Compute average rolls and set output parameters
    *avg_rolls = (double)sum_rolls / wins;
    *min_rolls = best_rolls;
    *best_path = best_p;
    *best_path_len = best_len;
    *connection_counts = conn_counts;
    return true;
}