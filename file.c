#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

Labyrinth* readLabyrinthFromFile(char* filename) {
    FILE* file;
    int rows, cols;
    char line[100];

    // Open the file in read mode
    file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return NULL;
    }

    // Read the labyrinth dimensions
    fgets(line, sizeof(line), file);
    sscanf(line, "%d %d", &rows, &cols);

    // Allocate memory for the labyrinth
    Labyrinth* labyrinth = (Labyrinth*)malloc(sizeof(Labyrinth));
    labyrinth->rows = rows;
    labyrinth->cols = cols;
    labyrinth->matrix = (Cell**)malloc(rows * sizeof(Cell*));

    for (int i = 0; i < rows; i++) {
        labyrinth->matrix[i] = (Cell*)malloc(cols * sizeof(Cell));
    }

    // Read the labyrinth content
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        for (int j = 0; j < cols; j++) {
            switch (line[j]) {
                case '*':
                    labyrinth->matrix[i][j].state = BLOCK;
                    break;
                case ' ':
                    labyrinth->matrix[i][j].state = EMPTY;
                    break;
                case '/':
                    labyrinth->matrix[i][j].state = EXIT;
                    break;
                default:
                    labyrinth->matrix[i][j].state = EMPTY;
                    break;
            }
            labyrinth->matrix[i][j].num_checked_directions = 0;
        }
        i++;
    }

    // Close the file
    fclose(file);

    return labyrinth;
}

void freeLabyrinth(Labyrinth* labyrinth) {
    for (int i = 0; i < labyrinth->rows; i++) {
        free(labyrinth->matrix[i]);
    }
    free(labyrinth->matrix);
    free(labyrinth);
}

void printLabyrinth(Labyrinth *labyrinth) {
    // printf("\x1b[2J");
    // Set cursor position 0, 0
    // printf("\x1b[H");

    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if ( labyrinth->matrix[i][j].num_checked_directions > 0) {
                Direction last_direction = labyrinth->matrix[i][j].checked_directions[labyrinth->matrix[i][j].num_checked_directions - 1]; // to see the last cell direction 
                switch (last_direction) {
                    case UP:
                        printf("\x1b[34;1m%c\x1b[0m",'^');
                        break;
                    case DOWN:
                        printf("\x1b[35;1m%c\x1b[0m", 'v');
                        break;
                    case LEFT:
                        printf("\x1b[33;1m%c\x1b[0m",'<');
                        break;
                    default:
                        printf(">");
                        break;
                }
                continue;
            }
            switch (labyrinth->matrix[i][j].state) {
                case BLOCK:
                    printf("\x1b[101m%c\x1b[0m", '*');
                    break;
                case EMPTY:
                    printf(" ");
                    break;
                case EXIT:
                    printf("\x1b[32m%c\x1b[0m", '/');
                    break;
                default:
                    printf(" ");
                    break;
            }
        }
        printf("\n");
    }
}
