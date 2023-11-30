/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "directorios.h"


/**
 * Main del programa.
 */
int main(int argc, char **argv)
{
    int escritos, offset, nbytes;
    char *camino, *ruta, *buffer_texto;
    
    if (argc != 5)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_escribir <nombre_dispositivo> </ruta_fichero> <texto> <offset>\n"RESET);
        return FALLO;
    }

    ruta = argv[2];
    if (ruta[strlen(ruta)-1] == '/')
    {
        fprintf(stderr,RED"ERROR: La entrada de '</ruta>' no es un fichero\n"RESET);
        return FALLO;
    }

    buffer_texto = argv[3];
    nbytes = strlen(buffer_texto);
    if (nbytes <= 0)
    {
        fprintf(stderr,RED"ERROR: La entrada de '<texto>' no es válida\n"RESET);
        return FALLO;
    }
    #if DEBUG_NIVEL9 || DEBUG_NIVEL10 || DEBUG_ENTREGA2
        fprintf(stderr, "longitud texto: %d\n", nbytes);
    #endif

    offset = atoi(argv[4]);
    if (offset < 0)
    {
        fprintf(stderr,RED"ERROR: La entrada de '<offset>' no es válida\n"RESET);
        return FALLO;
    }
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_escribir\n"RESET);
        return FALLO;
    }
    
    if ((escritos = mi_write(ruta, buffer_texto, offset, nbytes)) < 0)
    {
        mostrar_error_buscar_entrada(escritos);
        if (escritos == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_escribir\n"RESET);
            return FALLO;
        }
        escritos = 0;
    }

    #if DEBUG_NIVEL9 || DEBUG_NIVEL10 || DEBUG_ENTREGA2
        fprintf(stderr, "Bytes escritos: %d\n", escritos);
    #endif

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_escribir\n"RESET);
        return FALLO;
    }
    return EXITO;
}
