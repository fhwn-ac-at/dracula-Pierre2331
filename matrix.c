#include <stdio.h>
#include <stdlib.h>

// ANSI-Farben
#define RED   "\033[1;31m"
#define BLUE  "\033[1;34m"
#define YELLOW "\033[1;33m"
#define RESET "\033[0m"

typedef struct {
    int from;
    int to;
    int weight;
} Edge;

void fill_matrix(int** matrix, int size, Edge* edges, int edge_count) {
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            matrix[i][j] = 0;

    for (int i = 0; i < edge_count; i++) {
        int f = edges[i].from;
        int t = edges[i].to;
        int w = edges[i].weight;

        if (f < 0 || f >= size || t < 0 || t >= size) {
            printf(RED "Invalid edge: %d -> %d\n" RESET, f, t);
            continue;
        }

        if (matrix[f][t] != 0) {
            printf(YELLOW "Warning: Overwriting edge %d -> %d\n" RESET, f, t);
        }

        matrix[f][t] = w;
        matrix[t][f] = -w;
    }

    
}

void print_matrix(int** matrix, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int val = matrix[i][j];

            if (val > 0)
                printf(BLUE "%4d" RESET, val);
            else if (val < 0)
                printf(RED "%4d" RESET, val);
            else
                printf("%4d" RESET, val);
        }
        printf("\n");
    }
}

int main() {
    int size = 10;

    Edge edges[] = {
        {0, 1, 10},
        {1, 2, 20},
        {2, 3, 30},
        {3, 4, 40},
        {4, 5, 45},
        {5, 6, 50},
        {6, 7, 60},
        {7, 8, 70},
        {8, 9, 80},
        {9, 0, 18},
        {8, 3, 90}
    };
    int edge_count = sizeof(edges) / sizeof(edges[0]);

    // Matrix allokieren
    int** matrix = malloc(size * sizeof(int*));
    for (int i = 0; i < size; i++)
        matrix[i] = malloc(size * sizeof(int));

    // Matrix füllen und drucken
    fill_matrix(matrix, size, edges, edge_count);
    print_matrix(matrix, size);

    // Speicher freigeben
    for (int i = 0; i < size; i++)
        free(matrix[i]);
    free(matrix);

    return 0;
}

