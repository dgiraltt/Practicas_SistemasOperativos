/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "ficheros.h"


/**
 * Main del programa.
 */
int main(int argc, char **argv)
{
    int ninodo, permisos;
    char *camino;

    if (argc != 4)
    {
        fprintf(stderr,RED"Sintaxis: ./permitir <nombre_dispositivo> <ninodo> <permisos>\n"RESET);
        return FALLO;
    }

    ninodo = atoi(argv[2]);
    if (ninodo<0)
    {
        fprintf(stderr, RED"ERROR: La entrada de 'ninodo' no es válida\n"RESET);
        return FALLO;
    }

    permisos = atoi(argv[3]);
    if (permisos<0 || permisos>7)
    {
        fprintf(stderr,RED"ERROR: La entrada de 'permisos' no es válida (0-7)\n"RESET);
        return FALLO;
    }
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr,RED"ERROR EN ./permitir\n"RESET);
        return FALLO;
    }

    if (mi_chmod_f(ninodo, permisos) == FALLO)
    {
        fprintf(stderr,RED"ERROR EN ./permitir\n"RESET);
        return FALLO;
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr,RED"ERROR EN ./permitir\n"RESET);
        return FALLO;
    }
    return EXITO;
}