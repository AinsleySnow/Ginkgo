#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>

int permute(int* arr, int begin, int end)
{
    int mid = arr[begin];
    int i = begin;
    for (int j = i + 1; j < end; ++j)
    {
        if (arr[j] <= mid)
        {
            i += 1;
            int temp = arr[j];
            arr[j] = arr[i];
            arr[i] = temp;
        }
    }

    int temp = arr[begin];
    arr[begin] = arr[i];
    arr[i] = temp;
    return i;
}

void quick_sort(int* arr, int begin, int end)
{
    if (begin >= end)
        return;
    int axle = permute(arr, begin, end);
    quick_sort(arr, begin, axle);
    quick_sort(arr, axle + 1, end);
}

int main(void)
{
    srand(time(NULL));
    int array[100];
    for (int i = 0; i < 100; ++i)
        array[i] = rand() % 1000 + 1;

    quick_sort(array, 0, 100);
    for (int i = 0; i < 99; ++i)
        assert(array[i] <= array[i + 1]);

    printf("OK\n");
    return 0;
}
