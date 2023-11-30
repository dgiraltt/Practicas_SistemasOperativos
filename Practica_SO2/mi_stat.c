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
    struct STAT p_stat;
    char *camino, *ruta;
    int error;
    
    if (argc != 3)
    {
        fprintf(stderr, RED"Sintaxis: ./mi_stat <disco> </ruta>\n"RESET);
        return FALLO;
    }
    ruta = argv[2];
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_stat\n"RESET);
        return FALLO;
    }

    if ((error = mi_stat(ruta, &p_stat)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_stat\n"RESET);
            return FALLO;
        }
    }


    fprintf(stderr, "Nº de inodo: %d\n", error);
    fprintf(stderr, "tipo: %c\n", p_stat.tipo);
    fprintf(stderr, "permisos: %d\n", p_stat.permisos);
    
    char atime[80]; char ctime[80]; char mtime[80];
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.atime));
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.ctime));
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.mtime));
    
    fprintf(stderr, "atime: %s\n", atime);
    fprintf(stderr, "ctime: %s\n", ctime);
    fprintf(stderr, "mtime: %s\n", mtime);

    fprintf(stderr, "nlinks: %d\n", p_stat.nlinks);
    fprintf(stderr, "tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
    fprintf(stderr, "numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);


    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_stat\n"RESET);
        return FALLO;
    }
    return EXITO;
}
