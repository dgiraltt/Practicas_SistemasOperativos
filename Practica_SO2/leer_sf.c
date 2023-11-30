/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "directorios.h"
int imprimir_superbloque(struct superbloque SB);
void mostrar_buscar_entrada(char *camino, char reservar);


/**
 * Main del programa.
 */
int main(int argc, char **argv)
{
    const char *camino = argv[1];
    if (bmount(camino) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    struct superbloque SB;
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    imprimir_superbloque(SB);


#if DEBUG_NIVEL2
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    
    printf("\n\nRECORRIDO LISTA ENLAZADA DE INODOS LIBRES\n");
    int numInodos = BLOCKSIZE/INODOSIZE;
    
    for (int i=SB.posPrimerBloqueAI; i<SB.posUltimoBloqueAI; i++)
    {
        if (bread(i, &inodos) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
            return FALLO;
        }
        
        for (int j=0; j<numInodos; j++)
        {
            if (inodos[j].tipo == 'l')
            {
                printf("%d ", (i-SB.posPrimerBloqueAI+1)*numInodos + j);
            }
        }
    }
#endif


#if DEBUG_NIVEL3
    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS\n");
    int nBloque = reservar_bloque();
    if (nBloque == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("Se ha reservado el bloque físico nº %d que era el 1º libre indicado por el MB\n", nBloque);
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("SB.cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    
    if (liberar_bloque(nBloque) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);

    printf("\n\nMAPA DE BITS CON BLOQUES DE METADATOS OCUPADOS\n");
    printf("posSB: %d→  leer_bit(%d) = %d\n", posSB, posSB, leer_bit(posSB));
    printf("SB.posPrimerBloqueMB: %d→  leer_bit(%d) = %d\n", SB.posPrimerBloqueMB, SB.posPrimerBloqueMB, leer_bit(SB.posPrimerBloqueMB));
    printf("SB.posUltimoBloqueMB: %d→  leer_bit(%d) = %d\n", SB.posUltimoBloqueMB, SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf("SB.posPrimerBloqueAI: %d→  leer_bit(%d) = %d\n", SB.posPrimerBloqueAI, SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf("SB.posUltimoBloqueAI: %d→  leer_bit(%d) = %d\n", SB.posUltimoBloqueAI, SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf("SB.posPrimerBloqueDatos: %d→  leer_bit(%d) = %d\n", SB.posPrimerBloqueDatos, SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf("SB.posUltimoBloqueDatos: %d→  leer_bit(%d) = %d\n", SB.posUltimoBloqueDatos, SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));
    
    printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");
    struct inodo inodo;
    if (leer_inodo(SB.posInodoRaiz, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);
    
    char atime[80]; char mtime[80]; char ctime[80];
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.atime));
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.mtime));
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.ctime));
    
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);
#endif


#if DEBUG_NIVEL4
    struct inodo inodo;
    
    printf("\nINODO 1. TRADUCCION DE LOS BLOQUES LOGICOS 8, 204, 30.004, 400.004 y 468.750\n");
    int nblogicos[5] = {8, 204, 30004, 400004, 468750};
    int ninodo = reservar_inodo('f', 6);
    if (ninodo == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    
    for (int i=0; i<sizeof(nblogicos)/sizeof(int); i++)
    {
        if (traducir_bloque_inodo(&inodo, nblogicos[i], 1) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
            return FALLO;
        }
        printf("\n");
    }

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    
    printf("\nDATOS DEL INODO RESERVADO 1\n");
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }

    printf("tipo: %c\n", inodo.tipo);
    printf("permisos: %d\n", inodo.permisos);
    
    char atime[80]; char mtime[80]; char ctime[80];
    strftime(atime, sizeof(atime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.atime));
    strftime(mtime, sizeof(mtime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.mtime));
    strftime(ctime, sizeof(ctime), "%a %Y-%m-%d %H:%M:%S", localtime(&inodo.ctime));
    
    printf("atime: %s\n", atime);
    printf("mtime: %s\n", mtime);
    printf("ctime: %s\n", ctime);

    printf("nlinks: %d\n", inodo.nlinks);
    printf("tamEnBytesLog: %d\n", inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n", inodo.numBloquesOcupados);

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    printf("\nSB.posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
#endif


#if DEBUG_NIVEL7
    mostrar_buscar_entrada("pruebas/", 1); //ERROR_CAMINO_INCORRECTO
    mostrar_buscar_entrada("/pruebas/", 0); //ERROR_NO_EXISTE_ENTRADA_CONSULTA
    mostrar_buscar_entrada("/pruebas/docs/", 1); //ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
    mostrar_buscar_entrada("/pruebas/", 1); // creamos /pruebas/
    mostrar_buscar_entrada("/pruebas/docs/", 1); //creamos /pruebas/docs/
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1); //ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
    mostrar_buscar_entrada("/pruebas/", 1); //ERROR_ENTRADA_YA_EXISTENTE
    mostrar_buscar_entrada("/pruebas/docs/doc1", 0); //consultamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/docs/doc1", 1); //creamos /pruebas/docs/doc1
    mostrar_buscar_entrada("/pruebas/casos/", 1); //creamos /pruebas/casos/
    mostrar_buscar_entrada("/pruebas/docs/doc2", 1); //creamos /pruebas/docs/doc2
#endif


    if (bumount() == FALLO)
    {
        fprintf(stderr,RED"ERROR EN ./leer_sf\n"RESET);
        return FALLO;
    }
    return EXITO;
}


/**
 * Imprime los datos del superbloque.
 */
int imprimir_superbloque(struct superbloque SB)
{
    printf("DATOS DEL SUPERBLOQUE\n");
    printf("posPrimerBloqueMB: %d\n", SB.posPrimerBloqueMB);
    printf("posUltimoBloqueMB: %d\n", SB.posUltimoBloqueMB);
    printf("posPrimerBloqueAI: %d\n", SB.posPrimerBloqueAI);
    printf("posUltimoBloqueAI: %d\n", SB.posUltimoBloqueAI);
    printf("posPrimerBloqueDatos: %d\n", SB.posPrimerBloqueDatos);
    printf("posUltimoBloqueDatos: %d\n", SB.posUltimoBloqueDatos);
    printf("posInodoRaiz: %d\n", SB.posInodoRaiz);
    printf("posPrimerInodoLibre: %d\n", SB.posPrimerInodoLibre);
    printf("cantBloquesLibres: %d\n", SB.cantBloquesLibres);
    printf("cantInodosLibres: %d\n", SB.cantInodosLibres);
    printf("totBloques: %d\n", SB.totBloques);
    printf("totInodos: %d\n\n", SB.totInodos);

    #if DEBUG_NIVEL2
        printf("\n");
        printf ("sizeof struct superbloque: is: %lu\n", sizeof(struct superbloque));
        printf ("sizeof struct inodo is: %lu\n", sizeof(struct inodo));
    #endif

    return EXITO;
}


/**
 * Muestra la función buscar_entrada().
 */
void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0, p_inodo = 0, p_entrada = 0;
    int error;
    
    printf("\ncamino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error);
    }
    printf("**********************************************************************\n");
    
    return;
}
