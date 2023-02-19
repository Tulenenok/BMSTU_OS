#include <stdio.h>
#include <stdlib.h>

#define __USE_MINGW_ANSI_STDIO 1

#define INVALID_INPUT_SIZE 2
#define INVALID_INPUT_ARRAY 3
#define NO_ODD 4

#define N_MAX 1000
#define N 10

int input(size_t n, int arr[])
{
	printf("Input array (len %d): ", n);
    for (size_t i = 0; i < n; i++)
        if (scanf("%d", &arr[i]) != 1)
            return EXIT_FAILURE;
    return EXIT_SUCCESS;
}

int product_odd(size_t n, int arr[], int *prod)
{
    int count_odd = 0;
    for (size_t i = 0; i < n; i++)
        if (arr[i] % 2 != 0)
        {
            count_odd++;
            (*prod) *= arr[i];
        }
    return count_odd;
}

int main(void)
{
    size_t n;
    int arr[N_MAX];
    int prod = 1;

	printf("Input size of array: ");
    if (scanf("%zu", &n) != 1 || n == 0 || n > N)
    {
        printf("Invalid input size of array");
        return INVALID_INPUT_SIZE;
    }

    if (input(n, arr))
    {
        printf("Invalid input array");
        return INVALID_INPUT_ARRAY;
    }

    if (product_odd(n, arr, &prod) == 0)
    {
        printf("No odd elements in the array");
        return NO_ODD;
    }

    printf("Product of odd elements = %d", prod);

    return EXIT_SUCCESS;
}
