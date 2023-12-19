/* @author Daniel Giralt Pascual */

//Librerías
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <limits.h>


//Constantes
#define NUM_THREADS 10
#define N 1000000

#define EXITO 0
#define FALLO -1

#define DEBUG 0

#define RESET       "\x1b[0m"
#define ROJO_T      "\x1b[31m"
#define AZUL_T      "\x1b[34m"
#define NARANJA_T   "\x1B[38;2;255;128;0m"


//Estructuras
struct my_stack_node
{
    void *data;
    struct my_stack_node *next;
};

struct my_stack
{
    int size;
    struct my_stack_node *top;
};


//Funciones
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack);
int my_stack_write(struct my_stack *stack, char *filename);
struct my_stack *my_stack_read(char *filename);

void my_stack_print(struct my_stack *stack);
