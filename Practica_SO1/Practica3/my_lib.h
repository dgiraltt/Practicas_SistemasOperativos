/* @author Daniel Giralt Pascual */

/* lib.h librería con las funciones equivalentes a las
de <string.h> y las funciones y estructuras para el
manejo de una pila */

#include <stdio.h>     /* para printf en depurarión */
#include <string.h>    /* para funciones de strings  */
#include <stdlib.h>    /* Funciones malloc(), free(), y valor NULL */
#include <fcntl.h>     /* Modos de apertura de función open()*/
#include <sys/stat.h>  /* Permisos función open() */
#include <sys/types.h> /* Definiciones de tipos de datos como size_t*/
#include <unistd.h>    /* Funciones read(), write(), close()*/
#include <errno.h>     /* COntrol de errores (errno) */


//constantes
#define NUM_THREADS 10
#define N 1000000

#define EXITO 0
#define FALLO -1

#define RESET       "\x1b[0m"
#define ROJO_T      "\x1b[31m"
#define AZUL_T      "\x1b[34m"
#define NARANJA_T   "\x1B[38;2;255;128;0m"


//estructuras para gestor de pila
struct my_stack_node            // nodo de la pila (elemento)
{
    void *data;
    struct my_stack_node *next;
};

struct my_stack                 // pila
{
    int size;                   // tamaño de data, nos lo pasarán por parámetro
    struct my_stack_node *top;  // apunta al nodo de la parte superior
};  


//declaraciones funciones gestor de pila
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack);
int my_stack_write(struct my_stack *stack, char *filename);
struct my_stack *my_stack_read(char *filename);

void my_stack_print(struct my_stack *stack);
