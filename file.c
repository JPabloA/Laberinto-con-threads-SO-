#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *archivo;
    char nombre_archivo[] = "maps/lab2.txt";
    int filas, columnas;
    char linea[100];

    // Abrir el archivo en modo lectura
    archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return 1;
    }

    // Leer las dimensiones del laberinto
    fgets(linea, sizeof(linea), archivo);
    sscanf(linea, "%d %d", &filas, &columnas);

    // Reservar memoria para la matriz del laberinto
    char **laberinto = (char **)malloc(filas * sizeof(char *));
    for (int i = 0; i < filas; i++) {
        laberinto[i] = (char *)malloc((columnas + 1) * sizeof(char)); // +1 para el carácter nulo al final de cada línea
    }

    // Leer el contenido del laberinto
    int i = 0;
    while (fgets(linea, sizeof(linea), archivo) != NULL) {
        for (int j = 0; j < columnas; j++) {
            laberinto[i][j] = linea[j];
        }
        laberinto[i][columnas] = '\0'; // Agregar carácter nulo al final de cada línea
        i++;
    }

    // Cerrar el archivo
    fclose(archivo);

    // Imprimir el laberinto
    for (int i = 0; i < filas; i++) {
        printf("%s\n", laberinto[i]);
    }

    // Liberar la memoria utilizada por la matriz
    for (int i = 0; i < filas; i++) {
        free(laberinto[i]);
    }
    free(laberinto);

    return 0;
}