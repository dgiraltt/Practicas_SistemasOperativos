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
    int ninodo, nbytes;
    char *camino;

    if (argc != 4)
    {
        fprintf(stderr,RED"Sintaxis: ./truncar <nombre_dispositivo> <ninodo> <nbytes>\n"RESET);
        return FALLO;
    }
    
    ninodo = atoi(argv[2]);
    if (ninodo<0)
    {
        fprintf(stderr, RED"ERROR: La entrada de 'ninodo' no es válida\n"RESET);
        return FALLO;
    }

    nbytes = atoi(argv[3]);
    if (nbytes<0)
    {
        fprintf(stderr, RED"ERROR: La entrada de 'nbytes' no es válida\n"RESET);
        return FALLO;
    }
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./truncar\n"RESET);
        return FALLO;
    }

    if (nbytes == 0)
    {
        if (liberar_inodo(ninodo) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./truncar\n"RESET);
            return FALLO;
        }
    }
    else
    {
        if (mi_truncar_f(ninodo, nbytes) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./truncar\n"RESET);
            return FALLO;
        }
    }
    
    #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
        struct STAT p_stat;
        if (mi_stat_f(ninodo, &p_stat) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./truncar\n"RESET);
            return FALLO;
        }
        
        printf("\n\nDATOS INODO %d:\n", ninodo);
        printf("tipo: %c\n", p_stat.tipo);
        printf("permisos: %d\n", p_stat.permisos);
        
        char atime[80]; char mtime[80]; char ctime[80];
        strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.atime));
        strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.mtime));
        strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&p_stat.ctime));
        
        printf("atime: %s\n", atime);
        printf("mtime: %s\n", mtime);
        printf("ctime: %s\n", ctime);

        printf("nlinks: %d\n", p_stat.nlinks);
        printf("tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
        printf("numBloquesOcupados: %d\n", p_stat.numBloquesOcupados);
    #endif

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./truncar\n"RESET);
        return FALLO;
    }
    return EXITO;
}
