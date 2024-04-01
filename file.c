#include <stdio.h>
#include <stdlib.h>
#include "utilities.h"

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
    fgets(line, sizeof(line), file);
    sscanf(line, "%d %d", &rows, &cols);

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

void freeLabyrinth(Labyrinth *labyrinth)
{
    for (int i = 0; i < labyrinth->rows; i++)
    {
        free(labyrinth->matrix[i]);
    }
    free(labyrinth->matrix);
    free(labyrinth);
}

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