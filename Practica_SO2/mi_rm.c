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
    char *camino, *ruta;
    int error;
    
    if (argc != 3)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_rm <disco> </ruta_fichero>\n"RESET);
        return FALLO;
    }

    ruta = argv[2];
    if (ruta[strlen(ruta)-1] == '/')
    {
        fprintf(stderr,RED"ERROR: La entrada de '</ruta_fichero>' no es un fichero\n"RESET);
        return FALLO;
    }

    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_rm\n"RESET);
        return FALLO;
    }

    if ((error = mi_unlink(ruta)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_rm\n"RESET);
            return FALLO;
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_rm\n"RESET);
        return FALLO;
    }
    return EXITO;
}
