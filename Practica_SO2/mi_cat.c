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
    int leidos, total_leidos = 0, offset = 0;
    char *camino, *ruta, buffer_texto[TAMBUFFERLECTURA];
    
    if (argc != 3)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_cat <nombre_dispositivo> </ruta_fichero>\n"RESET);
        return FALLO;
    }

    ruta = argv[2];
    if (ruta[strlen(ruta)-1] == '/')
    {
        fprintf(stderr,RED"ERROR: La entrada de '</ruta>' no es un fichero\n"RESET);
        return FALLO;
    }
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        return FALLO;
    }
    
    
    memset(buffer_texto, 0, TAMBUFFERLECTURA);
    if ((leidos = mi_read(ruta, buffer_texto, offset, TAMBUFFERLECTURA)) < 0)
    {
        mostrar_error_buscar_entrada(leidos);
        if (leidos == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
            return FALLO;
        }
    }

    while (leidos > 0)
    {
        write(1, buffer_texto, leidos);
        total_leidos += leidos;
        offset += TAMBUFFERLECTURA;

        memset(buffer_texto, 0, TAMBUFFERLECTURA);
        if ((leidos = mi_read(ruta, buffer_texto, offset, TAMBUFFERLECTURA)) < 0)
        {
            mostrar_error_buscar_entrada(leidos);
            if (leidos == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
                return FALLO;
            }
        }
    }
    fprintf(stderr, "\n");


    #if DEBUG_NIVEL9 || DEBUG_NIVEL10 || DEBUG_ENTREGA2 || DEBUG_ENTREGA3
        fprintf(stderr, "Total_leidos: %d\n", total_leidos);
    #endif

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        return FALLO;
    }
    return EXITO;
}
