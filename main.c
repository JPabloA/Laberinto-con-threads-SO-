#include <stdbool.h>
#include <pthread.h>

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
struct Node {
    Snake snake;
    struct Node *next;
};

// Queue Structure
typedef struct PriorityQueue {
    struct Node* first;
    struct Node* last;
    pthread_mutex_t mutex;
} PriorityQueue;

// Hacer la matriz de celdas (Laberinto)
// Ver la "estructura" para guardar/manejar quien recorre el laberinto
// Hacer las funciones que hagan el recorrido del laberinto 

// >>> No sé como se vaya a manejar el laberinto
Labyrinth labyrinth;

// Check if cell has a BLOCKED state
bool isCellBlocked(Cell* cell) {
    return cell->state == BLOCK;
}

// Check if cell has the direction already registered
bool checkCellDirection(Cell* cell, Direction direction) {
    int max_num = cell->num_checked_directions;

    for (int i = 0; i < max_num; ++i) {
        if (cell->checked_directions[i] == direction) {
            return false;
        }
    }
    return true;
}

// Update the cell direction array
void updateCellState(Cell* cell, Direction new_direction) {
    cell->checked_directions[ cell->num_checked_directions ] = new_direction;
    cell->num_checked_directions++;
}

// Calculate the next position of the snake
void calculateNewPosition(Direction snake_direction, int* new_x, int* new_y) {
    // Set the movement direction
    int movement = (snake_direction == RIGHT || snake_direction == DOWN) ? 1 : -1;

    // Change the position based on the direction
    if (snake_direction == RIGHT || snake_direction == LEFT) {
        *new_x += movement;
    }
    else {
        *new_y += movement;
    }
}

// Check if the snake position is valid (Not out of bound)
bool isValidSnakePosition(Direction snake_direction, int x, int y) {
    return (x >= 0 && x < labyrinth.cols &&
            y >= 0 && y < labyrinth.rows &&
            checkCellDirection(&labyrinth.matrix[y][x], snake_direction));
}

// Create the threads for the adjacent cells (If available)
void createAdjacentThreads(Snake* snake, int new_x, int new_y) {
    int x1 = new_x - 1;
    int x2 = new_x + 1;
    int y1 = new_y - 1;
    int y2 = new_y + 1;

    // Check if adjacent cell are empty -> Create new thread
    bool left_empty  = x1 >= 0 && !isCellBlocked( &labyrinth.matrix[new_y][x1] );
    bool right_empty = x2 < labyrinth.cols && !isCellBlocked( &labyrinth.matrix[new_y][x2] );
    bool up_empty    = y1 >= 0 && !isCellBlocked( &labyrinth.matrix[y1][new_x] );
    bool down_empty  = y2 < labyrinth.rows && !isCellBlocked( &labyrinth.matrix[y2][new_x] );

    if (snake->direction == UP || snake->direction == DOWN) {
        if (left_empty && checkCellDirection(&labyrinth.matrix[new_y][x1], LEFT)) {
            // Create thread (x1, y, LEFT)
        }
        if (right_empty && checkCellDirection(&labyrinth.matrix[new_y][x2], RIGHT)) {
            // Create thread (x2, y, RIGHT)
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {
        if (up_empty && checkCellDirection(&labyrinth.matrix[y1][new_x], UP)) {
            // Create thread (x, y1, UP)
        }
        if (down_empty && checkCellDirection(&labyrinth.matrix[y2][new_x], DOWN)) {
            // Create thread (x, y2, DOWN)
        }
    }
}

// Initialize the snake movement
void moveSnake(Snake* snake) {
    // Get the current snake position
    int new_x = snake->x;
    int new_y = snake->y;

    calculateNewPosition(snake->direction, &new_x, &new_y);

    // Check if the snake has left the labyrinth || cell has been traverse already
    if (!isValidSnakePosition(snake->direction, new_x, new_y)) {
        snake->state = STOPPED;
        return; // Terminate thread
    }

    // Set the new snake position
    // Update the cell with the snake direction
    snake->x = new_x;
    snake->y = new_y;
    updateCellState(&labyrinth.matrix[new_y][new_x], snake->direction);

    createAdjacentThreads(snake, new_x, new_y);

}

int main() {
    return 0;
}
