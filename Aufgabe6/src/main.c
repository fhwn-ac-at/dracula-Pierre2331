#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include "board.h"
#include "simulator.h"

static void print_statistics(int sample_size, int rows, int columns, int die_sides, int roll_limit, int num_snakes) {
    puts("+--------------------------------+");
    puts("|     Simulation statistics      |");
    puts("+--------------------------------+");
    printf("| Sample size: %5d            |\n", sample_size);
    printf("| Board size: %5d x %-5d      |\n", rows, columns);
    printf("| Dice size:  %5d              |\n", die_sides);
    printf("| Dice roll limit: %5d         |\n", roll_limit);
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

    int idx_s = 1; //index snake
    idx_l = 1; //index ladders
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
    int best_game = -1; // index of the best game (for the fastest path)
    bool exact_finish = true ; // true means players must land exactly on the last cell to win

    // storage for up to 32 -s pairs
    int max_pairs = 32;
    int (*pairs)[2] = malloc(sizeof(*pairs) * max_pairs);
    int pair_count = 0;

    int opt;
    while ((opt = getopt(argc, argv, "w:h:d:n:l:s:e:")) != -1) {
        char *endptr;
        long val;

        switch (opt) {
            case 'w':  // columns, must be 1–10
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || val < 1 || val > 10) {
                    fprintf(stderr, "Error: -w requires an integer 1–10 (got '%s')\n", optarg);
                    return EXIT_FAILURE;
                }
                cols = (int)val;
                break;

            case 'h':  // rows, must be 1–10
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || val < 1 || val > 10) {
                    fprintf(stderr, "Error: -h requires an integer 1–10 (got '%s')\n", optarg);
                    return EXIT_FAILURE;
                }
                rows = (int)val;
                break;

            case 'd':
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || val < 1 || val > 10) {
                    fprintf(stderr, "Error: -d requires an integer 1–10 (got '%s')\n", optarg);
                    return EXIT_FAILURE;
                }
                die_sides = (int)val;
                break;
            case 'n':  // number of simulations, must be a positive integer
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || val < 1) {
                    fprintf(stderr, "Error: -n requires a positive integer (got '%s')\n", optarg);
                    return EXIT_FAILURE;
                }
                sample_size = (int)val;
                break;
            case 'l':
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || val < 1) {
                    fprintf(stderr, "Error: -l requires a positive integer (got '%s')\n", optarg);
                    return EXIT_FAILURE;
                }
                roll_limit = (int)val;
                break;

            case 's':
                 if (pair_count >= max_pairs) {
                fprintf(stderr, "Error: Too many -s pairs (max %d)\n", max_pairs);
                return EXIT_FAILURE;
                }
                // parse start
                char *arg1 = optarg;
                errno = 0;
                long start = strtol(arg1, &endptr, 10);
                if (errno || *endptr != '\0') {
                    fprintf(stderr, "Error: -s start must be an integer (got '%s')\n", arg1);
                    return EXIT_FAILURE;
                }

                // parse end from next argv
                if (optind >= argc) {
                    fprintf(stderr, "Error: Missing end value for -s\n");
                    return EXIT_FAILURE;
                }
                char *arg2 = argv[optind++];
                errno = 0;
                long end = strtol(arg2, &endptr, 10);
                if (errno || *endptr != '\0') {
                    fprintf(stderr, "Error: -s end must be an integer (got '%s')\n", arg2);
                    return EXIT_FAILURE;
                }

                // range check
                int target = rows * cols - 1;
                if (start < 0 || start > target ||
                    end   < 0 || end   > target) {
                    fprintf(stderr,
                        "Error: -s values must be between 0 and %d (got %ld->%ld)\n",
                        target, start, end);
                    return EXIT_FAILURE;
                }

                // no self-loop
                if (start == end) {
                    fprintf(stderr,
                        "Error: -s start and end must differ (%ld->%ld)\n",
                        start, end);
                    return EXIT_FAILURE;
                }

                // no snake on final square
                if (start == target && end < start) {
                    fprintf(stderr,
                        "Error: cannot place a snake on the final square (%ld->%ld)\n",
                        start, end);
                    return EXIT_FAILURE;
                }

                // everything OK: save pair
                pairs[pair_count][0] = (int)start;
                pairs[pair_count][1] = (int)end;
                pair_count++;
                break;
            
            case 'e':
                errno = 0;
                val = strtol(optarg, &endptr, 10);
                if (errno || *endptr != '\0' || (val != 0 && val != 1)) {
                    fprintf(stderr, "Error: -e requires 0 (overshoot wins) or 1 (exact finish), got '%s'\n", optarg);
                    return EXIT_FAILURE;
                }
                exact_finish = (val == 1);
                break;

            default:
                fprintf(stderr, "Usage: %s [-w 1-10] [-h 1-10] [-d 1-10] [-n ≥1] [-l ≥1] [-e 0|1] [-s start end]...\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Validate board dimensions one more time
    if (rows < 1 || rows > 10 || cols < 1 || cols > 10) {
        fprintf(stderr,
            "Error: board dimensions must be 1–10 (got %dx%d)\n",
            rows, cols);
        return EXIT_FAILURE;
    }

    // Validate each snake/ladder pair
    int target = rows * cols - 1;
    for (int i = 0; i < pair_count; ++i) {
        int start = pairs[i][0];
        int end   = pairs[i][1];
        if (start < 0 || start > target || end < 0 || end > target) {
            fprintf(stderr, "Error: -s values must be 0..%d (got %d->%d)\n",
                    target, start, end);
            return EXIT_FAILURE;
        }
        if (start == end) {
            fprintf(stderr, "Error: -s start and end must differ (%d->%d)\n",
                    start, end);
            return EXIT_FAILURE;
        }
        // forbid a snake starting on the final square
        if (start == target && end < start) {
            fprintf(stderr,
                    "Error: cannot place a snake on the final square (%d->%d)\n",
                    start, end);
            return EXIT_FAILURE;
        }
    }

    // Build the board and add all connections
    Board *board = create_board(rows, cols, die_sides, exact_finish);
    if (!board) {
        fprintf(stderr, "Error: Board creation failed\n");
        return EXIT_FAILURE;
    }
    for (int i = 0; i < pair_count; ++i) {
        if (!board_add_connection(board, pairs[i][0], pairs[i][1])) {
            fprintf(stderr, "Invalid connection: %d -> %d\n",
                    pairs[i][0], pairs[i][1]);
            destroy_board(board);
            return EXIT_FAILURE;
        }
    }
    free(pairs);

    // Run simulations...
    double avg_rolls;
    int min_rolls;
    int *best_path = NULL;
    int best_len = 0;
    long *conn_counts = NULL;

    board_build_graph(board);

    bool ok = simulator_run_batch(board, sample_size, roll_limit, &avg_rolls, &min_rolls, &best_path, &best_len, &best_game, &conn_counts);
    if (!ok) {
        fprintf(stderr, "No game won or simulation error\n");
        destroy_board(board);
        return EXIT_FAILURE;
    }

    print_statistics(sample_size, rows, cols, die_sides, roll_limit, board->num_connections);
    print_results(avg_rolls, best_game + 1, min_rolls, best_path, best_len, board, conn_counts);

    // Cleanup
    free(best_path);
    free(conn_counts);
    destroy_board(board);
    return EXIT_SUCCESS;
}
