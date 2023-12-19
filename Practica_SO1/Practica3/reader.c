/* @author Daniel Giralt Pascual */

#include "my_lib.h"


/**
 * Main del programa.
 * 
 * @param argc: Número de tokens de la línea de ejecución.
 * @param argv: Array con los diferentes tokens.
 * @return EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int main(int argc, char *argv[])
{
    struct my_stack *stack;

    if (argv[1] == NULL)
    {
        fprintf(stderr, ROJO_T"USAGE: ./reader <stack_file>\n"RESET);
        return FALLO;
    }

    if ((stack = my_stack_read(argv[1])) == NULL)
    {
        fprintf(stderr, ROJO_T"Couldn't open stack file %s\n"RESET, argv[1]);
        return FALLO;
    }

    int sum = 0, min = INT_MAX, max = 0, items, *data;
    items = my_stack_len(stack);
    fprintf(stderr, "Stack length: %d\n", items);

    for (int i=0; (stack->top != NULL) && (i<NUM_THREADS); i++)
    {  
        data = my_stack_pop(stack);
        fprintf(stderr, "%d\n", *data);
        
        sum += *data;
        if (*data > max)
        {
            max = *data;
        }
        if (*data < min)
        {
            min = *data;
        }
    }

    fprintf(stderr, "\nItems: %d, Sum: %d, Min: %d, Max: %d, Average: %d\n", items, sum, min, max, (sum/NUM_THREADS));

    my_stack_purge(stack);
    return EXITO;
}
