#include <pthread.h>

// Enum: Possibles directions
enum Direction {
    UP, DOWN, LEFT, RIGHT
};

// Enum: State of a single labyrinth cell
enum CellState {
    EMPTY,
    BLOCK,
    EXIT
};

// Enum: State of a snake
enum SnakeState {
    RUNNING,
    STOPPED
};

// Single cell of the labyrinth
typedef struct Cell {
    enum CellState state;
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
    Node *next;
} Node;

// Queue Structure
typedef struct PriorityQueue {
    Node* first;
    Node* last;
    pthread_mutex_t mutex;
} PriorityQueue;


// Hacer la matriz de celdas (Laberinto)
// Ver la "estructura" para guardar/manejar quien recorre el laberinto
// Hacer las funciones que hagan el recorrido del laberinto 

int main() {
    /* code */
    return 0;
}
