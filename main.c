#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "utilities.h"


// Hacer la matriz de celdas (Laberinto)
// Ver la "estructura" para guardar/manejar quien recorre el laberinto
// Hacer las funciones que hagan el recorrido del laberinto 

// >>> No sé como se vaya a manejar el laberinto
// Labyrinth labyrinth;

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
            printf("Generando snake IZQUIERDA\n");
            // Create thread (x1, y, LEFT)
            Snake snake = createSnake(x1, new_y, LEFT);
            updateCellState(&(*labyrinth).matrix[new_y][x1], snake.direction);
            moveSnake(&snake, labyrinth);
        }
        if (right_empty && checkCellDirection(&(*labyrinth).matrix[new_y][x2], RIGHT)) {
            printf("Generando snake DERECHA\n");
            // Create thread (x2, y, RIGHT)
            Snake snake = createSnake(x2, new_y, RIGHT);
            updateCellState(&(*labyrinth).matrix[new_y][x2], snake.direction);
            moveSnake(&snake, labyrinth);
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {
        if (up_empty && checkCellDirection(&(*labyrinth).matrix[y1][new_x], UP)) {
            printf("Generando snake ARRIBA\n");
            // Create thread (x, y1, UP)
            Snake snake = createSnake(new_x, y1, UP);
            updateCellState(&(*labyrinth).matrix[y1][new_x], snake.direction);
            moveSnake(&snake, labyrinth);
        }
        if (down_empty && checkCellDirection(&(*labyrinth).matrix[y2][new_x], DOWN)) {
            printf("Generando snake ABAJO\n");
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
    while (true) {
        int new_x = snake->x;
        int new_y = snake->y;

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
        updateCellState(&(*labyrinth).matrix[new_y][new_x], snake->direction);

        createAdjacentThreads(snake, new_x, new_y, labyrinth);
    }

}






// Función para crear una matriz de celdas
Cell** createMatrix(int rows, int cols) {
    Cell** matrix = (Cell**)malloc(rows * sizeof(Cell*));
    if (matrix == NULL) {
        // Manejar el error si la asignación de memoria falla
        return NULL;
    }
    for (int i = 0; i < rows; i++) {
        matrix[i] = (Cell*)malloc(cols * sizeof(Cell));
        if (matrix[i] == NULL) {
            // Manejar el error si la asignación de memoria falla
            return NULL;
        }
    }
    return matrix;
}

// Función para inicializar la matriz del laberinto
void initializeLabyrinth(Labyrinth* labyrinth, int rows, int cols) {
    labyrinth->rows = rows;
    labyrinth->cols = cols;
    labyrinth->matrix = createMatrix(rows, cols);

    // Rellenar la matriz con celdas vacías
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            labyrinth->matrix[i][j].state = (
                i == 0 ||
                i == (rows - 1) ||
                j == 0 ||
                j == (cols - 1) ||
                (i == ((rows / 2) - 1) && (j != (cols / 2) && j != 1)) ||
                (i == ((rows / 2) + 1) && (j != (cols / 2) && j != 1)) ||
                (j == ((cols / 2) - 1) && (i != (rows / 2))) ||
                (j == ((cols / 2) + 1) && (i != (rows / 2))) ||
                (j == ((cols / 2) - 5) && (i != (rows / 2)))
            ) ? BLOCK : EMPTY;
            labyrinth->matrix[i][j].num_checked_directions = 0;
        }
    }
}

// Función para obtener el nombre asociado a un estado de celda
const char* cellStateToString(enum CellState state) {
    switch (state) {
        case EMPTY:
            return "EMPTY";
        case BLOCK:
            return "BLOCK";
        case ALREADY_CHECKED:
            return "ALREADY CHECKED";
        default:
            return "EXIT";
    }
}

void printearLaberinto(Labyrinth* lab) {
    for (int i = 0; i < lab->rows; i++) {
        for (int j = 0; j < lab->cols; j++) {

            if (lab->matrix[i][j].state == BLOCK) {
                printf("#");
                continue;
            }

            if (lab->matrix[i][j].num_checked_directions == 0) {
                printf(" ");
                continue;
            }

            switch (lab->matrix[i][j].checked_directions[0]) {
                case UP:
                    printf("^");
                    break;
                case DOWN:
                    printf("v");
                    break;
                case LEFT:
                    printf("<");
                    break;
                default:
                    printf(">");
                    break;
                // Puedes agregar más casos según tus necesidades
            }
        }
        printf("\n");
    }
}

int main() {
    // Crear un laberinto de 5 filas y 5 columnas
    Labyrinth lab;
    Snake snake;

    snake.x = 7;
    snake.y = 1;
    snake.checked_spaces = 0;
    snake.state = RUNNING;
    snake.direction = DOWN;

    initializeLabyrinth(&lab, 15, 15);
    updateCellState(&(lab.matrix[snake.y][snake.x]), snake.direction);

    // Aquí puedes continuar trabajando con tu laberinto...
    moveSnake(&snake, &lab);

    printearLaberinto(&lab);


    // Liberar memoria
    for (int i = 0; i < lab.rows; i++) {
        free(lab.matrix[i]);
    }
    free(lab.matrix);

    return 0;
}