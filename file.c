#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

void freeLabyrinth(Labyrinth *labyrinth);

// to read the labyrinth from the file
Labyrinth *readLabyrinthFromFile(char *filename)
{
    FILE *file;
    int rows, cols;
    char line[100];

    // Open the file in read mode
    file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Error opening file.\n");
        return NULL;
    }

    // Read the labyrinth dimensions
     // Read the labyrinth dimensions
    if (fgets(line, sizeof(line), file) == NULL || sscanf(line, "%d %d", &rows, &cols) != 2)
    {
        printf("Error reading labyrinth dimensions.\n");
        fclose(file);
        return NULL;
    }

    // Validate dimensions
    if (rows <= 0 || cols <= 0)
    {
        printf("Error: Invalid labyrinth dimensions. Rows and columns must be positive integers.\n");
        fclose(file);
        return NULL;
    }

    // Allocate memory for the labyrinth
    Labyrinth *labyrinth = (Labyrinth *)malloc(sizeof(Labyrinth));
    labyrinth->rows = rows;
    labyrinth->cols = cols;
    labyrinth->matrix = (Cell **)malloc(rows * sizeof(Cell *));

    for (int i = 0; i < rows; i++)
    {
        labyrinth->matrix[i] = (Cell *)malloc(cols * sizeof(Cell));
    }

    // Read the labyrinth content
    int i = 0;
    while (fgets(line, sizeof(line), file) != NULL)
    {
        for (int j = 0; j < cols; j++)
        {
            switch (line[j])
            {
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
                printf("Error: Invalid character '%c' at position (%d, %d).\n", line[j], i + 1, j + 1);
                fclose(file);
                freeLabyrinth(labyrinth);
                return NULL;
            }
            labyrinth->matrix[i][j].num_checked_directions = 0;
        }
        i++;
    }

    // Close the file
    fclose(file);

    return labyrinth;
}

// to free memory space from labyrinth
void freeLabyrinth(Labyrinth *labyrinth)
{
    for (int i = 0; i < labyrinth->rows; i++)
    {
        free(labyrinth->matrix[i]);
    }
    free(labyrinth->matrix);
    free(labyrinth);
}

// to print the labyrinth (This function use cell state and the last direction added to the checked_directions array to print the correct character in each space)
void printLabyrinth(Labyrinth *labyrinth)
{
    // printf("\x1b[H");
    // printf("\x1b[J");
    printf("\n");
    for (int i = 0; i < labyrinth->rows; i++)
    {
        for (int j = 0; j < labyrinth->cols; j++)
        {
            if (labyrinth->matrix[i][j].num_checked_directions > 0)
            {
                Direction last_direction = labyrinth->matrix[i][j].checked_directions[labyrinth->matrix[i][j].num_checked_directions - 1];

                char character;
                int color_code;

                switch (last_direction)
                {
                case UP:
                    character = '^';
                    break;
                case DOWN:
                    character = 'v';
                    break;
                case LEFT:
                    character = '<';
                    break;
                default:
                    character = '>';
                    break;
                }

                if (labyrinth->matrix[i][j].state == EXIT)
                {
                    color_code = 32; // green color
                }
                else
                {
                    if (character == 'v')
                    {
                        color_code = 34; // blue color
                    }
                    else if (character == '^')
                    {
                        color_code = 36; // cian color
                    }
                    else if (character == '>')
                    {
                        color_code = 37; // gray color
                    }
                    else
                    {
                        color_code = 33; // yellow color
                    }
                }
                printf("\x1b[%d;1m %c\x1b[0m", color_code, character);
            }
            else
            {
                int color_code;
                switch (labyrinth->matrix[i][j].state)
                {
                case BLOCK:
                    color_code = 91; // Red color
                    printf("\x1b[%dm%s\x1b[0m", color_code, " *");
                    break;
                case EMPTY:
                    printf("  ");
                    break;
                case EXIT:
                    color_code = 32; // Green color
                    printf("\x1b[%dm%s\x1b[0m", color_code, " /");
                    break;
                default:
                    printf("  ");
                    break;
                }
            }
        }
        printf("\n");
    }
}