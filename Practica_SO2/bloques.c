/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "bloques.h"
static int descriptor = 0;
static sem_t *mutex;
static unsigned int inside_sc = 0;


/**
 * Monta el dispositivo virtual, y dado que se trata de un fichero, esa acción consistirá en abrirlo.
 */
int bmount(const char *camino)
{
    if (descriptor > 0)
    {
        close(descriptor);
    }

    if (!mutex)
    {
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return FALLO;
        }
    }
    
    umask(000);
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);
    if (descriptor < 0)
    {
        fprintf(stderr, RED"ERROR EN bmount()\n"RESET);
        return FALLO;
    }
    return descriptor;
}


/**
 * Desmonta el dispositivo virtual.
 */
int bumount()
{
    descriptor = close(descriptor);
    deleteSem();

    if (descriptor < 0)
    {
        fprintf(stderr, RED"ERROR EN bumount()\n"RESET);
        return FALLO;
    }
    return EXITO;
}


/**
 * Escribe 1 bloque en el dispositivo virtual, en el bloque físico especificado por nbloque.
 */
int bwrite(unsigned int nbloque, const void *buf)
{
    if (lseek(descriptor, nbloque*BLOCKSIZE, SEEK_SET) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN bwrite() (lseek)\n"RESET);
        return FALLO;
    }

    size_t bytes = write(descriptor, buf, BLOCKSIZE);
    if (bytes < 0)
    {
        fprintf(stderr, RED"ERROR EN bwrite()\n"RESET);
        return FALLO;
    }
    return bytes;
}


/**
 * Lee 1 bloque del dispositivo virtual, que se corresponde con el bloque físico especificado por nbloque.
 */
int bread(unsigned int nbloque, void *buf)
{
    if (lseek(descriptor, nbloque*BLOCKSIZE, SEEK_SET) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN bread() (lseek)\n"RESET);
        return FALLO;
    }

    size_t bytes = read(descriptor, buf, BLOCKSIZE);
    if (bytes < 0)
    {
        fprintf(stderr, RED"ERROR EN bread()\n"RESET);
        return FALLO;
    }
    return bytes;
}


/**
 * Manda una señal de Wait al Semáforo.
 */
void mi_waitSem()
{
    if (!inside_sc)
    {
        waitSem(mutex);
    }
    inside_sc++;
}


/**
 * Manda una señal de Signal al Semáforo.
 */
void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}
