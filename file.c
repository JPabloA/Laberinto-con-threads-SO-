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
    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            switch (labyrinth->matrix[i][j].state) {
                case BLOCK:
                    printf("*");
                    break;
                case EMPTY:
                    printf(" ");
                    break;
                case EXIT:
                    printf("/");
                    break;
                default:
                    printf(" ");
                    break;
            }
        }
        printf("\n");
    }
}

int main() {
    Labyrinth* labyrinth;
    char filename[] = "maps/lab2.txt"; // file route

    // read the labirynth from the file
    labyrinth = readLabyrinthFromFile(filename);
    if (labyrinth == NULL) {
        printf("Error reading the maze file.\n");
        return 1;
    }

    // to print labyrinth
    printLabyrinth(labyrinth);

    // here we can iterate over the labyrinth struct

    // ...

    // to free the memory used by the labyrinth
    freeLabyrinth(labyrinth);

    return 0;
}