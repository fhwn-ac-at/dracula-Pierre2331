#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <time.h>
#include "board.h"
#include "simulator.h"

static void print_statistics(int sample_size, int rows, int columns, int die_sides, int roll_limit, int num_snakes) {
    puts("+--------------------------------+");
    puts("|     Simulation statistics      |");
    puts("+--------------------------------+");
    printf("| Sample size: %5d             |\n", sample_size);
    printf("| Board size: %5d x %-5d      |\n", rows, columns);
    printf("| Dice size:  %5d              |\n", die_sides);
    printf("| Dice roll limit: %3d          |\n", roll_limit);
    printf("| Snakes & Ladders: %3d          |\n", num_snakes);
    puts("+--------------------------------+");
}

static void print_results(double avg_rolls, int fastest_id, int fastest_rolls, int *fastest_path, int fastest_len, Board *board, long *connection_counts) {
    int total_snakes = 0;
    int total_ladders = 0;

    //count seperate snakes and ladders
    for (int i = 0; i < board->num_connections; ++i) {
        if (board->connections[i].is_ladder) {
            total_ladders++;
        } else {
            total_snakes++;
        }
    }

    printf("| Average rolls to win: %8.4f |\n", avg_rolls);
    puts("| Fastest simulation:            |");
    printf("|   # %3d with %3d rolls         |\n",
           fastest_id, fastest_rolls);
    printf("| Rolls in this simulation:      |\n|   ");
    for (int i = 0; i < fastest_len; ++i) {
        printf("%d%s", fastest_path[i], (i+1<fastest_len) ? " -> " : "");
    }
    printf("                  |\n\n");

    puts("| Snakes & Ladders traversal counts:         |");
    puts("+--------------------------------------------+");

    // simuilate the traversal counts
    long sum_snake = 0, sum_ladder = 0;
    for (int i = 0; i < board->num_connections; ++i) {
        if (board->connections[i].is_ladder) sum_ladder += connection_counts[i];
        else                                  sum_snake  += connection_counts[i];
    }

    int idx_s = 1, idx_l = 1;
    for (int i = 0; i < board->num_connections; ++i) {
        Connection *c = &board->connections[i];
        if (c->is_ladder) {
            double pct = sum_ladder ? (100.0 * connection_counts[i] / sum_ladder) : 0.0;
            printf("| Ladder #%d (from %2d to %2d) - %4ld times (%.2f%% of ladders) |\n",
                   idx_l++, c->start, c->end, connection_counts[i], pct);
        } else {
            double pct = sum_snake ? (100.0 * connection_counts[i] / sum_snake) : 0.0;
            printf("| Snake  #%d (from %2d to %2d) - %4ld times (%.2f%% of snakes)  |\n",
                   idx_s++, c->start, c->end, connection_counts[i], pct);
        }
    }
    puts("+--------------------------------------------+");
}

int main(int argc, char *argv[]) {
    int rows = 10, cols = 10;
    int die_sides = 6;
    int sample_size = 1000;
    int roll_limit = 1000;

    // temporärer Speicher für -s Paare
    int max_pairs = 32;
    int (*pairs)[2] = malloc(sizeof(*pairs) * max_pairs);
    int pair_count = 0;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:d:n:l:s:")) != -1) {
        switch (opt) {
            case 'w': cols        = atoi(optarg); break;
            case 'h': rows        = atoi(optarg); break;
            case 'd': die_sides   = atoi(optarg); break;
            case 'n': sample_size = atoi(optarg); break;
            case 'l': roll_limit  = atoi(optarg); break;
            case 's':
                if (pair_count >= max_pairs) {
                    fprintf(stderr, "Too many -s pairs\n");
                    return EXIT_FAILURE;
                }
                pairs[pair_count][0] = atoi(optarg);
                if (optind < argc) {
                    pairs[pair_count][1] = atoi(argv[optind++]);
                } else {
                    fprintf(stderr, "Missing end value for -s\n");
                    return EXIT_FAILURE;
                }
                pair_count++;
                break;
            default:
                fprintf(stderr,
                        "Usage: %s [-w cols] [-h rows] [-d dice] "
                        "[-n simulations] [-l roll_limit] [-s start end]...\n",
                        argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Validate input parameters
    // Check board dimensions
    if(rows < 1 || rows > 10 || cols < 1 || cols > 10) {
        fprintf(stderr, "Error: Invalid board dimensions: %dx%d (must be between 1x1 and 10x10)\n", rows, cols);
        return EXIT_FAILURE;
    }
    // validate each snake/ladder pair
    int target = rows * cols - 1; // last cell index
    for (int i = 0; i < pair_count; ++i) {
        int start = pairs[i][0];
        int end   = pairs[i][1];
        if (start < 0 || start > target || end < 0 || end > target) {
            fprintf(stderr, "Invalid snake/ladder pair: %d -> %d\n", start, end);
            return EXIT_FAILURE;
        }
        if (start == end) {
            fprintf(stderr, "Error: -s start and end must differ (got %d->%d)\n", start, end);
            return EXIT_FAILURE;
        }
        if (end == target && !die_sides) {
            fprintf(stderr, "Error: cannot place a snake starting on the final square (%d->%d)\n", start, end);
            return EXIT_FAILURE;
        }
    }

    // CREATE BOARD AND ADD CONNECTIONS
    Board *board = create_board(rows, cols, die_sides, true);
    if (!board) {
        fprintf(stderr, "Error: Board creation failed\n");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < pair_count; ++i) {
        int start = pairs[i][0];
        int end   = pairs[i][1];
        if (!board_add_connection(board, start, end)) {
            fprintf(stderr,
                    "Invalid connection: %d -> %d\n",
                    start, end);
            return EXIT_FAILURE;
        }
    }
    free(pairs);

    // simulation start
    double avg_rolls;
    int min_rolls;
    int *best_path = NULL;
    int best_len = 0;
    long *conn_counts = NULL;

    bool ok = simulator_run_batch(board, sample_size, roll_limit, &avg_rolls, &min_rolls, &best_path, &best_len, &conn_counts);
    if (!ok) {
        fprintf(stderr, "No game was won or simulation error\n");
        destroy_board(board);
        return EXIT_FAILURE;
    }

    print_statistics(sample_size, rows, cols, die_sides, roll_limit, board->num_connections);
    print_results(avg_rolls, /* fastest_id = */ 1 +  0, min_rolls, best_path, best_len, board, conn_counts);

    // cleanup
    free(best_path);
    free(conn_counts);
    destroy_board(board);
    return EXIT_SUCCESS;
}
