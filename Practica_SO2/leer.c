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
    struct STAT p_stat;
    int ninodo, leidos, total_leidos = 0, offset = 0, tambuffer = BLOCKSIZE;
    char *camino, buffer_texto[tambuffer];
    
    if (argc != 3)
    {
        fprintf(stderr, RED"Sintaxis: ./leer <nombre_dispositivo> <ninodo>\n"RESET);
        return FALLO;
    }

    ninodo = atoi(argv[2]);
    if (ninodo<0)
    {
        fprintf(stderr, RED"ERROR: La entrada de 'ninodo' no es válida\n"RESET);
        return FALLO;
    }
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer\n"RESET);
        return FALLO;
    }
    
    memset(buffer_texto, 0, tambuffer);
    leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
    if (leidos == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer\n"RESET);
        return FALLO;
    }

    while (leidos > 0)
    {
        write(1, buffer_texto, leidos);
        total_leidos += leidos;
        offset += tambuffer;

        memset(buffer_texto, 0, tambuffer);
        leidos = mi_read_f(ninodo, buffer_texto, offset, tambuffer);
        if (leidos == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./leer\n"RESET);
            return FALLO;
        }
    }
    fprintf(stderr, "\n");

    if (mi_stat_f(ninodo, &p_stat) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer\n"RESET);
        return FALLO;
    }

    #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
        fprintf(stderr, "total_leidos: %d\n", total_leidos);
        fprintf(stderr, "tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    #endif

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer\n"RESET);
        return FALLO;
    }
    return EXITO;
}
