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
    char *camino, *ruta_fichero_original, *ruta_enlace;
    int error;
    
    if (argc != 4)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_link <disco> </ruta_fichero_original> </ruta_enlace>\n"RESET);
        return FALLO;
    }

    ruta_fichero_original = argv[2];
    if (ruta_fichero_original[strlen(ruta_fichero_original)-1] == '/')
    {
        fprintf(stderr, RED"ERROR: La entrada de '</ruta>' no es un fichero\n"RESET);
        return FALLO;
    }

    ruta_enlace = argv[3];
    if (ruta_enlace[strlen(ruta_enlace)-1] == '/')
    {
        fprintf(stderr, RED"ERROR: La entrada de '</ruta>' no es un fichero\n"RESET);
        return FALLO;
    }

    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_link\n"RESET);
        return FALLO;
    }

    if ((error = mi_link(ruta_fichero_original, ruta_enlace)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_link\n"RESET);
            return FALLO;
        }
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_link\n"RESET);
        return FALLO;
    }
    return EXITO;
}
