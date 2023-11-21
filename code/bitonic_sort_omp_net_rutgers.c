// https://people.cs.rutgers.edu/~venugopa/parallel_summer2012/bitonic_openmp.html#:~:text=Bitonic%20sort%20is%20one%20of,and%20other%20being%20bitonic%20merge.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <omp.h>


#define MAX(A, B) (((A) > (B)) ? (A) : (B))
#define MIN(A, B) (((A) > (B)) ? (B) : (A))
#define UP 0
#define DOWN 1


int is_power_of_two(int num);
int* generate_random_array(int size, int mod_factor);
void print_sequence(int* sequence, int sequence_size, int first_n, int last_n);
void bitonic_sort_seq(int start, int length, int *seq, int flag);
void bitonic_sort_par(int start, int length, int *seq, int flag);
void swap(int *a, int *b);


int m;


int main(int argc, char *argv[])  // Arg1: Número de threads; Arg2: Tamanho da sequência
{
    int i, j;
    int n;
    int flag;
    int *seq;
    int numThreads;
    double startTime, finishTime, elapsedTime;

    n = atoi(argv[2]);
    seq = generate_random_array(n, n);

    // start
    startTime = omp_get_wtime();

    numThreads = atoi(argv[1]);
    omp_set_num_threads(numThreads);

    printf("Número de threads %d\n", numThreads);

    // making sure input is okay
    if ( n < numThreads * 2 )
    {
        printf("Erro: O tamanho do vetor é menor que 2 * o número de processos. Abortando...\n");
        exit(1);
    }
    if (!is_power_of_two(n))
    {
        printf("Erro: o tamanho do vetor não é uma potência de 2. Abortando...\n");
        exit(1);
    }

    // the size of sub part
    m = n / numThreads;

    // make the sequence bitonic - part 1
    for (i = 2; i <= m; i = 2 * i)
    {
#pragma omp parallel for shared(i, seq) private(j, flag)
        for (j = 0; j < n; j += i)
        {
            if ((j / i) % 2 == 0)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_seq(j, i, seq, flag);
        }
    }

    // make the sequence bitonic - part 2
    for (i = 2; i <= numThreads; i = 2 * i)
    {
        for (j = 0; j < numThreads; j += i)
        {
            if ((j / i) % 2 == 0)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_par(j*m, i*m, seq, flag);
        }
#pragma omp parallel for shared(j)
        for (j = 0; j < numThreads; j++)
        {
            if (j < i)
                flag = UP;
            else
                flag = DOWN;
            bitonic_sort_seq(j*m, m, seq, flag);
        }
    }

    // bitonic sort
    //bitonic_sort_par(0, n, seq, UP);
    //bitonic_sort_seq(0, n, seq, UP);

    //end
    finishTime = omp_get_wtime();
    elapsedTime = finishTime - startTime;

    
    print_sequence(seq, n, 10, 10);
    
    printf("Começo: %f seg\n", startTime);
    printf("Fim: %f seg\n", finishTime);
    printf("Tempo decorrido: %f seg\n", elapsedTime);

    free(seq);

    return 0;
}


int is_power_of_two(int num)
{
    // Verifica se o número é positivo e se tem apenas um bit definido
    return (num > 0) && ((num & (num - 1)) == 0);
}


int* generate_random_array(int size, int mod_factor) 
{
    int *array = (int *) malloc(size * sizeof(int));

    // Seed the random number generator with the current time
    srand(time(NULL));

    // Generate n random integers and store them in the vector
    for (int i = 0; i < size; i++) {
        array[i] = rand() % mod_factor;
    }

    return array;
}


void print_sequence(int* sequence, int sequence_size, int first_n, int last_n)
{
    printf("[");

    if (first_n != 0)
    {
        for (int i = 0; i < first_n - 1; i++)
        {
            printf("%d, ", sequence[i]);
        }
        printf("%d", sequence[first_n - 1]);
    }
    
    if (first_n != sequence_size && last_n != sequence_size)
    {
        printf(", ... , ");
    }

    if (last_n != 0)
    {
        for (int i = sequence_size - last_n; i < sequence_size - 1; i++)
        {
            printf("%d, ", sequence[i]);
        }
        printf("%d", sequence[sequence_size-1]);
    }
    
    printf("]\n");
}


void bitonic_sort_seq(int start, int length, int *seq, int flag)
{
    int i;
    int split_length;

    if (length == 1)
        return;

    if (length % 2 !=0 )
    {
        printf("error\n");
        exit(0);
    }

    split_length = length / 2;

    // bitonic split
    for (i = start; i < start + split_length; i++)
    {
        if (flag == UP)
        {
            if (seq[i] > seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
        else
        {
            if (seq[i] < seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
    }

    bitonic_sort_seq(start, split_length, seq, flag);
    bitonic_sort_seq(start + split_length, split_length, seq, flag);
}


void bitonic_sort_par(int start, int length, int *seq, int flag)
{
    int i;
    int split_length;

    if (length == 1)
        return;

    if (length % 2 !=0 )
    {
        printf("The length of a (sub)sequence is not divided by 2.\n");
        exit(0);
    }

    split_length = length / 2;


// bitonic split
#pragma omp parallel for shared(seq, flag, start, split_length) private(i)
    for (i = start; i < start + split_length; i++)
    {
        if (flag == UP)
        {
            if (seq[i] > seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
        else
        {
            if (seq[i] < seq[i + split_length])
                swap(&seq[i], &seq[i + split_length]);
        }
    }

    if (split_length > m)
    {
        // m is the size of sub part-> n/numThreads
        bitonic_sort_par(start, split_length, seq, flag);
        bitonic_sort_par(start + split_length, split_length, seq, flag);
    }

    return;
}


void swap(int *a, int *b)
{
    int t;
    t = *a;
    *a = *b;
    *b = t;
}
