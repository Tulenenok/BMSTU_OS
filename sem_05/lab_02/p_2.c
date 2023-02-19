// Заполнить матрицу по спирали по часовой стрелке.

#include <stdio.h>
#include <stdlib.h>

#define __USE_MINGW_ANSI_STDIO 1

#define INVALID_INPUT_COUNT_ROWS 2
#define INVALID_INPUT_COUNT_COLUMNS 3
#define INVALID_INPUT_MATRIX 4
#define INVALID_INPUT_SIZE_MATRIX 5

#define R_MAX 100
#define R 10
#define C_MAX 100
#define C 10

void fill_one_circle(int *last_num, size_t circle_number, size_t r, size_t c, int matrix[][C_MAX])
{
    for (size_t j = circle_number; j < c - circle_number; j++)
    {
        (*last_num)++;
        matrix[circle_number][j] = *last_num;
    }

    for (size_t i = circle_number + 1; i < r - circle_number; i++)
    {
        (*last_num)++;
        matrix[i][c - circle_number - 1] = *last_num;
    }

    for (size_t j = c - circle_number - 1; j > circle_number; j--)
    {
        (*last_num)++;
        matrix[c - circle_number - 1][j - 1] = *last_num;
    }

    for (size_t i = r - circle_number - 1; i > circle_number + 1; i--)
    {
        (*last_num)++;
        matrix[i - 1][circle_number] = *last_num;
    }
}

void fill(size_t r, size_t c, int matrix[][C_MAX])
{
    int last_num = 0;
    for (size_t i = 0; i <= r / 2; i++)
        fill_one_circle(&last_num, i, r, c, matrix);
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

	printf("Input count of rows: ");
    if (scanf("%zu", &r) != 1 || r == 0 || r > R)
    {
        printf("Invalid input count of rows.");
        return INVALID_INPUT_COUNT_ROWS;
    }
	
	printf("Input count of columns: ");
    if (scanf("%zu", &c) != 1 || c == 0 || c > C)
    {
        printf("Invalid input count of colums.");
        return INVALID_INPUT_COUNT_COLUMNS;
    }

    if (r != c)
    {
        printf("Invalid input size of matrix. Count of rows and count of columns must be equal");
        return INVALID_INPUT_SIZE_MATRIX;
    }

    fill(r, c, matrix);

    print(r, c, matrix);

    return EXIT_SUCCESS;
}
