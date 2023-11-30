/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "directorios.h"
static struct ultimaEntrada ultimaEntrada[TAMCACHE];
static int CACHE_RESTANTE = TAMCACHE;
static int ultimaCahceActualizada = 0;


/**
 * Dada una cadena de caracteres camino, separa su contenido en dos.
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    if (camino[0] != '/')
    {
        return FALLO;
    }

    char *aux = strchr((camino+1), '/');

    memset(tipo, 0, strlen(tipo));
    strcpy(tipo, "f");
    
    memset(inicial, 0, strlen(inicial));
    memset(final, 0, strlen(final));
    if (aux)
    {
        strncpy(inicial, (camino+1), (strlen(camino)-strlen(aux)-1));
        strcpy(final, aux);

        if (final[0] == '/')
        {
            memset(tipo, 0, strlen(tipo));
            strcpy(tipo, "d");
        }
    }
    else
    {
        strcpy(inicial, (camino+1));
        strcpy(final, "");
    }
    
    return EXITO;
}


/**
 * Busca una determinada entrada entre las entradas del inodocorrespondiente a su directorio padre.
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    struct entrada entrada;
    struct inodo inodo_dir;
    struct superbloque SB;
    char inicial[TAMNOMBRE], final[strlen(camino_parcial)+1], tipo;
    int cant_entradas_inodo, num_entrada_inodo, error, offset;


    if (!strcmp(camino_parcial, "/"))
    {
        if (bread(posSB, &SB) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN buscar_entrada()\n"RESET);
            return FALLO;
        }
        
        if (reservar == 1)
        {
            return ERROR_NO_SE_PUEDE_MODIFICAR_RAIZ;
        }
        
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    memset(inicial, 0, sizeof(inicial));
    memset(final, 0, sizeof(final));
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        return ERROR_CAMINO_INCORRECTO;
    }

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN buscar_entrada()\n"RESET);
        return FALLO;
    }
    if ((inodo_dir.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }

    #if DEBUG_NIVEL7 || DEBUG_NIVEL8
        fprintf(stderr, GRAY"[buscar_entrada()→  inicial: %s, final: %s, reservar: %d]\n"RESET, inicial, final, reservar);
    #endif


    cant_entradas_inodo = inodo_dir.tamEnBytesLog/TAMENTRADA;
    num_entrada_inodo = offset = 0;
    
    int NUMENTRADAS = BLOCKSIZE/TAMENTRADA;
    struct entrada entradas[NUMENTRADAS];
    memset(&entrada, 0, TAMENTRADA);
    memset(entradas, 0, sizeof(entradas));

    if (cant_entradas_inodo > 0)
    {
        if ((error = mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE)) < 0)
        {
            if (error == FALLO)
            {
                fprintf(stderr, RED"ERROR EN buscar_entrada()\n"RESET);
            }
            return error;
        }

        while ((num_entrada_inodo < cant_entradas_inodo) && (strcmp(inicial, entradas[num_entrada_inodo%NUMENTRADAS].nombre)))
        {
            num_entrada_inodo++;
            if ((num_entrada_inodo%NUMENTRADAS) == 0)
            {
                offset += BLOCKSIZE;
                memset(entradas, 0, sizeof(entradas));
                if ((error = mi_read_f(*p_inodo_dir, entradas, offset, BLOCKSIZE)) < 0)
                {
                    if (error == FALLO)
                    {
                        fprintf(stderr, RED"ERROR EN buscar_entrada()\n"RESET);
                    }
                    return error;
                }
            }
        }
        
        memcpy(&entrada, &entradas[num_entrada_inodo%NUMENTRADAS], TAMENTRADA);
    }

    
    if ((strcmp(inicial, entradas[num_entrada_inodo%NUMENTRADAS].nombre)) && (num_entrada_inodo == cant_entradas_inodo))
    {
        switch (reservar)
        {
            case 0:
                return ERROR_NO_EXISTE_ENTRADA_CONSULTA;

            case 1:
                if (inodo_dir.tipo == 'f')
                {
                    return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
                }

                if ((inodo_dir.permisos & 2) != 2)
                {
                    return ERROR_PERMISO_ESCRITURA;
                }

                else
                {
                    strcpy(entrada.nombre, inicial);
                    if (tipo == 'd')
                    {
                        if (!strcmp(final, "/"))
                        {
                            entrada.ninodo = reservar_inodo('d', permisos);
                        }
                        else
                        {
                            return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                        }
                    }
                    else
                    {
                        entrada.ninodo = reservar_inodo('f', permisos);
                    }

                    #if DEBUG_NIVEL7 || DEBUG_NIVEL8
                        fprintf(stderr, GRAY"[buscar_entrada()→  reservado inodo %d tipo %c con permisos %d para %s]\n"RESET, entrada.ninodo, tipo, permisos, entrada.nombre);
                        fprintf(stderr, GRAY"[buscar_entrada()→  creada entrada: %s, %d]\n"RESET, inicial, entrada.ninodo);
                    #endif

                    if (mi_write_f(*p_inodo_dir, &entrada, num_entrada_inodo*TAMENTRADA, TAMENTRADA) == FALLO)  //error de escritura
                    {
                        if (entrada.ninodo != FALLO)
                        {
                            if (liberar_inodo(entrada.ninodo) == FALLO)
                            {
                                fprintf(stderr, RED"ERROR EN buscar_entrada()\n"RESET);
                                return FALLO;
                            }
                        }
                        return FALLO;
                    }
                }    
                
        }
    }
    

    if (!strcmp(final, "/") || !strcmp(final, ""))
    {
        if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
        {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }

        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXITO;
    }
    else
    {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }

    return EXITO;
}


/**
 * Muestra el tipo de error que ha provocado el método buscar_entrada().
 */
void mostrar_error_buscar_entrada(int error)
{
    switch (error)
    {
        case -2: fprintf(stderr, RED"ERROR: Camino incorrecto.\n"RESET); break;
        case -3: fprintf(stderr, RED"ERROR: Permiso denegado de lectura.\n"RESET); break;
        case -4: fprintf(stderr, RED"ERROR: No existe el archivo o el directorio.\n"RESET); break;
        case -5: fprintf(stderr, RED"ERROR: No existe algún directorio intermedio.\n"RESET); break;
        case -6: fprintf(stderr, RED"ERROR: Permiso denegado de escritura.\n"RESET); break;
        case -7: fprintf(stderr, RED"ERROR: El archivo ya existe.\n"RESET); break;
        case -8: fprintf(stderr, RED"ERROR: No es un directorio.\n"RESET); break;
        case -9: fprintf(stderr, RED"ERROR: No se puede crear o modificar la raíz\n"RESET); break;
    }
}


/**
 * Crea un fichero/directorio y su entrada de directorio.
 */
int mi_creat(const char *camino, unsigned char permisos)
{
    mi_waitSem();
    
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_creat()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, permisos)) < 0)
    {
        mi_signalSem();
        return error;
    }
    mi_signalSem();
    
    return EXITO;
}


/**
 * Pone el contenido del directorio en un buffer de memoria y devuelve el número de entradas.
 */
int mi_dir(const char *camino, char *buffer, char tipo, int flag)
{
    struct superbloque SB;
    struct inodo inodo;
    struct entrada entrada;
    struct tm *tm;
    
    int error, numEntradas;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    char tmp[TAMFILA];


    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        return error;
    }

    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
        return FALLO;
    }
    if ((inodo.permisos & 4) != 4)
    {
        return ERROR_PERMISO_LECTURA;
    }
    if (inodo.tipo != tipo)
    {
        fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
        return FALLO;   //Error: la sintaxis no concuerda con el tipo
    }

    if (tipo == 'f')
    {
        numEntradas = 1;
        memset(&entrada, 0, TAMENTRADA);
        if (mi_read_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA) < 0)
        {
            fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
            return FALLO;
        }
        
        if (leer_inodo(entrada.ninodo, &inodo) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
            return FALLO;
        }
        
        if (flag == 1)
        {
            sprintf(tmp, "%c\t", inodo.tipo);
            strcat(buffer, tmp);
            
            if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
            if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
            if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
            strcat(buffer, "\t\t");

            tm = localtime(&inodo.mtime);
            sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
            strcat(buffer, tmp);

            sprintf(tmp, "%d\t\t", inodo.tamEnBytesLog);
            strcat(buffer, tmp);
        }

        strcat(buffer, YELLOW);
        sprintf(tmp, "%s\n", entrada.nombre);
        strcat(buffer, tmp);
        strcat(buffer, RESET);
    }
    else
    {
        if ((numEntradas = inodo.tamEnBytesLog/TAMENTRADA) == 0)
        {
            return 0;
        }

        int offset = 0, NUMENTRADAS = BLOCKSIZE/TAMENTRADA;
        struct entrada entradas[NUMENTRADAS];
        memset(entradas, 0, sizeof(entradas));
        if ((error = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE)) < 0)
        {
            if (error == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
            }
            return error;
        }
        
        for (int i=0; i<numEntradas; i++)
        {
            if (leer_inodo(entradas[i%NUMENTRADAS].ninodo, &inodo) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
                return FALLO;
            }

            if (flag == 1)
            {
                sprintf(tmp, "%c\t", inodo.tipo);
                strcat(buffer, tmp);
                
                if (inodo.permisos & 4) strcat(buffer, "r"); else strcat(buffer, "-");
                if (inodo.permisos & 2) strcat(buffer, "w"); else strcat(buffer, "-");
                if (inodo.permisos & 1) strcat(buffer, "x"); else strcat(buffer, "-");
                strcat(buffer, "\t\t");

                tm = localtime(&inodo.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d\t\t", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);

                sprintf(tmp, "%d\t\t", inodo.tamEnBytesLog);
                strcat(buffer, tmp);
            }
            
            if (inodo.tipo == 'f')
            {
                strcat(buffer, YELLOW);
            }
            else
            {
                strcat(buffer, BLUE);
            }
            sprintf(tmp, "%s\n", entradas[i%NUMENTRADAS].nombre);
            strcat(buffer, tmp);
            strcat(buffer, RESET);
            

            if (((i+1)%NUMENTRADAS) == 0)
            {
                offset += BLOCKSIZE;
                memset(entradas, 0, sizeof(entradas));
                if ((error = mi_read_f(p_inodo, entradas, offset, BLOCKSIZE)) < 0)
                {
                    if (error == FALLO)
                    {
                        fprintf(stderr, RED"ERROR EN mi_dir()\n"RESET);
                    }
                    return error;
                }
            }
        }
    }
    
    return numEntradas;
}


/**
 * Cambia los permisos de un fichero o directorio.
 */
int mi_chmod(const char *camino, unsigned char permisos)
{
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_chmod()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, permisos)) < 0)
    {
        return error;
    }

    if (mi_chmod_f(p_inodo, permisos) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_chmod()\n"RESET);
        return FALLO;
    }
    
    return EXITO;
}


/**
 * Muestra la información acerca del inodo de un fichero o directorio.
 */
int mi_stat(const char *camino, struct STAT *p_stat)
{
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_stat()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;

    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        return error;
    }

    if (mi_stat_f(p_inodo, p_stat) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_stat()\n"RESET);
        return FALLO;
    }
    
    return p_inodo;
}


/**
 * Escribe texto en una posición de un fichero.
 */
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes)
{
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_write()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    
    for (int i=0; i<(TAMCACHE-1); i++)
    {
        if (strcmp(ultimaEntrada[i].camino, camino) == 0)
        {
            p_inodo = ultimaEntrada[i].p_inodo;
            
            #if DEBUG_NIVEL9
                fprintf(stderr, BLUE"[mi_write()→  Utilizamos cache[%d]: %s]\n"RESET, i, camino);
            #endif

            break;
        }
    }
    
    if (p_inodo_dir == p_inodo)     //si no se ha modificado
    {
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
        {
            return error;
        }
        
        if (CACHE_RESTANTE > 0)
        {
            strcpy(ultimaEntrada[TAMCACHE-CACHE_RESTANTE].camino, camino);
            ultimaEntrada[TAMCACHE-CACHE_RESTANTE].p_inodo = p_inodo;
            CACHE_RESTANTE--;
        }
        else
        {
            strcpy(ultimaEntrada[ultimaCahceActualizada].camino, camino);
            ultimaEntrada[ultimaCahceActualizada].p_inodo = p_inodo;
        }
        
        #if DEBUG_NIVEL9
            fprintf(stderr, ORANGE"[mi_write()→  Reemplazamos cache[%d]: %s]\n"RESET, ultimaCahceActualizada, camino);
        #endif

        ultimaCahceActualizada++;
        ultimaCahceActualizada %= TAMCACHE;
    }

    return mi_write_f(p_inodo, buf, offset, nbytes);
}


/**
 * Muestra todo el contenido de un fichero.
 */
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes)
{
    struct superbloque SB;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_read()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    
    for (int i=0; i<(TAMCACHE-1); i++)
    {
        if (strcmp(ultimaEntrada[i].camino, camino) == 0)
        {
            p_inodo = ultimaEntrada[i].p_inodo;

            #if DEBUG_NIVEL9
                fprintf(stderr, BLUE"\n[mi_read()→  Utilizamos cache[%d]: %s]\n"RESET, i, camino);
            #endif

            break;
        }
    }
    
    if (p_inodo_dir == p_inodo)     //si no se ha modificado
    {
        if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
        {
            return error;
        }
        
        if (CACHE_RESTANTE > 0)
        {
            strcpy(ultimaEntrada[TAMCACHE-CACHE_RESTANTE].camino, camino);
            ultimaEntrada[TAMCACHE-CACHE_RESTANTE].p_inodo = p_inodo;
            CACHE_RESTANTE--;
        }
        else
        {
            strcpy(ultimaEntrada[ultimaCahceActualizada].camino, camino);
            ultimaEntrada[ultimaCahceActualizada].p_inodo = p_inodo;
        }

        #if DEBUG_NIVEL9
            fprintf(stderr, ORANGE"[mi_read()→  Reemplazamos cache[%d]: %s]\n"RESET, ultimaCahceActualizada, camino);
        #endif

        ultimaCahceActualizada++;
        ultimaCahceActualizada %= TAMCACHE;
    }

    return mi_read_f(p_inodo, buf, offset, nbytes);
}


/**
 * Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1 .
 */
int mi_link(const char *camino1, const char *camino2)
{
    mi_waitSem();

    struct superbloque SB;
    struct inodo inodo1;
    struct entrada entrada2;
    unsigned int p_inodo_dir1, p_inodo_dir2, p_inodo1, p_inodo2, p_entrada1, p_entrada2;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_link()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    p_inodo_dir1 = p_inodo1 = p_inodo_dir2 = p_inodo2 = SB.posInodoRaiz;
    p_entrada1 = p_entrada2 = 0;
    
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0, 4)) < 0)
    {
        mi_signalSem();
        return error;
    }
    
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0)
    {
        mi_signalSem();
        return error;
    }


    if ((error = mi_read_f(p_inodo_dir2, &entrada2, p_entrada2*TAMENTRADA, TAMENTRADA)) < 0)
    {
        mi_signalSem();
        return error;
    }

    entrada2.ninodo = p_inodo1;

    if ((error = mi_write_f(p_inodo_dir2, &entrada2, p_entrada2*TAMENTRADA, TAMENTRADA)) < 0)
    {
        mi_signalSem();
        return error;
    }

    if (liberar_inodo(p_inodo2) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_link()\n"RESET);
        mi_signalSem();
        return FALLO;
    }


    if (leer_inodo(p_inodo1, &inodo1) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_link()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    inodo1.nlinks++;
    inodo1.ctime = time(NULL);
    if (escribir_inodo(p_inodo1, &inodo1) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_link()\n"RESET);
        mi_signalSem();
        return FALLO;
    }
    mi_signalSem();
    
    return EXITO;
}


/**
 * Borra la entrada de directorio especificada y, en caso de que fuera el último enlace existente, borra el propio fichero/directorio.
 */
int mi_unlink(const char *camino)
{
    mi_waitSem();

    struct superbloque SB;
    struct inodo inodo, inodo_dir;
    struct entrada entrada;
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error, nEntradas;
    
    if (bread(posSB, &SB) == FALLO)
    {
        mi_signalSem();
        fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        mi_signalSem();
        return error;
    }

    if (SB.posInodoRaiz == p_inodo)
    {
        mi_signalSem();
        return ERROR_NO_SE_PUEDE_MODIFICAR_RAIZ;
    }


    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    if ((inodo.tipo == 'd') && (inodo.tamEnBytesLog > 0))
    {
        fprintf(stderr, RED"ERROR: El Directorio %s no está vacío\n"RESET, camino);
        mi_signalSem();
        return -10;
    }


    if (leer_inodo(p_inodo_dir, &inodo_dir) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    nEntradas = inodo_dir.tamEnBytesLog/TAMENTRADA;
    if (p_entrada != nEntradas-1)
    {
        if ((error = mi_read_f(p_inodo_dir, &entrada, (nEntradas-1)*TAMENTRADA, TAMENTRADA)) < 0)
        {
            mi_signalSem();
            return error;
        }

        if ((error = mi_write_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA)) < 0)
        {
            mi_signalSem();
            return error;
        }
    }

    if ((error = mi_truncar_f(p_inodo_dir, (nEntradas-1)*TAMENTRADA)) < 0)
    {
        mi_signalSem();
        return error;
    }


    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
        mi_signalSem();
        return FALLO;
    }

    inodo.nlinks--;
    if (inodo.nlinks == 0)
    {
        if (liberar_inodo(p_inodo) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
            mi_signalSem();
            return FALLO;
        }
    }
    else
    {
        inodo.ctime = time(NULL);
        if (escribir_inodo(p_inodo, &inodo) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN mi_unlink()\n"RESET);
            mi_signalSem();
            return FALLO;
        }
    }
    mi_signalSem();
    
    return EXITO;
}


/**
 * Renombra el archivo o directorio elegido.
 */
int mi_rename(const char *camino, const char *nuevo)
{
    struct superbloque SB;
    struct entrada entrada;
    char caminoNuevo[strlen(nuevo)+1];
    unsigned int p_inodo_dir, p_inodo, p_entrada;
    int error;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN mi_read()\n"RESET);
        return FALLO;
    }

    p_inodo_dir = p_inodo = SB.posInodoRaiz;
    p_entrada = 0;
    
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) < 0)
    {
        return error;
    }

    sprintf(caminoNuevo, "/%s", nuevo);
    if ((error = buscar_entrada(caminoNuevo, &p_inodo_dir, &p_inodo, &p_entrada, 0, 4)) != ERROR_NO_EXISTE_ENTRADA_CONSULTA)
    {
        if (error == EXITO)
        {
            fprintf(stderr, RED"ERROR: Ya hay un archivo con este nombre\n"RESET);
        }
        return error;
    }
    
    if ((error = mi_read_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA)) < 0)
    {
        return error;
    }
    strcpy(entrada.nombre, nuevo);
    if ((error = mi_write_f(p_inodo_dir, &entrada, p_entrada*TAMENTRADA, TAMENTRADA)) < 0)
    {
        return error;
    }

    return EXITO;
}
