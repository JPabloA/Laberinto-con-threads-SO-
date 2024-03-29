#include <pthread.h>

#ifndef utilities
#define utilities

#define MAX_NUM_DIRECTION 4

// Enum: Possibles directions
typedef enum Direction {
    UP, DOWN, LEFT, RIGHT
} Direction;

// Enum: State of a single labyrinth cell
typedef enum CellState {
    EMPTY,
    BLOCK,
    ALREADY_CHECKED,
    EXIT
} CellState;

// Enum: State of a snake
typedef enum SnakeState {
    RUNNING,
    STOPPED
} SnakeState;

// Single cell of the labyrinth
typedef struct Cell {
    enum CellState state;
    enum Direction checked_directions[ MAX_NUM_DIRECTION ];
    int num_checked_directions;           // Inicia en 0
} Cell;

// Labyrinth structure
typedef struct Labyrinth {
    int rows;
    int cols;
    Cell** matrix;
} Labyrinth;

// Snake Structure
typedef struct Snake {
    int x, y;
    int checked_spaces;
    enum Direction direction;
    enum SnakeState state;
} Snake;

// Queue Node Structure
typedef struct Node {
    Snake snake;
    struct Node *next;
} Node;

// Queue Structure
typedef struct PriorityQueue {
    Node* first;
    Node* last;
    pthread_mutex_t mutex;
} PriorityQueue;

#endif