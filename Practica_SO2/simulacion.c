/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "simulacion.h"
#define DEBUG 0
int acabados = 0;


/**
 * Main del programa.
 */
int main(int argc, char **argv)
{
    signal(SIGCHLD, reaper);
    
    struct tm tm;
    time_t tiempo;
    pid_t pid;
    struct REGISTRO registro;
    char camino[21], directorio[38], fichero[48];
    int error;

    if (argc != 2)
    {
        fprintf(stderr, RED"Sintaxis: ./simulacion <nombre_dispositivo>\n"RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
        return FALLO;
    }


    tiempo = time(NULL);
    tm = *localtime(&tiempo);
    memset(camino, 0, strlen(camino));
    sprintf(camino, "/simul_%d%02d%02d%02d%02d%02d/",
        tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    if ((error = mi_creat(camino, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        }
        return error;
    }
    
    for (int proceso=1; proceso<=NUMPROCESOS; proceso++)
    {
        pid = fork();
        if (pid == 0)
        {
            if (bmount(argv[1]) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                return FALLO;
            }

            memset(directorio, 0, strlen(directorio));
            sprintf(directorio, "%sproceso_%d/", camino, getpid());
            if ((error = mi_creat(directorio, 6)) < 0)
            {
                mostrar_error_buscar_entrada(error);
                if (error == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                }
                
                if (bumount() == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                    return FALLO;
                }
                exit(0);
            }

            memset(fichero, 0, strlen(fichero));
            sprintf(fichero, "%sprueba.dat", directorio);
            if ((error = mi_creat(fichero, 6)) < 0)
            {
                mostrar_error_buscar_entrada(error);
                if (error == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                }
                
                if (bumount() == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                    return FALLO;
                }
                exit(0);
            }
            

            srand(time(NULL) + getpid());
            for (int nescritura=1; nescritura<=NUMESCRITURAS; nescritura++)
            {
                memset(&registro, 0, TAMREGISTRO);
                registro.fecha = time(NULL);
                registro.pid = getpid();
                registro.nEscritura = nescritura;
                registro.nRegistro = rand()%REGMAX;

                if ((error = mi_write(fichero, &registro, registro.nRegistro*TAMREGISTRO, TAMREGISTRO)) < 0)
                {
                    mostrar_error_buscar_entrada(error);
                    if (error == FALLO)
                    {
                        fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                    }

                    if (bumount() == FALLO)
                    {
                        fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                        return FALLO;
                    }
                    exit(0);
                }

                #if DEBUG
                    fprintf(stderr, "[simulacion.c -> Escritura %i en %s]\n", nescritura, fichero);
                #endif
                
                //my_sleep(50);
                usleep(50000);
            }

            fprintf(stderr, GRAY"[Proceso %i: Completadas %i escrituras en %s]\n"RESET, proceso, NUMESCRITURAS, fichero);
            if (bumount() == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
                
            }
            exit(0);
        }

        //my_sleep(150);
        usleep(150000);
    }

    while (acabados < NUMPROCESOS)
    {
        pause();
    }

    if (bumount() == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./simulacion\n"RESET);
        return FALLO;
    }
    return EXITO;
}


/*
 * Enterrador.
 */
void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    
    while ((ended = waitpid(FALLO, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}


/*
 * Función que fuerza la espera de los milisegundos.
 */
void my_sleep(unsigned msec)
{
    struct timespec req, rem;
    int err;
    req.tv_sec = msec/1000;
    req.tv_nsec = (msec%1000)*1000000;
    
    while ((req.tv_sec != 0) || (req.tv_nsec != 0))
    {
        if (nanosleep(&req, &rem) == 0) break;
        
        err = errno;
        if (err == EINTR)
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}
