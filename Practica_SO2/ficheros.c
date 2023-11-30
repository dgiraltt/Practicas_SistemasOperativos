/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "ficheros.h"


/**
 * Escribe el contenido procedente de un buffer de memoria en un fichero/directorio.
 */
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{   
    mi_waitSem();
    
    struct inodo inodo;
    unsigned char buf_bloque[BLOCKSIZE];
    int primerBL, ultimoBL, desp1, desp2, nbfisico, escritos = 0;
    
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2)
    {
        #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
            fprintf(stderr, RED"ERROR: Permiso denegado de escritura\n"RESET);
        #endif
        mi_signalSem();
        return -6;
    }

    primerBL = offset/BLOCKSIZE;
    ultimoBL = (offset+nbytes-1)/BLOCKSIZE;
    desp1 = offset%BLOCKSIZE;
    desp2 = (offset+nbytes-1)%BLOCKSIZE;


    //Paso 1
    if ((nbfisico = traducir_bloque_inodo(&inodo, primerBL, 1)) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    if (bread(nbfisico, buf_bloque) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    if (primerBL == ultimoBL)
    {
        memcpy(buf_bloque+desp1, buf_original, nbytes);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
            mi_signalSem();
            return FALLO;
        }
        escritos += nbytes;
    }
    else
    {
        memcpy(buf_bloque+desp1, buf_original, BLOCKSIZE-desp1);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
            mi_signalSem();
            return FALLO;
        }
        escritos += BLOCKSIZE-desp1;
        

        //Paso 2
        for (int i=(primerBL+1); i<ultimoBL; i++)
        {
            if ((nbfisico = traducir_bloque_inodo(&inodo, i, 1)) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
                mi_signalSem();
                return FALLO;
            }
            
            memcpy(buf_bloque, (buf_original+(BLOCKSIZE-desp1)+(i-primerBL-1)*BLOCKSIZE), BLOCKSIZE);
            if (bwrite(nbfisico, buf_bloque) == FALLO)
            {
                mi_signalSem();
                fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
                return FALLO;
            }
            escritos += BLOCKSIZE;
        }


        //Paso 3
        if ((nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 1)) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
            mi_signalSem();
            return FALLO;
        }

        if (bread(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
            return FALLO;
        }

        memcpy(buf_bloque, buf_original+(nbytes-(desp2+1)), desp2+1);
        if (bwrite(nbfisico, buf_bloque) == FALLO)
        {
            mi_signalSem();
            fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
            return FALLO;
        }
        escritos += (desp2+1);
    }
    

    if (inodo.tamEnBytesLog < (offset+escritos))
    {
        inodo.tamEnBytesLog = (offset+escritos);
        inodo.ctime = time(NULL);
    }

    inodo.mtime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }


    if (escritos != nbytes)
    {
        mi_signalSem();
        fprintf(stderr, RED"ERROR EN mi_write_f()\n"RESET);
        return FALLO;
    }

    mi_signalSem();
    return escritos;
}


/**
 * Lee información de un fichero/directorio.
 */
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{
    struct inodo inodo;
    char buf_bloque[BLOCKSIZE];
    int primerBL, ultimoBL, desp1, desp2, nbfisico, leidos = 0;
    
    mi_waitSem();
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    inodo.atime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();


    if ((inodo.permisos & 4) != 4)
    {
        #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
            fprintf(stderr, RED"ERROR: Permiso denegado de lectura\n"RESET);
        #endif

        return -3;
    }

    if (offset >= inodo.tamEnBytesLog)
    {
        leidos = 0;
        return leidos;
    }
    if ((offset+nbytes) >= inodo.tamEnBytesLog)
    {
        nbytes = inodo.tamEnBytesLog-offset;  
    }
    
    primerBL = offset/BLOCKSIZE;
    ultimoBL = (offset+nbytes-1)/BLOCKSIZE;
    desp1 = offset%BLOCKSIZE;
    desp2 = (offset+nbytes-1)%BLOCKSIZE;


    //Paso 1
    if (primerBL == ultimoBL)
    {
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
                return FALLO;
            }
            memcpy(buf_original, buf_bloque+desp1, nbytes);
        }
        leidos += nbytes;
    }
    else
    {
        nbfisico = traducir_bloque_inodo(&inodo, primerBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
                return FALLO;
            }
            memcpy(buf_original, buf_bloque+desp1, BLOCKSIZE-desp1);
        }
        leidos += (BLOCKSIZE-desp1);
        

        //Paso 2
        for (int i=(primerBL+1); i<ultimoBL; i++)
        {
            nbfisico = traducir_bloque_inodo(&inodo, i, 0);
            if (nbfisico != FALLO)
            {
                if (bread(nbfisico, buf_bloque) == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
                    return FALLO;
                }
                memcpy((buf_original+(BLOCKSIZE-desp1)+(i-primerBL-1)*BLOCKSIZE), buf_bloque, BLOCKSIZE);
            }
            leidos += BLOCKSIZE;
        }

        
        //Paso 3
        nbfisico = traducir_bloque_inodo(&inodo, ultimoBL, 0);
        if (nbfisico != FALLO)
        {
            if (bread(nbfisico, buf_bloque) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
                return FALLO;
            }
            memcpy(buf_original+(nbytes-(desp2+1)), buf_bloque, desp2+1);
        }
        leidos += (desp2+1);
    }
    

    if (leidos != nbytes)
    {
        fprintf(stderr, RED"ERROR EN mi_read_f()\n"RESET);
        return FALLO;
    }
    return leidos;
}


/**
 * Devuelve la metainformación de un fichero/directorio.
 */
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_stat_f()\n"RESET);
        return FALLO;
    }

    p_stat->tipo = inodo.tipo;
    p_stat->permisos = inodo.permisos;
    p_stat->atime = inodo.atime;
    p_stat->mtime = inodo.mtime;
    p_stat->ctime = inodo.ctime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;

    return EXITO;
}


/**
 * Cambia los permisos de un fichero/directorio.
 */
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    mi_waitSem();

    struct inodo inodo;
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_chmod_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_chmod_f()\n"RESET);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    
    return EXITO;
}


/**
 * Trunca un fichero/directorio a los bytes indicados, liberando los bloques necesarios.
 */
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    int primerBL, bloquesLiberados;

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_truncar_f()\n"RESET);
        return FALLO;
    }

    if ((inodo.permisos & 2) != 2)
    {
        #if DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
            fprintf(stderr, RED"ERROR: Permiso denegado de escritura\n"RESET);
        #endif
        
        return -6;
    }

    if (nbytes>inodo.tamEnBytesLog)
    {
        fprintf(stderr, RED"ERROR EN mi_truncar_f()\n"RESET);
        return FALLO;
    }

    if (nbytes%BLOCKSIZE == 0)
    {
        primerBL = nbytes/BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes/BLOCKSIZE + 1;
    }

    bloquesLiberados = liberar_bloques_inodo(primerBL, &inodo);
    if (bloquesLiberados == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_truncar_f()\n"RESET);
        return FALLO;
    }

    inodo.mtime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloquesLiberados;

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_truncar_f()\n"RESET);
        return FALLO;
    }
    
    return bloquesLiberados;
}
