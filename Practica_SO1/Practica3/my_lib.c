/* @author Daniel Giralt Pascual */

#include "my_lib.h"


/**
 * Reserva espacio para una variable de tipo struct my_stack.
 * 
 * @param size: Valor del tamaño de la data que guardaremos.
 * @return: Puntero a la pila, NULL de producirse un error.
 */
struct my_stack *my_stack_init(int size)
{
    struct my_stack *stack;

    if ((stack = malloc(sizeof(struct my_stack))) == NULL)
    {
        fprintf(stderr, ROJO_T"malloc: %s\n"RESET, strerror(errno));
        return NULL;
    }

    stack->size = size;
    stack->top = NULL;

    return stack;
}


/**
 * Inserta un nuevo nodo en los elementos de la pila.
 * 
 * @param stack: Pila al que le insertaremos un nuevo elemento.
 * @param data: Nuevo elemento a insertar.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int my_stack_push(struct my_stack *stack, void *data)
{
    if ((stack == NULL) || (stack->size < 0))
    {
        fprintf(stderr, "ERROR: La pila no existe o no se ha creado correctamente\n");
        return FALLO;
    }

    struct my_stack_node *node;
    if ((node = malloc(stack->size)) == NULL)
    {
        fprintf(stderr, ROJO_T"malloc: %s\n"RESET, strerror(errno));
        return FALLO;
    }

    node->next = stack->top;
    stack->top = node;
    node->data = data;

    return EXITO;
}


/**
 * Elimina el nodo superior de los elementos de la pila.
 * 
 * @param stack: Pila de la que sacaremos el último elemento.
 * @return: Data extraída.
 */
void *my_stack_pop(struct my_stack *stack)
{
    if (stack->top == NULL)
    {
        return NULL;
    }

    void *data = stack->top->data;
    stack->top = stack->top->next;
    
    return data;
}


/**
 * Recorre la pila y retorna el número de nodos totales que hay en los elementos de la pila.
 * 
 * @param stack: Pila que recorreremos.
 * @return: Tamaño de la pila, el número de nodos que tiene.
 */
int my_stack_len(struct my_stack *stack)
{
    struct my_stack_node *node = stack->top;
    unsigned int length = 0;

    while (node != NULL)
    {    
        length++;
        node = node->next;        
    }

    return length;
}


/**
 * Recorre la pila liberando la memoria que habíamos reservado para cada uno de los datos y la de cada nodo.
 * Finalmente libera también la memoria que ocupa el struct my_stack.
 * 
 * @param stack: Pila que eliminaremos.
 * @return: Bytes liberados.
 */
int my_stack_purge(struct my_stack *stack)
{
    struct my_stack_node *node;
    unsigned int stackLength = my_stack_len(stack);
    unsigned int bytes = 0;

    while (stackLength > 0)
    {
        node = stack->top;
        bytes += (sizeof(struct my_stack_node) + stack->size);
        
        stack->top = node->next;
        free(node);
        stackLength--;
    }

    bytes += sizeof(struct my_stack);
    free(stack);

    return bytes;
}


/**
 * Almacena los datos de la pila en el fichero indicado por filename.
 * 
 * @param stack: Pila que guardaremos.
 * @param filename: Nombre del fichero donde almacenaremos la pila.
 * @return: Número de elementos almacenados en el fichero.
 */
int my_stack_write(struct my_stack *stack, char *filename)
{
    if ((stack == NULL) || (stack->size < 0))
    {
        fprintf(stderr, "ERROR: La pila no existe o no se ha creado correctamente\n");
        return FALLO;
    }

    int fd, cont = 0;
    struct my_stack *stackAux;
    struct my_stack_node *node;

    if ((stackAux = my_stack_init(stack->size)) == NULL)
    {
        return FALLO;
    }
    node = stack->top;

    while (node != NULL)
    {
        if (my_stack_push(stackAux, node->data) == FALLO)
        {
            return FALLO;
        }
        node = node->next;
    }


    if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 0)
    {
        fprintf(stderr, ROJO_T"open: %s\n"RESET, strerror(errno));
        return FALLO;
    }

    if ((write(fd, &(stack->size), sizeof(stack->size))) < 0)
    {
        fprintf(stderr, ROJO_T"write: %s\n"RESET, strerror(errno));
        return FALLO;
    }

    while (stackAux->top != NULL)
    {
        if ((write(fd, my_stack_pop(stackAux), stack->size)) < 0)
        {
            fprintf(stderr, ROJO_T"write: %s\n"RESET, strerror(errno));
            return FALLO;
        }
        cont++;
    }
    

    if ((fd = close(fd)) < 0)
    {
        fprintf(stderr, ROJO_T"close: %s\n"RESET, strerror(errno));
        return FALLO;
    }

    return cont;
}


/**
 * Lee los datos de la pila almacenados en el fichero indicado por filename.
 * 
 * @param filename: Nombre del fichero del que leeremos la pila.
 * @return: Puntero a la pila leída, NULL de producirse un error.
 */
struct my_stack *my_stack_read(char *filename)
{
    int fd, aux, size;
    void *data;
    struct my_stack *stack;
    
    if ((fd = open(filename, O_RDONLY)) < 0)
    {
        //fprintf(stderr, ROJO_T"open: %s\n"RESET, strerror(errno));
        return NULL;
    }

    if ((aux = read(fd, &size, sizeof(int))) < 0)
    {
        fprintf(stderr, ROJO_T"read: %s\n"RESET, strerror(errno));
        return NULL;
    }

    if ((stack = my_stack_init(size)) == NULL)
    {
        return NULL;
    }
    

    while (aux > 0)
    {
        if ((data = malloc(size)) == NULL)
        {
            fprintf(stderr, ROJO_T"malloc: %s\n"RESET, strerror(errno));
            aux = FALLO;
        }

        if ((aux = read(fd, data, size)) < 0)
        {
            fprintf(stderr, ROJO_T"read: %s\n"RESET, strerror(errno));
            free(data);
        }
        else if (aux > 0)
        {
            if (my_stack_push(stack, data) == FALLO)
            {
                aux = FALLO;
            }
        }
    }

    if (aux < 0)
    {
        my_stack_purge(stack);
        return NULL;
    }


    if (close(fd) < 0)
    {
        fprintf(stderr, ROJO_T"close: %s\n"RESET, strerror(errno));
        return NULL;
    }

    return stack;
}


/**
 * Imprime por pantalla los datos almacenados en la pila.
 * 
 * @param stack: Pila que mostraremos por consola.
 */
void my_stack_print(struct my_stack *stack)
{
    void *data = malloc(sizeof(int));
    struct my_stack_node *node = stack->top;
    while (node != NULL)
    {
        data = node->data;
        fprintf(stderr, "%d\n", *((int*)data));
        node = node->next;
    }
}
