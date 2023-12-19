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


//constantes
#define EXITO 0
#define FALLO -1

#define RESET       "\033[0m"
#define ROJO_T      "\x1b[31m"


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


//Funciones String
size_t my_strlen(const char *str);
int my_strcmp(const char *str1, const char *str2);
char *my_strcpy(char *dest, const char *src);
char *my_strncpy(char *dest, const char *src, size_t n);
char *my_strcat(char *dest, const char *src);
char *my_strchr(const char *s, int c);


//Funciones Pila
struct my_stack *my_stack_init(int size);
int my_stack_push(struct my_stack *stack, void *data);
void *my_stack_pop(struct my_stack *stack);
int my_stack_len(struct my_stack *stack);
int my_stack_purge(struct my_stack *stack);
int my_stack_write(struct my_stack *stack, char *filename);
struct my_stack *my_stack_read(char *filename);
