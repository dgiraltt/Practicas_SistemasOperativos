/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "verificacion.h"


/**
 * Main del programa.
 */
int main(int argc, char **argv)
{
    pid_t pid;
    struct STAT p_stat;
    struct INFORMACION info;
    struct entrada entradas[NUMPROCESOS];
    char directorio[38], fichero[49], prueba[128], buffer[BLOCKSIZE];
    int error, offset_escritura, offset_lectura;
    
    
    if (argc != 3)
    {
        fprintf(stderr, RED"Sintaxis: verificacion <nombre_dispositivo> <directorio_simulación>\n"RESET);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./verificacion\n"RESET);
        return FALLO;
    }


    strcpy(directorio, argv[2]);
    fprintf(stderr, "Directorio de Simulación: %s\n", directorio);
    if ((error = mi_stat(directorio, &p_stat)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        }
        return error;
    }

    fprintf(stderr, "Número de Entradas: %ld || NUMPROCESOS: %d\n", (p_stat.tamEnBytesLog/TAMENTRADA), NUMPROCESOS);
    if ((p_stat.tamEnBytesLog/TAMENTRADA) != NUMPROCESOS)
    {
        fprintf(stderr, RED"ERROR: El número de entradas no coincide\n"RESET);
        return FALLO;
    }


    memset(fichero, 0, strlen(fichero));
    sprintf(fichero, "%sinforme.txt", directorio);
    if ((error = mi_creat(fichero, 7)) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        }
        return error;
    }


    if ((error = mi_read(directorio, entradas, 0, sizeof(entradas))) < 0)
    {
        mostrar_error_buscar_entrada(error);
        if (error == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
        }
        return error;
    }


    offset_escritura = 0;
    for(int i=0 ; i<NUMPROCESOS ; i++)
    {
        pid = atoi(strchr(entradas[i].nombre, '_') + 1);

        memset(&info, 0, TAMINFORMACION);
        info.pid = pid;
        info.nEscrituras = 0;

        memset(prueba, 0, strlen(prueba));
        sprintf(prueba, "%s%s/prueba.dat", directorio, entradas[i].nombre);
        
        int cant_registros_buffer_escrituras = 256; 
        struct REGISTRO buffer_escrituras[cant_registros_buffer_escrituras];
        memset(buffer_escrituras, 0, sizeof(buffer_escrituras));


        offset_lectura = 0;
        while(mi_read(prueba, buffer_escrituras, offset_lectura, sizeof(buffer_escrituras)) > 0)
        {
            for(int j=0; j<cant_registros_buffer_escrituras; j++)
            {
                if (buffer_escrituras[j].pid == info.pid)
                {
                    if (info.nEscrituras == 0)
                    {
                        info.MenorPosicion = buffer_escrituras[j];
                        info.MayorPosicion = buffer_escrituras[j];
                        info.PrimeraEscritura = buffer_escrituras[j];
                        info.UltimaEscritura = buffer_escrituras[j];
                        info.nEscrituras++;
                    }
                    else
                    {
                        if (((difftime(buffer_escrituras[j].fecha, info.PrimeraEscritura.fecha)) <= 0) &&
                            (buffer_escrituras[j].nEscritura < info.PrimeraEscritura.nEscritura))
                        {
                            info.PrimeraEscritura = buffer_escrituras[j];
                        }
                        
                        if (((difftime(buffer_escrituras[j].fecha, info.UltimaEscritura.fecha)) >= 0) &&
                            (buffer_escrituras[j].nEscritura > info.UltimaEscritura.nEscritura))
                        {
                            info.UltimaEscritura = buffer_escrituras[j];
                        }
                        
                        if (buffer_escrituras[j].nRegistro < info.MenorPosicion.nRegistro)
                        {
                            info.MenorPosicion = buffer_escrituras[j];
                        }
                        
                        if (buffer_escrituras[j].nRegistro > info.MayorPosicion.nRegistro)
                        {
                            info.MayorPosicion = buffer_escrituras[j];
                        }

                        info.nEscrituras++;
                    }
                }
            }
            
            offset_lectura += sizeof(buffer_escrituras);
            memset(&buffer_escrituras, 0, sizeof(buffer_escrituras));
        }
        

        fprintf(stderr, GRAY"[%d) %d escrituras validadas en %s]\n"RESET, (i+1), info.nEscrituras, prueba);

        memset(buffer, 0, BLOCKSIZE);
        sprintf(buffer, "PID: %d\n"
                        "Numero de escrituras:\t%d\n"
                        "Primera escritura:\t%d\t%d\t%s"
                        "Ultima escritura:\t%d\t%d\t%s"
                        "Menor posición:\t\t%d\t%d\t%s"
                        "Mayor posición:\t\t%d\t%d\t%s\n",
                        info.pid, info.nEscrituras,
                        info.PrimeraEscritura.nEscritura,
                        info.PrimeraEscritura.nRegistro,
                        asctime(localtime(&info.PrimeraEscritura.fecha)),
                        info.UltimaEscritura.nEscritura,
                        info.UltimaEscritura.nRegistro,
                        asctime(localtime(&info.UltimaEscritura.fecha)),
                        info.MenorPosicion.nEscritura,
                        info.MenorPosicion.nRegistro,
                        asctime(localtime(&info.MenorPosicion.fecha)),
                        info.MayorPosicion.nEscritura,
                        info.MayorPosicion.nRegistro,
                        asctime(localtime(&info.MayorPosicion.fecha)));


        if ((offset_escritura += mi_write(fichero, &buffer, offset_escritura, strlen(buffer))) < 0)
        {
            mostrar_error_buscar_entrada(offset_escritura);
            if (offset_escritura == FALLO)
            {
                fprintf(stderr, RED"ERROR EN ./mi_cat\n"RESET);
            }
            return offset_escritura;
        }
    }
}
