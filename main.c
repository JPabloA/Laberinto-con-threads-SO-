#include <stdbool.h>
#include <pthread.h>
#include "utilities.h"


// Hacer la matriz de celdas (Laberinto)
// Ver la "estructura" para guardar/manejar quien recorre el laberinto
// Hacer las funciones que hagan el recorrido del laberinto 

// >>> No sé como se vaya a manejar el laberinto
Labyrinth labyrinth;

// Check if an specific cell is available
bool isCellAvailable(CellState state) {
    return state == EMPTY || state == ALREADY_CHECKED;
}

bool isCellDirectionAvailable(Cell* cell, Direction direction) {
    int max_num = cell->num_checked_directions;

    for (int i = 0; i < max_num; ++i) {
        if (cell->checked_directions[i] == direction) {
            return false;
        }
    }
    return true;
}

void updateCellState(Cell* cell, Direction new_direction) {
    if (isCellDirectionAvailable(cell, new_direction)) {
        return;
    }

    cell->checked_directions[ cell->num_checked_directions ] = new_direction;
    cell->num_checked_directions++;
}

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

    // Check if the snake has left the labyrinth || cell has been traverse already
    if (new_x < 0 || new_x > labyrinth.cols || new_y < 0 || new_y > labyrinth.rows || !isCellDirectionAvailable(&labyrinth.matrix[new_y][new_x], snake->direction)) {
        snake->state = STOPPED;
        return; // Terminate thread
    }

    // Set the new snake position
    snake->x = new_x;
    snake->y = new_y;
    // Update the cell with the snake direction
    updateCellState(&labyrinth.matrix[new_y][new_x], snake->direction);

    // Check if adyacent cell are empty -> Create new thread
    int x1 = new_x - 1;
    int x2 = new_x + 1;
    int y1 = new_y - 1;
    int y2 = new_y + 1;

    bool left_empty  = x1 >= 0 && isCellAvailable(labyrinth.matrix[new_y][x1].state);
    bool right_empty = x2 < labyrinth.cols && isCellAvailable(labyrinth.matrix[new_y][x2].state);
    bool up_empty    = y1 >= 0 && isCellAvailable(labyrinth.matrix[y1][new_x].state);
    bool down_empty  = y2 < labyrinth.rows && isCellAvailable(labyrinth.matrix[y2][new_x].state);

    if (snake->direction == UP || snake->direction == DOWN) {
        if (left_empty && isCellDirectionAvailable(&labyrinth.matrix[new_y][x1], LEFT)) {
            // Create thread (x1, y, LEFT)
        }
        if (right_empty && isCellDirectionAvailable(&labyrinth.matrix[new_y][x2], RIGHT)) {
            // Create thread (x2, y, RIGHT)
        }
    }
    else if (snake->direction == RIGHT || snake->direction == LEFT) {
        if (up_empty && isCellDirectionAvailable(&labyrinth.matrix[y1][new_x], UP)) {
            // Create thread (x, y1, UP)
        }
        if (down_empty && isCellDirectionAvailable(&labyrinth.matrix[y2][new_x], DOWN)) {
            // Create thread (x, y2, DOWN)
        }
    }
}

int main() {
    /* code */
    return 0;
}
