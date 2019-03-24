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

// Заполнение матрицы нулями
// matrix - указатель на матрицу
// elements_amount - количество элементов в матрице
void fill_zero(int* matrix, size_t elements_amount)
{
    for(size_t i = 0; i < elements_amount; ++i)
    {

        matrix[i] = 0;
    }
}

// Вывод матрицы на экран
// output - указатель на начало матрицы
// l_num - количество строк
// с_num - количество столбцов
// pid - момер вызывающего процесса
void print_matrix(int* output, size_t l_num, size_t c_num, int pid)
{
    for(size_t i = 0; i < l_num; ++i)
    {
        for(size_t j = 0; j < c_num; ++j)
        {
            printf("%5d ", *(output + (i * c_num) + j));
        }
        printf("\n");
    }

    printf("I`m process %d!\n\n", pid);
}

// Извлечение трёх средних столбцов из матрицы
// matrix - указатель на начало исходной матрицы
// l_num - количество строк в исходной матрице
// с_num - количество столбцов в исходной матрице
// three_columns - указатель на начало извлечённой матрицы
// needed_columns - количество столбцов в извлечённой матрице
void extract_three_columns(int* matrix, size_t l_num, size_t c_num, int* three_columns, size_t needed_columns)
{
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
}

int main (int argc, char* argv[])
{
    // Initialize MPI
    int errorCode;
    if ((errorCode = MPI_Init(&argc, &argv)) != 0)
    {
        return errorCode;
    }

    // Get MPI data
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if(world_size != 2)
    {
        printf("This program should work with 2 proceses, but has %d\n", world_size);
        return errorCode;
    }

    // Matrix
    size_t l_num = 6;
    size_t c_num = 5;
    int* matrix;

    matrix = (int*)malloc(l_num * c_num * sizeof(int));
    fill_zero(matrix, l_num * c_num);

    // Part of matrix
    size_t needed_columns = 3;
    int* three_columns;
    // Create needed columns
    three_columns = (int*)malloc(l_num * needed_columns * sizeof(int));
    
    // MPI derived type
    MPI_Datatype mpi_three_columns;
    MPI_Type_vector(l_num, needed_columns, needed_columns, MPI_INT, &mpi_three_columns);
    MPI_Type_commit(&mpi_three_columns);

    // Status for communicating
    MPI_Status status;

    if(world_rank == 0) 
    {
        // Create the matrix
        fill_with_random_values(matrix, l_num * c_num);
        
        printf("Original matrix:\n");
        print_matrix(matrix, l_num, c_num, world_rank);

        // Create the result matrix
        fill_with_random_values(matrix, l_num * c_num);
        extract_three_columns(matrix, l_num, c_num, three_columns, needed_columns);

        printf("Needed part of matrix:\n");
        print_matrix(three_columns, l_num, needed_columns, world_rank);

        // Sending data to process 1 (with derived datatype)
        MPI_Send(three_columns, 1, mpi_three_columns, 1, 0, MPI_COMM_WORLD);

        // Sending data to process 1 again
        MPI_Send(three_columns, 1, mpi_three_columns, 1, 1, MPI_COMM_WORLD);

        int buff[l_num * needed_columns];
        fill_zero(buff, l_num * needed_columns);

        MPI_Recv(buff, 1, mpi_three_columns, 1, 2, MPI_COMM_WORLD, &status);
        printf("Geting data from process 1 (stored via derived datatype):\n");
        print_matrix(buff, l_num, needed_columns, world_rank);
        printf("____________________________________________________\n");
    }

    else 
    {
        printf("Needed part of matrix:\n");
        fill_zero(three_columns, needed_columns * l_num);
        print_matrix(three_columns, l_num, needed_columns, world_rank);

        printf("Getting data from process 0 (stored via derived datatype):\n");
        MPI_Recv(three_columns, 1, mpi_three_columns, 0, 0, MPI_COMM_WORLD, &status);
        print_matrix(three_columns, l_num, needed_columns, world_rank);
        printf("____________________________________________________\n");

        int buff[l_num * needed_columns];
        fill_zero(buff, l_num * needed_columns);

        MPI_Recv(buff, l_num * needed_columns, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        printf("Geting data from process 0 (stored via base datatype):\n");
        print_matrix(buff, l_num, needed_columns, world_rank);
        printf("____________________________________________________\n");

        // Sending data to process 0
        MPI_Send(three_columns, needed_columns * l_num, MPI_INT, 0, 2, MPI_COMM_WORLD);
    }

    free(three_columns);
    free(matrix);
    MPI_Type_free(&mpi_three_columns);

    MPI_Finalize();
    return 0;
}
