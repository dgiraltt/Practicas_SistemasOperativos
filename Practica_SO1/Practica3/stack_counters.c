/* @author Daniel Giralt Pascual */

#include "my_lib.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static struct my_stack *stack;
void *worker(void *ptr);


/**
 * Main del programa.
 * 
 * @param argc: Número de tokens de la línea de ejecución.
 * @param argv: Array con los diferentes tokens.
 * @return EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int main(int argc,char *argv[])
{
    if (argv[1] == NULL)
    {
        fprintf(stderr, ROJO_T"USAGE: ./stack_counters <stack_file>\n"RESET);
        return FALLO;
    }

    fprintf(stderr,"Threads: %d, Iterations: %d\n", NUM_THREADS, N);


    int *data;
    if ((stack = my_stack_read(argv[1])) == NULL)
    {
        if ((stack = my_stack_init(sizeof(int))) == NULL)
        {
            return FALLO;
        }

        fprintf(stderr, "stack->size: %d\n", stack->size);

        #if DEBUG
            fprintf(stderr, "original stack length: %d\n", my_stack_len(stack));
        #else
            fprintf(stderr, "initial stack length: %d\n", my_stack_len(stack));
            fprintf(stderr, "initial stack content:\n");
        #endif

        for (int i=0; i<NUM_THREADS; i++)
        {
            if ((data = malloc(sizeof(int))) == NULL)
            {
                fprintf(stderr, ROJO_T"malloc: %s\n"RESET, strerror(errno));
                return FALLO;
            }
            
            if (my_stack_push(stack, data) == FALLO)
            {
                return FALLO;
            }
        }
        
        #if !DEBUG
            fprintf(stderr, "stack content for treatment:\n");
            my_stack_print(stack);
        #endif
        fprintf(stderr, "new stack length: %d\n\n", my_stack_len(stack));
    }
    else if (my_stack_len(stack) < NUM_THREADS)
    {
        int items = my_stack_len(stack);
        fprintf(stderr, "initial stack length: %d\n", items);
        fprintf(stderr, "original stack content:\n");
        my_stack_print(stack);

        fprintf(stderr, "\nNumber of elements added to inital stack: %d\n", (NUM_THREADS-items));
        for (int i=0; i<(NUM_THREADS-items); i++)
        {
            if ((data = malloc(sizeof(int))) == NULL)
            {
                fprintf(stderr, ROJO_T"malloc: %s\n"RESET, strerror(errno));
                return FALLO;
            }
            
            if (my_stack_push(stack, data) == FALLO)
            {
                return FALLO;
            }
        }

        fprintf(stderr, "stack content for treatment:\n");
        my_stack_print(stack);
        fprintf(stderr, "new stack length: %d\n\n", my_stack_len(stack));
    }
    else
    {
        fprintf(stderr, "original stack:\n");
        my_stack_print(stack);
        fprintf(stderr, "original stack length: %d\n\n", my_stack_len(stack));
    }


    pthread_t threads[NUM_THREADS];
    for (int i=0; i<NUM_THREADS; i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
        #if DEBUG
            fprintf(stderr, NARANJA_T"%d) Thread %lu created\n"RESET, i, threads[i]);
        #else
            fprintf(stderr, "%d) Thread %lu created\n", i, threads[i]);
        #endif
    }
    
    for (int i=0; i<NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    #if !DEBUG
        fprintf(stderr,"\nstack content after threads iterations:\n");
        my_stack_print(stack);
        fprintf(stderr, "stack length: %d \n", my_stack_len(stack));
    #endif

    int elements;
    if ((elements = my_stack_write(stack, argv[1])) < 0)
    {
        return FALLO;
    }
    
    fprintf(stderr, "\nWritten elements from stack to file: %d\n", elements);
    fprintf(stderr, "Released bytes: %d\n", my_stack_purge(stack));
    fprintf(stderr, "Bye from main\n");
    
    pthread_exit(0);
    return EXITO;
}


/**
 * Extrae un dato de la pila, incrementa su valor en uno, y lo vuelve a insertar en la pila.
 */
void *worker(void *ptr)
{
    int *data;
    for (int i=0; i<N; i++)
    {
        pthread_mutex_lock(&mutex);
        #if DEBUG
            fprintf(stderr, AZUL_T"Soy el hilo %ld ejecutando pop\n"RESET, pthread_self());
        #endif
        data = my_stack_pop(stack);
        pthread_mutex_unlock(&mutex);

        (*data)++;

        pthread_mutex_lock(&mutex);
        #if DEBUG
            fprintf(stderr, "Soy el hilo %ld ejecutando push\n", pthread_self());
        #endif
        my_stack_push(stack, data);
        pthread_mutex_unlock(&mutex);

    }

    pthread_exit(NULL);
}
