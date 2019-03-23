#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

// Заполнение матрицы случайными числами
// matrix - указатель на матрицу
// elements_amount - количество элементов в матрице
void fill_with_random_values(int* matrix, size_t elements_amount)
{
    srand(time(NULL));

    for(size_t i = 0; i < elements_amount; ++i)
    {

        matrix[i] = rand() % 10;
    }
}

// Вывод матрицы на экран
// output - указатель на начало матрицы
// l_num - количество строк
// с_num - количество столбцов
void print_matrix(int* output, size_t l_num, size_t c_num)
{
    for(size_t i = 0; i < l_num; ++i)
    {
        for(size_t j = 0; j < c_num; ++j)
        {
            printf("%5d ", *(output + (i * c_num) + j));
        }
        printf("\n");
    }
}

// Сложение двух соседних строк матрицы
// input_lines - указатель на первый элемент
// c_num - количество столбцов
void add_adjacent_lines(int* input_lines, size_t c_num, int* result)
{
    print_matrix(input_lines, 1, c_num);
    for(size_t i = 0; i < c_num; ++i)
    {
        result[i] = input_lines[i] + input_lines[i + c_num];
    }

    //printf("Result:\n");
    //print_matrix(result, 1, c_num);
}

int main (int argc, char* argv[])
{
    // Work with MPI
    int errorCode;
    if ((errorCode = MPI_Init(&argc, &argv)) != 0)
    {
        return errorCode;
    }

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Matrix
    size_t l_num;
    size_t c_num = 5;
    int* matrix;

    // Part of matrix
    size_t needed_columns = 3;
    int* three_columns;

    // Initialize matrix only in main process
    if(world_rank == 0)
    {
        printf("Enter an number of lines!\n");
        scanf("%zu", &l_num);

        printf("l_num = %zu\n", l_num);

        //TODO: Создать ограничение на минимальное количество процессов.
        // Если процессов меньше необходимого - аварийно завершать программу.
        // Если сильно больше - тоже как-то прервать.

        // Create the matrix
        matrix = (int*)malloc(l_num * c_num * sizeof(int));
        fill_with_random_values(matrix, l_num * c_num);
        
        printf("Matrix:\n");
        print_matrix(matrix, l_num, c_num);

        // Create needed columns
        three_columns = (int*)malloc(l_num * needed_columns * sizeof(int));

        printf("\n");

        size_t j  = 0;
        for(size_t line_counter = 1; line_counter < l_num * c_num; line_counter += 5)
        {

                for(size_t i = 0; i < needed_columns; ++i)
                {
                    // Set value to three_columns element
                    *(three_columns + i + j) = *(matrix + i + line_counter);
                }

                // Switch to next line of three_columns
                j += 3;
        }

        printf("Needed part of matrix:\n");
        print_matrix(three_columns, l_num, needed_columns);
    }

    // For each process, create a buffer that will hold two lines of entire matrix
    // TODO change 4 to c_num (complicated, in workers process have no c_num)
    int two_lines_size = 2 * c_num;
    int* two_lines = (int*)malloc(two_lines_size * sizeof(int));

    // Result line
    int* added_lines = (int*)malloc(c_num * sizeof(int));

    if(world_rank == 0)
    {
        printf("Result matrix:\n");

        free(matrix);
        free(three_columns);
    }
    free(two_lines);
    free(added_lines);

    MPI_Finalize();
    return 0;
}
