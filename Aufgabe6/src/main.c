#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "board.h"
#include "simulator.h"

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;  // reserved for future command-line options maybe haha

    // Seed the random number generator once at startup
    srand((unsigned)time(NULL));

    // Create a 10×10 board, 6-sided die, must land exactly on the last square to win
    Board *board = create_board(10, 10, 6, true);
    if (!board) {
        fprintf(stderr, "Board creation failed\n");
        return EXIT_FAILURE;
    }

    // Define snakes and ladders (using 0-based indices)
    board_add_connection(board, 1, 38);
    board_add_connection(board, 4, 14);
    board_add_connection(board, 9, 31);
    board_add_connection(board, 16, 6);
    board_add_connection(board, 21, 42);
    board_add_connection(board, 28, 84);
    board_add_connection(board, 36, 44);
    board_add_connection(board, 48, 26);
    board_add_connection(board, 49, 11);
    board_add_connection(board, 51, 67);
    board_add_connection(board, 56, 53);
    board_add_connection(board, 62, 19);
    board_add_connection(board, 64, 60);
    board_add_connection(board, 71, 91);
    board_add_connection(board, 80, 100);
    board_add_connection(board, 87, 24);
    board_add_connection(board, 93, 73);
    board_add_connection(board, 95, 75);
    board_add_connection(board, 98, 78);

    // Simulation parameters
    int num_games = 10000;
    int max_steps = 10000;

    // Outputs for aggregated statistics
    double avg_rolls = 0.0;
    int min_rolls = 0;
    int *best_path = NULL;
    int best_path_len = 0;
    long *connection_counts = NULL;

    // Run the batch of simulations
    bool success = simulator_run_batch(board, num_games, max_steps, &avg_rolls, &min_rolls, &best_path, &best_path_len,  &connection_counts);
    if (!success) {
        fprintf(stderr, "No games won or simulation error\n");
        destroy_board(board);
        return EXIT_FAILURE;
    }

    // Print summary statistics
    printf("=== Statistics after %d games ===\n", num_games);
    printf("Average rolls to win: %.2f\n", avg_rolls);
    printf("Minimum rolls for a win: %d\n", min_rolls);
    printf("Fastest winning sequence (%d rolls):\n", min_rolls);
    for (int i = 0; i < best_path_len; ++i) {
        printf("%d ", best_path[i]);
    }
    printf("\n\n");

    // Print how often each snake/ladder was traversed
    printf("Snake & Ladder frequencies (index: start -> end : count):\n");
    for (int i = 0; i < board->num_connections; ++i) {
        Connection *c = &board->connections[i];
        printf("  [%2d] %2d -> %2d : %ld\n", i, c->start, c->end, connection_counts[i]);
    }

    // Clean up
    free(best_path);
    free(connection_counts);
    destroy_board(board);
    return EXIT_SUCCESS;
}
