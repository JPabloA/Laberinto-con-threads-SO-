#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "file.c"
#include "utilities.h"

#define MAX_NUMBER_CREATED 100

pthread_mutex_t mutex;
pthread_mutex_t snake_mutex;
pthread_mutex_t threads_mutex;

pthread_t threads[100];
pthread_t threadToPrint;
Snake *snakes[100] = { NULL };
int threadCounter = 1;
int snakeCounter = 0;
bool keepPrinting = true;

Labyrinth* labyrinth;

// **********************************************************
// ***** Function Declarations                          *****
// **********************************************************

Snake createSnake(int x, int y, Direction direction);
Cell* getMatrixCell(int x, int y);
CellState getCellState(Cell* cell);

bool checkCellDirection(Cell* cell, Direction direction);
bool isValidSnakePosition(Direction snake_direction, int x, int y);

void moveSnake(Snake* snake);
void updateCellState(Cell* cell, Direction new_direction);
void calculateNewPosition(Direction snake_direction, int* new_x, int* new_y);
void createThreadSnake(int x, int y, Direction direction);
void createAdjacentThreads(Snake* snake, int new_x, int new_y);

void* startSnakeProcess(void* args);

// Create and return an snake struct object
Snake createSnake(int x, int y, Direction direction) {
    Snake snake;
    snake.x = x;
    snake.y = y;
    snake.direction = direction;
    snake.checked_spaces = 0;
    snake.state = RUNNING;

    return snake;
}
// Return a pointer to the matrix cell coordinates specified
Cell* getMatrixCell(int x, int y) {
    return &(labyrinth->matrix[y][x]);
}
// Return the cell state
CellState getCellState(Cell* cell) {
    pthread_mutex_lock(&mutex);
    CellState state = cell->state;
    pthread_mutex_unlock(&mutex);

    return state;
}
// Check the verified cell directions
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
// Check if the x and y coordinates are valid
bool isValidSnakePosition(Direction snake_direction, int x, int y) {
    Cell* cell = getMatrixCell(x, y);

    return (x >= 0 && x < labyrinth->cols &&
            y >= 0 && y < labyrinth->rows &&
            checkCellDirection(cell, snake_direction) &&
            (getCellState(cell) != BLOCK )) ;
}
// Update the cell direction array
void updateCellState(Cell* cell, Direction new_direction) {

    pthread_mutex_lock(&mutex);
    cell->state = ALREADY_CHECKED;
    cell->checked_directions[ cell->num_checked_directions ] = new_direction;
    cell->num_checked_directions++;
    pthread_mutex_unlock(&mutex);
}
// Calculate the next position based on its direction
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
// Create and return the params of a snake thread
ThreadArgs* createThreadParams(int x, int y, Direction direction) {
    ThreadArgs* params = malloc(sizeof(ThreadArgs));

    params->x = x;
    params->y = y;
    params->direction = direction;

    return params;
}
// Create and start the snake thread
void createThreadSnake(int x, int y, Direction direction) {
    ThreadArgs* params = createThreadParams(x, y, direction);


    pthread_mutex_lock(&threads_mutex);
    pthread_create(&threads[threadCounter], NULL, startSnakeProcess, (void*)params);
    threadCounter++;
    pthread_mutex_unlock(&threads_mutex);
}

// Thread function: Start the snake process
void* startSnakeProcess(void* args) {
    ThreadArgs* params = (ThreadArgs*)args;

    int x = params->x;
    int y = params->y;
    Direction direction = params->direction;

    // array to save snakes
    pthread_mutex_lock(&snake_mutex);
    snakeCounter++;
    snakes[snakeCounter] = malloc(sizeof(Snake));
    *snakes[snakeCounter] = createSnake(x, y, direction);
    pthread_mutex_unlock(&snake_mutex);

    moveSnake(snakes[snakeCounter]);

    free(args);

    return NULL;
}

// Check if adjacent spaces are available (If so -> Create thread)
void createAdjacentThreads(Snake* snake, int new_x, int new_y) {
    int x1 = new_x - 1; int x2 = new_x + 1;
    int y1 = new_y - 1; int y2 = new_y + 1;

    Cell* cell1 = NULL; Cell* cell2 = NULL;

    if (snake->direction == UP || snake->direction == DOWN) {

        cell1 = getMatrixCell(x1, new_y);
        cell2 = getMatrixCell(x2, new_y);

        bool left_empty  = x1 >= 0 && (getCellState(cell1) != BLOCK);
        bool right_empty = x2 < (*labyrinth).cols && (getCellState(cell2) != BLOCK);
        
        if (left_empty && checkCellDirection(cell1, LEFT)) {
            // Create thread (x1, y, LEFT)
            createThreadSnake(x1, new_y, LEFT);
        }
        if (right_empty && checkCellDirection(cell2, RIGHT)) {
            // Create thread (x2, y, RIGHT)
            createThreadSnake(x2, new_y, RIGHT);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {

        cell1 = getMatrixCell(new_x, y1);
        cell2 = getMatrixCell(new_x, y2);

        bool up_empty    = y1 >= 0 && (getCellState(cell1) != BLOCK);
        bool down_empty  = y2 < (*labyrinth).rows && (getCellState(cell2) != BLOCK);

        if (up_empty && checkCellDirection(cell1, UP)) {
            // Create thread (x, y1, UP)
            createThreadSnake(new_x, y1, UP);
        }
        if (down_empty && checkCellDirection(cell2, DOWN)) {
            // Create thread (x, y2, DOWN)
            createThreadSnake(new_x, y2, DOWN);
        }
    }
}

// Start the snake movement through the maze
void moveSnake(Snake* snake) {
    int new_x = 0;
    int new_y = 0;
    Cell* cell = NULL;

    while (true) {
        new_x = snake->x;
        new_y = snake->y;
        cell = getMatrixCell(new_x, new_y);

        if ( getCellState(cell) == EXIT ) {
            snake->state = FINISHED;
            break;
        }

        sleep(1);
        updateCellState(cell, snake->direction);
        createAdjacentThreads(snake, new_x, new_y);
        
        // Check if the snake has left the labyrinth || cell has been traverse already
        calculateNewPosition(snake->direction, &new_x, &new_y);
        if (!isValidSnakePosition(snake->direction, new_x, new_y)) {
            snake->state = STOPPED;
            break;
        }
        
        // Set the new snake position
        pthread_mutex_lock(&snake_mutex);
        snake->x = new_x;
        snake->y = new_y;
        snake->checked_spaces++;
        pthread_mutex_unlock(&snake_mutex);
    }
}

void* printTheLabyrinth(void* args){

    bool flag = true;

    while (keepPrinting){

        flag = false;

        sleep(1);

        printf("\x1b[H");
        printf("\x1b[J");

        printLabyrinth(labyrinth);

        pthread_mutex_lock(&threads_mutex);
        printf("Threads Count: %d\n", threadCounter);
        for (int i = 0; i < 100; ++i) {
            if (threads[i]) {
                printf("Thread ID: %lu\n", (unsigned long)threads[i]);
            }
        }
        pthread_mutex_unlock(&threads_mutex);


        pthread_mutex_lock(&snake_mutex);
        for (int i=0; i < 100; i++){
            if (snakes[i]){
                // printf("fila: %d  columna: %d \n", snakes[i]->x, snakes[i]->y);
                if (snakes[i]->state == RUNNING)
                {
                    flag = true;
                }
            }
        }
        pthread_mutex_unlock(&snake_mutex);

        if (!flag) {
            break;
        }
    }

    pthread_exit(NULL);
    return NULL;
}

int main() {    
    char filename[] = "maps/lab5.txt"; // file route

    // read the labirynth from the file
    labyrinth = readLabyrinthFromFile(filename);
    if (labyrinth == NULL) {
        printf("Error reading the maze file.\n");
        return 1;
    }
    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&snake_mutex, NULL);
    pthread_mutex_init(&threads_mutex, NULL);

    ThreadArgs* params = createThreadParams(0, 0, DOWN);
    pthread_create(&threads[0], NULL, startSnakeProcess, (void*)params);

    // Thread to print the maze
    pthread_create(&threadToPrint, NULL, printTheLabyrinth, NULL);

    for (int i = 0; i < 100; i++) {
        if (threads[i]) {
            if (pthread_join(threads[i], NULL) != 0) {
                printf("Something went wrong :(\n");
            }
        }
    }

    keepPrinting = false;
    pthread_join(threadToPrint, NULL);

    for (int i = 0; i < 100; i++) {
        free(snakes[i]);
    }

    // to free the memory used by the labyrinth
    freeLabyrinth(labyrinth);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&snake_mutex);
    pthread_mutex_destroy(&threads_mutex);
    
    printf("\nTerminado\n");
    return 0;
}