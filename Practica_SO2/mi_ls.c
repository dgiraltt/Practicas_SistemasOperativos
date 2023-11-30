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
    char *camino, *ruta, buffer[TAMBUFFER], tipo;
    int numEntradas, flag = 0;
    
    if ((argc<3) || (argc>4))
    {
        fprintf(stderr, RED"Sintaxis: ./mi_ls <'-l' opcional> <disco> </ruta>\n"RESET);
        return FALLO;
    }

    if (argc == 4)
    {
        if (strcmp(argv[1], "-l"))
        {
            fprintf(stderr, RED"ERROR: el flag indicado no és correcto.\n" "Flags admitidos: <-l>\n"RESET);
            return FALLO;
        }
        else
        {
            flag = 1;
        }
    }

    
    ruta = argv[2+flag];
    if (ruta[strlen(ruta)-1] == '/')
    {
        tipo = 'd';
    }
    else
    {
        tipo = 'f';
    }
    
    camino = argv[1+flag];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_ls\n"RESET);
        return FALLO;
    }

    memset(buffer, 0, sizeof(buffer));
    if ((numEntradas = mi_dir(ruta, buffer, tipo, flag)) < 0)
    {
        mostrar_error_buscar_entrada(numEntradas);
        if (numEntradas == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_ls\n"RESET);
            return FALLO;
        }
    }

    if (numEntradas >= 0)
    {
        if (tipo == 'd')
        {
            fprintf(stderr, "Total: %d\n", numEntradas);
        }
        
        if (flag == 1)
        {
            fprintf(stderr, "Tipo\tPermisos\tmTime\t\t\t\tTamaño\t\tNombre\n");
            fprintf(stderr, "----------------------------------------------------------------------------------------------------\n");
        }
        
        fprintf(stderr, "%s\n", buffer);
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_ls\n"RESET);
        return FALLO;
    }
    return EXITO;
}
