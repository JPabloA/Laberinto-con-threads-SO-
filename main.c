#include <stdbool.h>
#include <pthread.h>

// Enum: Possibles directions
enum Direction {
    UP, DOWN, LEFT, RIGHT
};

// Enum: State of a single labyrinth cell
enum CellState {
    EMPTY,
    BLOCK,
    ALREADY_CHECK,
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
    enum Direction checked_directions[4];
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


// >>> Cambiar el 999 por el rows|cols del laberinto
Labyrinth labyrinth;

//Set the new snake position
void moveSnake(Snake* snake) {
    // Get the current snake position
    int new_x = snake->x;
    int new_y = snake->y;

    // Set the movement direction
    int movement = (snake->direction == RIGHT || snake->direction == DOWN) ? 1 : -1;

    // Change the position based on the direction
    if (snake->direction == RIGHT || snake->direction == LEFT) {
        new_x += movement;
    }
    else {
        new_y += movement;
    }

    // Check if the snake has left the labyrinth
    if (new_x < 0 || new_x > labyrinth.cols || new_y < 0 || new_y > labyrinth.rows) {
        snake->state = STOPPED;
        return;
    }

    // Set the new snake position
    snake->x = new_x;
    snake->y = new_y;

    // Check if adyacent cell are empty -> Create new thread
    int x1 = new_x - 1;
    int x2 = new_x + 1;
    int y1 = new_y - 1;
    int y2 = new_y + 1;

    bool left_empty  = x1 >= 0 && labyrinth.matrix[new_y][x1].state == EMPTY;
    bool right_empty = x2 < labyrinth.cols && labyrinth.matrix[new_y][x2].state == EMPTY;
    bool up_empty    = y1 >= 0 && labyrinth.matrix[y1][new_x].state == EMPTY;
    bool down_empty  = y2 < labyrinth.rows && labyrinth.matrix[y2][new_x].state == EMPTY;

    if (left_empty) {
        // Create thread (x1, y, LEFT)
    }
    if (right_empty) {
        // Create thread (x2, y, RIGHT)
    }
    if (up_empty) {
        // Create thread (x, y1, UP)
    }
    if (down_empty) {
        // Create thread (x, y2, DOWN)
    }
}


int main() {
    /* code */
    return 0;
}
