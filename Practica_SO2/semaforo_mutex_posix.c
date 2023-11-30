/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "semaforo_mutex_posix.h"


/**
 * Inicializa el semáforo.
 */
sem_t *initSem()
{
   sem_t *sem;

   sem = sem_open(SEM_NAME, O_CREAT, S_IRWXU, SEM_INIT_VALUE);
   if (sem == SEM_FAILED)
   {
      return NULL;
   }
   return sem;
}


/**
 * Elimina el semáforo.
 */
void deleteSem()
{
   sem_unlink(SEM_NAME);
}


/**
 * Manda la señal Signal del Semáforo.
 */
void signalSem(sem_t *sem)
{
   sem_post(sem);
}


/**
 * Manda la señal Wait del Semáforo.
 */
void waitSem(sem_t *sem)
{
   sem_wait(sem);
}
