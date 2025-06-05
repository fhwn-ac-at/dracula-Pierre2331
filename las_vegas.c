#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

typedef struct point {
    float x;
    float y;
} Point;

Point random_point() {
    Point p;
    p.x = ((float)rand() / (float)RAND_MAX) * 2 -1;
    p.y = ((float)rand() / (float)RAND_MAX) * 2 -1;
    return p;
}

bool is_point_in_unit_circle(Point p) {
    return (p.x * p.x + p.y * p.y) <= 1;
}

Point random_point_in_unit_circle() {
    Point p;
    do {
        p = random_point();
    } while (!is_point_in_unit_circle(p));
    return p;
}

float approximate_pi(int num_points) {
    int inside_circle = 0;
    for (int i = 0; i < num_points; i++) {
        Point p = random_point();
        if (is_point_in_unit_circle(p)) {
            inside_circle++;
        }
    }
    return 4.0 * inside_circle / num_points;
}
int main() {
    srand(time(NULL));
    int num_points = 1000000; // Adjust as needed for accuracy
    float pi_approximation = approximate_pi(num_points);
    printf("Approximate value of Pi: %f\n", pi_approximation);
    return 0;
}