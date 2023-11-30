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
    int nbloques = atoi(argv[2]);
    unsigned char buf[BLOCKSIZE];
    memset(buf, '0', BLOCKSIZE);


    //abrimos
    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
        return FALLO;
    }


    //escribimos en los nbloques
    for (int i; i<nbloques; i++)
    {
        if (bwrite(i, buf) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
            return FALLO;
        }
    }


    if (initSB(nbloques, (nbloques/4)) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
        return FALLO;
    }
    if (initMB() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
        return FALLO;
    }
    if (initAI() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
        return FALLO;
    }


    #if !(DEBUG_NIVEL1 || DEBUG_NIVEL2)
        if (reservar_inodo('d', 7) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
            return FALLO;
        }
    #endif

    
    //cerramos
    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./mi_mkfs\n"RESET);
        return FALLO;
    }
    return EXITO;
}
