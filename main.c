#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "file.c"
#include "utilities.h"

#define THREAD_NUMBER 7

pthread_mutex_t mutex;
pthread_mutex_t snake_mutex;
pthread_mutex_t threads_mutex;

pthread_t threads[5];
pthread_t threadToPrint;
Snake *snakes[100] = { NULL };

int snakeCounter = 0;
int activeSnakeCounter = 0;

bool keepPrinting = true;
bool allStopped = false;

Labyrinth* labyrinth;

pthread_cond_t cond_searchSnake;

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
void createAdjacentThreads(Snake* snake, int new_x, int new_y);
void submitSnake(int x, int y, Direction direction);

void* startThread(void* args);

// Create and return an snake struct object
Snake createSnake(int x, int y, Direction direction) {
    Snake snake;
    snake.x = x;
    snake.y = y;
    snake.direction = direction;
    snake.checked_spaces = 0;
    snake.state = NOT_INITIALIZE;

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
    // cell->state = ALREADY_CHECKED;
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
            submitSnake(x1, new_y, LEFT);
        }
        if (right_empty && checkCellDirection(cell2, RIGHT)) {
            // Create thread (x2, y, RIGHT)
            submitSnake(x2, new_y, RIGHT);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {

        cell1 = getMatrixCell(new_x, y1);
        cell2 = getMatrixCell(new_x, y2);

        bool up_empty    = y1 >= 0 && (getCellState(cell1) != BLOCK);
        bool down_empty  = y2 < (*labyrinth).rows && (getCellState(cell2) != BLOCK);

        if (up_empty && checkCellDirection(cell1, UP)) {
            // Create thread (x, y1, UP)
            submitSnake(new_x, y1, UP);
        }
        if (down_empty && checkCellDirection(cell2, DOWN)) {
            // Create thread (x, y2, DOWN)
            submitSnake(new_x, y2, DOWN);
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
            updateCellState(cell, snake->direction);
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

void submitSnake(int x, int y, Direction direction) {
    pthread_mutex_lock(&snake_mutex);
    snakes[snakeCounter] = malloc(sizeof(Snake));
    *snakes[snakeCounter] = createSnake(x, y, direction);
    snakeCounter++;
    activeSnakeCounter++;
    pthread_mutex_unlock(&snake_mutex);
    pthread_cond_signal(&cond_searchSnake);
}

void* startThread(void* args) {
    printf("Iniciando...\n");

    while (true) {
        bool found = false;
        Snake* snake;

        pthread_mutex_lock(&snake_mutex);
        if (activeSnakeCounter == 0 && !allStopped) {
            pthread_cond_wait(&cond_searchSnake, &snake_mutex);
        }
        if (allStopped) {
            pthread_mutex_unlock(&snake_mutex);
            break;
        }
        for (int i = 0; i < snakeCounter; ++i) {
            if (snakes[i]) {
                if (snakes[i]->state == NOT_INITIALIZE) {
                    found = true;
                    snake = snakes[i];
                    snake->state = RUNNING;
                    activeSnakeCounter--;
                    break;
                }
            }
        }
        pthread_mutex_unlock(&snake_mutex);

        if (found) {
            moveSnake(snake);
        }

        pthread_mutex_lock(&snake_mutex);
        allStopped = true;
        for (int i = 0; i < snakeCounter; ++i) {
            if (snakes[i] && (snakes[i]->state == RUNNING || snakes[i]->state == NOT_INITIALIZE)) {
                allStopped = false;
                break;
            }
        }
        pthread_mutex_unlock(&snake_mutex);
    }
    pthread_mutex_lock(&snake_mutex);
    pthread_cond_broadcast(&cond_searchSnake);
    pthread_mutex_unlock(&snake_mutex);

    return NULL;
}

// const char* getSnakeStateName(SnakeState state) {
//     switch (state) {
//         case RUNNING:
//             return "RUNNING";
//         case STOPPED:
//             return "STOPPED";
//         case FINISHED:
//             return "FINISHED";
//         case NOT_INITIALIZE:
//             return "NOT INITIALIZE";
//         default:
//             return "NOT IDENTIFIED";
//     }
// }
// const char* getSnakeDirectionName(Direction direction) {
//     switch (direction) {
//         case UP:
//             return "UP";
//         case DOWN:
//             return "DOWN";
//         case LEFT:
//             return "LEFT";
//         default:
//             return "RIGHT";
//     }
// }

void* printTheLabyrinth(void* args){
    bool flag = true;

    while (keepPrinting){
        flag = false;
        sleep(1);

        // printLabyrinth(labyrinth);

        pthread_mutex_lock(&snake_mutex);
        // printf("ID\tDireccion\tEspacios Recorridos\tEstado\n");
        for (int i=0; i < snakeCounter; i++){
            if (snakes[i]){
                // printf("0\t%s\t%d\t\t%s\n", getSnakeDirectionName(snakes[i]->direction), snakes[i]->checked_spaces ,getSnakeStateName(snakes[i]->state));
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
    pthread_cond_init(&cond_searchSnake, NULL);

    submitSnake(0, 0, DOWN);

    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_create(&threads[i], NULL, &startThread, NULL);
    }
    pthread_create(&threadToPrint, NULL, printTheLabyrinth, NULL);
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_join(threads[0], NULL);
    }
    keepPrinting = false;

    // Thread to print the maze
    pthread_join(threadToPrint, NULL);

    for (int i = 0; i < snakeCounter; i++) {
        free(snakes[i]);
    }

    // to free the memory used by the labyrinth
    freeLabyrinth(labyrinth);

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&snake_mutex);
    pthread_mutex_destroy(&threads_mutex);
    pthread_cond_destroy(&cond_searchSnake);
    
    printf("\nTerminado\n");
    return 0;
}