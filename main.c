#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>

#include "file.c"
#include "utilities.h"

pthread_mutex_t mutex;
pthread_t threads[100];
int contador = 0;

Labyrinth* labyrinth;

// Function declaration
void* moveSnake(void* snake);

Cell* getMatrixCell(int x, int y) {
    Cell* selected_cell = &(labyrinth->matrix[y][x]);

    return selected_cell;
}

// Get the cell state
bool getCellState(Cell* cell) {

    pthread_mutex_lock(&mutex);
    CellState state = cell->state;
    pthread_mutex_unlock(&mutex);

    return state;
}

// Check if cell has the direction already registered
bool checkCellDirection(Cell* cell, Direction direction) {
    bool result = true;
    
    pthread_mutex_lock(&mutex);
    int max_num = cell->num_checked_directions;

    for (int i = 0; i < max_num; ++i) {
        if (cell->checked_directions[i] == direction) {
            result = false;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

// Update the cell direction array
void updateCellState(Cell* cell, Direction new_direction) {

    pthread_mutex_lock(&mutex);

    cell->state = ALREADY_CHECKED;
    cell->checked_directions[ cell->num_checked_directions ] = new_direction;
    cell->num_checked_directions++;

    pthread_mutex_unlock(&mutex);
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

    Cell* cell = getMatrixCell(x, y);

    return (x >= 0 && x < labyrinth->cols &&
            y >= 0 && y < labyrinth->rows &&
            checkCellDirection(cell, snake_direction) &&
            ( getCellState(cell) != BLOCK )) ;
}

Snake createSnake(int x, int y, Direction direction) {
    Snake snake;
    snake.x = x;
    snake.y = y;
    snake.direction = direction;
    snake.checked_spaces = 0;
    snake.state = RUNNING;

    return snake;
}

void startSnakeProcess(int x, int y, Direction direction) {
    Snake* snake = malloc( sizeof(Snake) );
    Cell* cell = getMatrixCell(x, y);

    *snake = createSnake(x, y, direction);

    updateCellState( cell , snake->direction);

    // Llamar a moveSnake
    pthread_create(&threads[contador], NULL, &moveSnake, snake);
    pthread_join(threads[contador], NULL);
}

// Create the threads for the adjacent cells (If available)
void createAdjacentThreads(Snake* snake, int new_x, int new_y) {
    int x1 = new_x - 1;
    int x2 = new_x + 1;
    int y1 = new_y - 1;
    int y2 = new_y + 1;

    Cell* cell1 = NULL;
    Cell* cell2 = NULL;

    // Check if adjacent cell are empty -> Create new thread
    if (snake->direction == UP || snake->direction == DOWN) {

        cell1 = getMatrixCell(x1, new_y);
        cell2 = getMatrixCell(x2, new_y);

        bool left_empty  = x1 >= 0 && (getCellState(cell1) != BLOCK);
        bool right_empty = x2 < (*labyrinth).cols && (getCellState(cell2) != BLOCK);
        
        if (left_empty && checkCellDirection(cell1, LEFT)) {
            // Create thread (x1, y, LEFT)
            startSnakeProcess(x1, new_y, LEFT);
        }
        if (right_empty && checkCellDirection(cell2, RIGHT)) {
            // Create thread (x2, y, RIGHT)
            startSnakeProcess(x2, new_y, RIGHT);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {

        cell1 = getMatrixCell(new_x, y1);
        cell2 = getMatrixCell(new_x, y2);

        bool up_empty    = y1 >= 0 && (getCellState(cell1) != BLOCK);
        bool down_empty  = y2 < (*labyrinth).rows && (getCellState(cell2) != BLOCK);

        if (up_empty && checkCellDirection(cell1, UP)) {
            // Create thread (x, y1, UP)
            startSnakeProcess(new_x, y1, UP);
        }
        if (down_empty && checkCellDirection(cell2, DOWN)) {
            // Create thread (x, y2, DOWN)
            startSnakeProcess(new_x, y2, DOWN);
        }
    }
}

// Initialize the snake movement
void* moveSnake(void* arg) {
    contador++;

    Snake snake = *(Snake*)arg;

    // Get the current snake position
    int new_x = 0;
    int new_y = 0;

    while (true) {
        new_x = snake.x;
        new_y = snake.y;

        Cell* cell = getMatrixCell(new_x, new_y);

        if ( getCellState(cell) == EXIT ) {
            snake.state = FINISHED;
            break;
        }

        updateCellState(cell, snake.direction);
        createAdjacentThreads(&snake, new_x, new_y);
        
        calculateNewPosition(snake.direction, &new_x, &new_y);

        // Check if the snake has left the labyrinth || cell has been traverse already
        if (!isValidSnakePosition(snake.direction, new_x, new_y)) {
            snake.state = STOPPED;
            break; // Terminate thread
        }
        
        // Set the new snake position
        snake.x = new_x;
        snake.y = new_y;
    }

    free(arg);
}

int main() {
    Snake snake;
    
    char filename[] = "maps/lab1 (Cerrado).txt"; // file route

    // read the labirynth from the file
    labyrinth = readLabyrinthFromFile(filename);
    if (labyrinth == NULL) {
        printf("Error reading the maze file.\n");
        return 1;
    }
    printLabyrinth(labyrinth);
    printf("\n");

    startSnakeProcess(0, 0, DOWN);

    // to print labyrinth
    printLabyrinth(labyrinth);

    // to free the memory used by the labyrinth
    freeLabyrinth(labyrinth);

    return 0;
}