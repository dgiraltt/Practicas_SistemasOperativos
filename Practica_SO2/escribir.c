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
    int offsets[] = {9000, 209000, 30725000, 409605000, 480000000};
    int nbytes, diferentes_inodos, ninodo, escritos;
    char *camino, *buffer;
    
    if (argc != 4)
    {
        fprintf(stderr, RED"Sintaxis: ./escribir <nombre_dispositivo> <'$(cat fichero)'> <diferentes_inodos>\n"
                            "Offsets: 9000, 209000, 30725000, 409605000, 480000000)\n"
                            "Si diferentes_inodos=0 se reserva un solo inodo para todos los offsets\n"RESET);
        return FALLO;
    }

    buffer = argv[2];
    nbytes = strlen(buffer);
    diferentes_inodos = atoi(argv[3]);
    
    if (diferentes_inodos<0 || diferentes_inodos>1)
    {
        fprintf(stderr, RED"ERROR: La entrada de 'diferentes inodos' no es válida\n"RESET);
        return FALLO;
    }

    #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
        printf("longitud texto: %d\n\n", nbytes);
    #endif
    
    camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
        return FALLO;
    }
    
    if (diferentes_inodos == 0)
    {
        ninodo = reservar_inodo ('f', 6);
        if (ninodo == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
            return FALLO;
        }
    }
    for (int i=0; i<sizeof(offsets)/sizeof(int); i++)
    {
        if (diferentes_inodos == 1)
        {
            ninodo = reservar_inodo ('f', 6);
            if (ninodo == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
                return FALLO;
            }
        }
        
        #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
            printf("Nº inodo reservado: %d\n", ninodo);
            printf("Offset: %d\n", offsets[i]);
        #endif

        escritos = mi_write_f(ninodo, buffer, offsets[i], nbytes);
        if (escritos < 0)
        {
            if (escritos == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
            }
            return FALLO;
        }

        if (mi_stat_f(ninodo, &p_stat) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
            return FALLO;
        }
        
        #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
            printf("Bytes escritos: %d\n", escritos);
            printf("stat.tamEnBytesLog: %d\n", p_stat.tamEnBytesLog);
            printf("stat.numBloquesOcupados: %d\n\n", p_stat.numBloquesOcupados);
        #endif
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./escribir\n"RESET);
        return FALLO;
    }
    return EXITO;
}
