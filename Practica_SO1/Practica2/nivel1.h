/* @author Daniel Giralt Pascual */

#define _POSIX_C_SOURCE 200112L

//Librerías
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


//Constantes
#define ARGS_SIZE 64
#define COMMAND_LINE_SIZE 1024

#define EXITO 0
#define FALLO -1

#define DEBUGN1 1

#define RESET       "\033[0m"
#define NEGRO_T     "\x1b[30m"
#define NEGRO_F     "\x1b[40m"
#define GRIS_T      "\x1B[38;2;176;174;174m"
#define ROJO_T      "\x1b[31m"
#define VERDE_T     "\x1b[32m"
#define AMARILLO_T  "\x1b[33m"
#define AZUL_T      "\x1b[34m"
#define MAGENTA_T   "\x1b[35m"
#define ROSA_T      "\x1B[38;2;255;70;150m"
#define CYAN_T      "\x1b[36m"
#define BLANCO_T    "\x1b[97m"
#define NEGRITA     "\x1b[1m"

const char PROMPT = '$';


//Funciones
char *read_line(char *line);
int execute_line(char *line);
int parse_args(char **args, char *line);
int check_internal(char **args);
int internal_cd(char **args); 
int internal_export(char **args); 
int internal_source(char **args); 
int internal_jobs(char **args); 
int internal_fg(char **args);
int internal_bg(char **args); 
