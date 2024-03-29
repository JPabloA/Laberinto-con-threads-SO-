#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#include "file.c"
#include "utilities.h"


void moveSnake(Snake* snake, Labyrinth* labyrinth);

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
    cell->state = ALREADY_CHECKED;
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
bool isValidSnakePosition(Direction snake_direction, int x, int y, Labyrinth* labyrinth) {
    return (x >= 0 && x < labyrinth->cols &&
            y >= 0 && y < labyrinth->rows &&
            checkCellDirection(&(*labyrinth).matrix[y][x], snake_direction) &&
            !isCellBlocked(&(*labyrinth).matrix[y][x]));
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

// Create the threads for the adjacent cells (If available)
void createAdjacentThreads(Snake* snake, int new_x, int new_y, Labyrinth* labyrinth) {
    int x1 = new_x - 1;
    int x2 = new_x + 1;
    int y1 = new_y - 1;
    int y2 = new_y + 1;

    // Check if adjacent cell are empty -> Create new thread
    bool left_empty  = x1 >= 0 && !isCellBlocked( &(*labyrinth).matrix[new_y][x1] );
    bool right_empty = x2 < (*labyrinth).cols && !isCellBlocked( &(*labyrinth).matrix[new_y][x2] );
    bool up_empty    = y1 >= 0 && !isCellBlocked( &(*labyrinth).matrix[y1][new_x] );
    bool down_empty  = y2 < (*labyrinth).rows && !isCellBlocked( &(*labyrinth).matrix[y2][new_x] );

    if (snake->direction == UP || snake->direction == DOWN) {
        if (left_empty && checkCellDirection(&(*labyrinth).matrix[new_y][x1], LEFT)) {
            // Create thread (x1, y, LEFT)
            Snake snake = createSnake(x1, new_y, LEFT);
            updateCellState(&(*labyrinth).matrix[new_y][x1], snake.direction);
            moveSnake(&snake, labyrinth);
        }
        if (right_empty && checkCellDirection(&(*labyrinth).matrix[new_y][x2], RIGHT)) {
            // Create thread (x2, y, RIGHT)
            Snake snake = createSnake(x2, new_y, RIGHT);
            updateCellState(&(*labyrinth).matrix[new_y][x2], snake.direction);
            moveSnake(&snake, labyrinth);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {
        if (up_empty && checkCellDirection(&(*labyrinth).matrix[y1][new_x], UP)) {
            // Create thread (x, y1, UP)
            Snake snake = createSnake(new_x, y1, UP);
            updateCellState(&(*labyrinth).matrix[y1][new_x], snake.direction);
            moveSnake(&snake, labyrinth);
        }
        if (down_empty && checkCellDirection(&(*labyrinth).matrix[y2][new_x], DOWN)) {
            // Create thread (x, y2, DOWN)
            Snake snake = createSnake(new_x, y2, DOWN);
            updateCellState(&(*labyrinth).matrix[y2][new_x], snake.direction);
            moveSnake(&snake, labyrinth);
        }
    }
}

// Initialize the snake movement
void moveSnake(Snake* snake, Labyrinth* labyrinth) {
    // Get the current snake position
    int new_x = 0;
    int new_y = 0;

    while (true) {
        new_x = snake->x;
        new_y = snake->y;

        updateCellState(&(*labyrinth).matrix[new_y][new_x], snake->direction);
        createAdjacentThreads(snake, new_x, new_y, labyrinth);
        
        calculateNewPosition(snake->direction, &new_x, &new_y);

        // Check if the snake has left the labyrinth || cell has been traverse already
        if (!isValidSnakePosition(snake->direction, new_x, new_y, labyrinth)) {
            snake->state = STOPPED;
            return; // Terminate thread
        }
        
        // Set the new snake position
        // Update the cell with the snake direction
        snake->x = new_x;
        snake->y = new_y;
    }

}





int main() {
    // Crear un laberinto de 5 filas y 5 columnas
    // Labyrinth lab;
    // Snake snake;

    // snake.x = 7;
    // snake.y = 1;
    // snake.checked_spaces = 0;
    // snake.state = RUNNING;
    // snake.direction = DOWN;

    // printLabyrinth(&lab);


    // initializeLabyrinth(&lab, 15, 15);
    // updateCellState(&(lab.matrix[snake.y][snake.x]), snake.direction);

    // // Aquí puedes continuar trabajando con tu laberinto...
    // moveSnake(&snake, &lab);

    // printearLaberinto(&lab);


    // // Liberar memoria
    // for (int i = 0; i < lab.rows; i++) {
    //     free(lab.matrix[i]);
    // }
    // free(lab.matrix);

    Snake snake;
    Labyrinth* labyrinth;
    char filename[] = "maps/lab1 (Cerrado).txt"; // file route

    // read the labirynth from the file
    labyrinth = readLabyrinthFromFile(filename);
    if (labyrinth == NULL) {
        printf("Error reading the maze file.\n");
        return 1;
    }
    printLabyrinth(labyrinth);
    printf("\n");

    snake = createSnake(0, 0, DOWN);
    moveSnake(&snake, labyrinth);

    // to print labyrinth
    printLabyrinth(labyrinth);

    // here we can iterate over the labyrinth struct

    // ...

    // to free the memory used by the labyrinth
    freeLabyrinth(labyrinth);

    return 0;
}