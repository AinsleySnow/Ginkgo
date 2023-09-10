#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

int main(void)
{
    printf("Hello world!\n");
    printf("%d %d %d %d %d\n", 1, 2, 3, 4, 5);

    int a = 0;
    scanf("%d", &a);
    printf("%d\n", a);

    int* array = malloc(sizeof(int) * 100);
    int sum = 0;
    for (int i = 0; i < 100; ++i)
        array[i] = i + 1;
    for (int i = 0; i < 100; ++i)
        sum += array[i];
    printf("%d\n", sum);

    free(array);

    array = calloc(sizeof(int), 100);
    int sum = 0;
    for (int i = 0; i < 100; ++i)
        array[i] = i + 1;
    for (int i = 0; i < 100; ++i)
        sum += array[i];
    printf("%d\n", sum);

    free(array);

    fprintf(stdout, "Hello world!\n");
    fprintf(stderr, "Hello world!\n");
    return 0;
}
