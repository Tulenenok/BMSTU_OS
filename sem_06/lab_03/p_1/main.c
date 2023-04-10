// Поменять местами элементы верхнего и нижнего треугольника в квадратной матрице.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define INVALID_INPUT_COUNT_ROWS 2
#define INVALID_INPUT_COUNT_COLUMNS 3
#define INVALID_INPUT_MATRIX 4
#define INVALID_INPUT_SIZE_MATRIX 5

#define R_MAX 100
#define R 10
#define C_MAX 100
#define C 10

int input(size_t r, size_t c, int matrix[][C_MAX])
{
    for (size_t i = 0; i < r; i++)
        for (size_t j = 0; j < c; j++)
            if (scanf("%d", &matrix[i][j]) != 1)
                return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

void swap(int *x, int *y)
{
    int t = *x;
    *x = *y;
    *y = t;
}

void swap_upper_lower_triangle(size_t r, size_t c, int matrix[][C_MAX])
{
    for (size_t i = 0; i < r / 2; i++)
        for (size_t j = 0; j < c; j++)
            if ((i <= j) && (i + j <= r - 1))
                swap(&matrix[i][j], &matrix[r - i - 1][j]);
}

void print(size_t r, size_t c, int matrix[][C_MAX])
{
    for (size_t i = 0; i < r; i++)
    {
        for (size_t j = 0; j < c; j++)
            printf("%d ", matrix[i][j]);
        printf("\n");
    }
}

int main(void)
{
    size_t r, c;
    int matrix[R_MAX][C_MAX];

    if (scanf("%zu", &r) != 1 || r == 0 || r > R)
    {
        printf("Invalid input count of rows.");
        return INVALID_INPUT_COUNT_ROWS;
    }

    if (scanf("%zu", &c) != 1 || c == 0 || c > C)
    {
        printf("Invalid input count of rows.");
        return INVALID_INPUT_COUNT_COLUMNS;
    }

    if (r != c)
    {
        printf("Invalid input size of matrix. Count of rows and count of columns must be equal");
        return INVALID_INPUT_SIZE_MATRIX;
    }

    if (input(r, c, matrix))
    {
        printf("Invalid input matrix.");
        return INVALID_INPUT_MATRIX;
    }

    swap_upper_lower_triangle(r, c, matrix);
    print(r, c, matrix);

    sleep(100);

    return EXIT_SUCCESS;
}