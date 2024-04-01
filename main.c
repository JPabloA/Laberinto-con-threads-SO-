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

pthread_t threads[5];
pthread_t threadToPrint;
pthread_cond_t cond_searchSnake;

Snake *snakes[100] = { NULL };

int snakeCounter = 0;
int activeSnakeCounter = 0;

bool allStopped = false;

Labyrinth* labyrinth;

// **********************************************************
// ***** Function Declarations                          *****
// **********************************************************

Snake createSnake(int x, int y, Direction direction, int num_spaces);
Cell* getMatrixCell(int x, int y);
CellState getCellState(Cell* cell);

bool checkCellDirection(Cell* cell, Direction direction);
bool isValidSnakePosition(Direction snake_direction, int x, int y);

const char* getSnakeStateName(SnakeState state);
const char* getSnakeDirectionName(Direction direction);

void updateCellState(Cell* cell, Direction new_direction);
void calculateNewPosition(Direction snake_direction, int* new_x, int* new_y);
void createAdjacentThreads(Snake* snake, int new_x, int new_y);
void moveSnake(Snake* snake);
void submitSnake(int x, int y, Direction direction, int num_spaces);

void* startThread(void* args);

// **********************************************************
// ***** Function Definitions                           *****
// **********************************************************

// Create and return an snake struct object
Snake createSnake(int x, int y, Direction direction, int num_spaces) {
    Snake snake;
    snake.x = x;
    snake.y = y;
    snake.direction = direction;
    snake.checked_spaces = num_spaces + 1;
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
            submitSnake(x1, new_y, LEFT, snake->checked_spaces);
        }
        if (right_empty && checkCellDirection(cell2, RIGHT)) {
            // Create thread (x2, y, RIGHT)
            submitSnake(x2, new_y, RIGHT, snake->checked_spaces);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {

        cell1 = getMatrixCell(new_x, y1);
        cell2 = getMatrixCell(new_x, y2);

        bool up_empty    = y1 >= 0 && (getCellState(cell1) != BLOCK);
        bool down_empty  = y2 < (*labyrinth).rows && (getCellState(cell2) != BLOCK);

        if (up_empty && checkCellDirection(cell1, UP)) {
            // Create thread (x, y1, UP)
            submitSnake(new_x, y1, UP, snake->checked_spaces);
        }
        if (down_empty && checkCellDirection(cell2, DOWN)) {
            // Create thread (x, y2, DOWN)
            submitSnake(new_x, y2, DOWN, snake->checked_spaces);
        }
    }
}
// Start the snake movement through the maze
void moveSnake(Snake* snake) {
    int new_x = 0;
    int new_y = 0;
    Cell* cell = NULL;

    while (true) {
        sleep(1);

        new_x = snake->x;
        new_y = snake->y;
        cell = getMatrixCell(new_x, new_y);

        if ( getCellState(cell) == EXIT ) {
            updateCellState(cell, snake->direction);
            snake->state = FINISHED;
            break;
        }
        
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
// Create and add a snake to the snakes list
void submitSnake(int x, int y, Direction direction, int num_spaces) {
    pthread_mutex_lock(&snake_mutex);
    
    snakes[snakeCounter] = malloc(sizeof(Snake));
    *snakes[snakeCounter] = createSnake(x, y, direction, num_spaces);
    (*snakes[snakeCounter]).ID = snakeCounter;
    snakeCounter++;
    activeSnakeCounter++;
    
    pthread_mutex_unlock(&snake_mutex);
    pthread_cond_signal(&cond_searchSnake);
}
// Thread Function: Loop througth the snake list to start the snake movement
void* startThread(void* args) {
    printf("Iniciando thread...\n");
    bool found = false;
    Snake* snake = NULL;

    while (true) {
        found = false;
        snake = NULL;

        pthread_mutex_lock(&snake_mutex);
        // Thread wait until signal | broadcast
        if (activeSnakeCounter == 0 && !allStopped) {
            pthread_cond_wait(&cond_searchSnake, &snake_mutex);
        }
        // Stop the while loop
        if (allStopped) {
            pthread_mutex_unlock(&snake_mutex);
            break;
        }
        // Check if one snake has not been asigned (INITIALIZED)
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
        // Start snake movement
        if (found) {
            moveSnake(snake);
        }

        pthread_mutex_lock(&snake_mutex);
        // Check if still one snake RUNNING (If not -> Finish the thread execution)
        allStopped = true;
        for (int i = 0; i < snakeCounter; ++i) {
            if (snakes[i] && (snakes[i]->state == RUNNING || snakes[i]->state == NOT_INITIALIZE)) {
                allStopped = false;
                break;
            }
        }
        pthread_mutex_unlock(&snake_mutex);
    }
    // Broadcast all thread to finish their execution
    pthread_mutex_lock(&snake_mutex);
    pthread_cond_broadcast(&cond_searchSnake);
    pthread_mutex_unlock(&snake_mutex);

    return NULL;
}
// Get the string name of the snake state
const char* getSnakeStateName(SnakeState state) {
    switch (state) {
        case RUNNING:
            return "RUNNING";
        case STOPPED:
            return "STOPPED";
        case FINISHED:
            return "FINISHED";
        case NOT_INITIALIZE:
            return "NOT INITIALIZE";
        default:
            return "NOT IDENTIFIED";
    }
}
// Get the string name of the direction
const char* getSnakeDirectionName(Direction direction) {
    switch (direction) {
        case UP:
            return "UP";
        case DOWN:
            return "DOWN";
        case LEFT:
            return "LEFT";
        default:
            return "RIGHT";
    }
}
// Thread Function: Print the maze until each snake has finished
void* printTheLabyrinth(void* args){
    bool flag = true;

    while (true){
        usleep(1000);
        printf("\x1b[H");
        printf("\x1b[J");
        // system("clear");

        flag = false;

        // Print each snake information
        pthread_mutex_lock(&snake_mutex);
        printf(" ID\tDireccion\tEspacios Recorridos\tEstado\n");
        for (int i=0; i < snakeCounter; i++){
            if (snakes[i]){
                if (snakes[i]->state == FINISHED){
                    printf(" \x1b[32m%d\t%s\t\t%d\t\t\t%s\x1b[0m\n", snakes[i]->ID, getSnakeDirectionName(snakes[i]->direction), snakes[i]->checked_spaces ,getSnakeStateName(snakes[i]->state));
                }else{
                    printf(" %d\t%s\t\t%d\t\t\t%s\n", snakes[i]->ID, getSnakeDirectionName(snakes[i]->direction), snakes[i]->checked_spaces ,getSnakeStateName(snakes[i]->state));
                }
                if (snakes[i]->state == RUNNING || snakes[i]->state == NOT_INITIALIZE)
                {
                    flag = true;
                }
            }
        }
        pthread_mutex_unlock(&snake_mutex);

        // Print the whole labyrinth
        printLabyrinth(labyrinth);

        // Stop the while loop if no snake are running
        if (!flag) {
            break;
        }
    }
    return NULL;
}
// Initialize all pthread structures
void initPthreadStructures() {
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&snake_mutex, NULL);
    pthread_cond_init(&cond_searchSnake, NULL);
}
// Destroy all pthread structures
void destroyPthreadStructures() {
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&snake_mutex);
    pthread_cond_destroy(&cond_searchSnake);
}
// Release all allocated memory
void freeAllStructures() {
    for (int i = 0; i < snakeCounter; i++) {
        free(snakes[i]);
    }
    // Free the memory used by the labyrinth
    freeLabyrinth(labyrinth);
}

int main() {
    // File route
    char filename[] = "maps/lab6.txt";

    // Read the labyrinth from the file
    labyrinth = readLabyrinthFromFile(filename);
    if (labyrinth == NULL) {
        printf("Error reading the maze file.\n");
        return 1;
    }

    // Add the initial snake to the snake list
    submitSnake(0, 0, DOWN, 0);
    // Initialize all pthread necessary structures
    initPthreadStructures();

    // Create all pthreads (Maze and printer)
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_create(&threads[i], NULL, &startThread, NULL);
    }
    pthread_create(&threadToPrint, NULL, printTheLabyrinth, NULL);    
    // Join all pthreads (Maze and printer)
    for (int i = 0; i < THREAD_NUMBER; i++) {
        pthread_join(threads[0], NULL);
    }
    pthread_join(threadToPrint, NULL);

    // Destroy all pthread structures
    destroyPthreadStructures();

    // Release all allocated memory structures
    freeAllStructures();
    
    printf("\nTerminado\n");
    return 0;
}