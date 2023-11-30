/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/
#include "ficheros_basico.h"


/**
 * Calcula el tamaño en bloques necesario para el mapa de bits.
 */
int tamMB(unsigned int nbloques)
{
    if ((nbloques/8)%BLOCKSIZE != 0)
    {
        return (((nbloques/8)/BLOCKSIZE)+1);
    }
    
    return ((nbloques/8)/BLOCKSIZE);
}


/**
 * Calcula el tamaño en bloques del array de inodos.
 */
int tamAI(unsigned int ninodos)
{
    if ((ninodos*INODOSIZE)%BLOCKSIZE != 0)
    {
        return (((ninodos*INODOSIZE)/BLOCKSIZE)+1);
    }
    
    return ((ninodos*INODOSIZE)/BLOCKSIZE);
}


/**
 * Inicializa los datos del superbloque.
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;

    SB.posPrimerBloqueMB = (posSB + tamSB);
    SB.posUltimoBloqueMB = (SB.posPrimerBloqueMB + tamMB(nbloques) - 1);
    SB.posPrimerBloqueAI = (SB.posUltimoBloqueMB + 1);
    SB.posUltimoBloqueAI = (SB.posPrimerBloqueAI + tamAI(ninodos) - 1);
    SB.posPrimerBloqueDatos = (SB.posUltimoBloqueAI + 1);
    SB.posUltimoBloqueDatos = (nbloques - 1);
    
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN initSB()\n"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos.
 */
int initMB()
{
    struct superbloque SB;
    int tamMetadatos, bloquesEscribir, bytesEscribir, bloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN initMB()\n"RESET);
        return FALLO;
    }

    tamMetadatos = SB.posPrimerBloqueDatos;
    bloquesEscribir = (tamMetadatos/8)/BLOCKSIZE;
    bytesEscribir = (tamMetadatos/8)%BLOCKSIZE;
    
    memset(bufferMB, 255, BLOCKSIZE);
    for (bloqueMB=SB.posPrimerBloqueMB; bloqueMB<=bloquesEscribir; bloqueMB++)
    {
        if (bwrite(bloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN initMB()\n"RESET);
            return FALLO;
        }
    }

    if (bytesEscribir != 0)
    {
        memset(bufferMB, 0, BLOCKSIZE);
        for (int i=0; i<bytesEscribir; i++)
        {
            bufferMB[i] = 255;
        }
        
        if (tamMetadatos%8 != 0)
        {
            int res = 0;
            for (int j=0; j<(tamMetadatos%8); j++)
            {
                int two_power = 1;
                for (int n=0; n<(7-j); n++)
                {
                    two_power *= 2;
                }
                res += two_power;
            }
            bufferMB[bytesEscribir] = res;
        }

        if (bwrite(bloqueMB, bufferMB) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN initMB()\n"RESET);
            return FALLO;
        }
    }

    SB.cantBloquesLibres -= tamMetadatos;
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN initMB()\n"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * Inicializa el array de inodos.
 */
int initAI()
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int contInodos;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN initAI()\n"RESET);
        return FALLO;
    }

    contInodos = SB.posPrimerInodoLibre+1;
    for (int i=SB.posPrimerBloqueAI; i<=SB.posUltimoBloqueAI; i++)
    {
        if (bread(i, &inodos) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN initAI()\n"RESET);
            return FALLO;
        }

        for (int j=0; j<(BLOCKSIZE/INODOSIZE); j++)
        {
            inodos[j].tipo = 'l';
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos;
                contInodos++;
            }
            else
            {
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break;
            }
        }

        if (bwrite(i, &inodos) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN initAI()\n"RESET);
            return FALLO;
        }
    }

    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN initAI()\n"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * Escribe un determinado bit en el MB.
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char bufferMB[BLOCKSIZE], mascara = 128;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_bit()\n"RESET);
        return FALLO;
    }

    posbyte = nbloque/8;
    posbit = nbloque%8;

    nbloqueMB = posbyte/BLOCKSIZE;
    nbloqueabs = SB.posPrimerBloqueMB+nbloqueMB;

    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_bit()\n"RESET);
        return FALLO;
    }
    posbyte = posbyte%BLOCKSIZE;

    mascara >>= posbit;
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara;
    }
    else if (bit == 0)
    {
        bufferMB[posbyte] &= ~mascara;
    }

    if (bwrite(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_bit()\n"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * Lee un determinado bit del MB y devuelve el valor del bit leído.
 */
char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;
    int posbyte, posbit, nbloqueMB, nbloqueabs;
    unsigned char bufferMB[BLOCKSIZE], mascara = 128;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN leer_bit()\n"RESET);
        return FALLO;
    }

    posbyte = nbloque/8;
    posbit = nbloque%8;

    nbloqueMB = posbyte/BLOCKSIZE;
    nbloqueabs = SB.posPrimerBloqueMB+nbloqueMB;

    #if DEBUG_NIVEL3
        printf(GRAY"[leer_bit(%d)→ posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n"RESET, nbloque, posbyte, posbit, nbloqueMB, nbloqueabs);
    #endif

    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN leer_bit()\n"RESET);
        return FALLO;
    }
    posbyte = posbyte%BLOCKSIZE;

    mascara >>= posbit;
    mascara &= bufferMB[posbyte];
    mascara >>= (7-posbit);

    return mascara;
}


/**
 * Encuentra el primer bloque libre, consultando el MB, lo ocupa y devuelve su posición.
 */
int reservar_bloque()
{
    struct superbloque SB;
    int nbloqueabs, posbyte, posbit, nbloque;
    unsigned char bufferMB[BLOCKSIZE], bufferAux[BLOCKSIZE], mascara = 128;

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
        return FALLO;
    }
    
    if (SB.cantBloquesLibres <= 0)
    {
        fprintf(stderr, RED"ERROR: No hay bloques libres\n"RESET);
        return FALLO;
    }

    memset(bufferAux, 255, BLOCKSIZE);
    nbloqueabs = SB.posPrimerBloqueMB;
    if (bread(nbloqueabs, bufferMB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
        return FALLO;
    }
    
    while ((memcmp(bufferMB, bufferAux, BLOCKSIZE) == 0) && (nbloqueabs<=SB.posUltimoBloqueMB))
    {
        if (bread(++nbloqueabs, bufferMB) == FALLO)
        {
            fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
            return FALLO;
        }
    }    

    posbyte = 0;
    while ((bufferMB[posbyte] == 255) && (posbyte<BLOCKSIZE))
    {
        posbyte++;
    }

    posbit = 0;
    while (bufferMB[posbyte] & mascara)
    {
        bufferMB[posbyte] <<= 1;
        posbit++;
    }

    nbloque = ((nbloqueabs - SB.posPrimerBloqueMB) * BLOCKSIZE + posbyte) * 8 + posbit;
    if (escribir_bit(nbloque, 1) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
        return FALLO;
    }
    
    SB.cantBloquesLibres--;
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
        return FALLO;
    }
    
    memset(bufferAux, 0, BLOCKSIZE);
    if (bwrite(nbloque, bufferAux) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_bloque()\n"RESET);
        return FALLO;
    }

    return nbloque;
}


/**
 * Libera un bloque determinado.
 */
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;
    
    if (escribir_bit(nbloque, 0) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_bloque()\n"RESET);
        return FALLO;
    }
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_bloque()\n"RESET);
        return FALLO;
    }
    
    SB.cantBloquesLibres++;
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_bloque()\n"RESET);
        return FALLO;
    }

    return nbloque;
}


/**
 * Escribe el contenido de una variable de tipo struct inodo, en un determinado inodo del array de inodos.
 */
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int nbloque;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_inodo()\n"RESET);
        return FALLO;
    }

    nbloque = (ninodo/8)+SB.posPrimerBloqueAI;
    if (bread(nbloque, &inodos) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_inodo()\n"RESET);
        return FALLO;
    }

    inodos[(ninodo%(BLOCKSIZE/INODOSIZE))] = *inodo;
    if (bwrite(nbloque, &inodos) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN escribir_inodo()\n"RESET);
        return FALLO;
    }

    return EXITO;
}


/**
 * Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo.
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    struct inodo inodos[BLOCKSIZE/INODOSIZE];
    int nbloque;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN leer_inodo()\n"RESET);
        return FALLO;
    }

    nbloque = (ninodo/8)+SB.posPrimerBloqueAI;
    if (bread(nbloque, &inodos) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN leer_inodo()\n"RESET);
        return FALLO;
    }

    *inodo = inodos[(ninodo%(BLOCKSIZE/INODOSIZE))];
    return EXITO;
}


/**
 * Encuentra el primer inodo libre, devuelve su número y actualiza la lista enlazada de inodos libres.
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    struct inodo inodo;
    int posInodoReservado;
    
    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_inodo()\n"RESET);
        return FALLO;
    }


    if (SB.cantInodosLibres <= 0)
    {
        fprintf(stderr, RED"ERROR EN reservar_()\n"RESET);
        return FALLO;
    }

    posInodoReservado = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre++;
    
    inodo.tipo = tipo;
    inodo.permisos = permisos;
    inodo.nlinks = 1;
    inodo.tamEnBytesLog = 0;
    inodo.atime = time(NULL);
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.numBloquesOcupados = 0;
    for (int i=0; i<12; i++)
    {
        inodo.punterosDirectos[i] = 0;
    }   
    for (int i=0; i<3; i++)
    {   
        inodo.punterosIndirectos[i] = 0;
    }
    
    if (escribir_inodo(posInodoReservado, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_inodo()\n"RESET);
        return FALLO;
    }
    
    SB.cantInodosLibres--;
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN reservar_inodo()\n"RESET);
        return FALLO;
    }

    return posInodoReservado;
}


/**
 * Obtiene el rango de punteros en el que se sitúa el bloque lógico que buscamos y obtenemos
 * la dirección almacenada en el puntero correspondiente del inodo.
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico<DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico<INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico<INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico<INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        fprintf(stderr, RED"ERROR EN obtener_nRangoBL()\n"RESET);
        return FALLO;
    }
}


/**
 * Obtiene el índice de los bloques de punteros.
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico<DIRECTOS)
    {
        return nblogico;
    }
    else if (nblogico<INDIRECTOS0)
    {
        return (nblogico-DIRECTOS);
    }
    else if (nblogico<INDIRECTOS1)
    {
        if (nivel_punteros == 2)
        {
            return ((nblogico-INDIRECTOS0)/NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico-INDIRECTOS0)%NPUNTEROS);
        }
    }
    else if (nblogico<INDIRECTOS2)
    {
        if (nivel_punteros == 3)
        {
            return ((nblogico-INDIRECTOS1)/(NPUNTEROS*NPUNTEROS));
        }
        else if (nivel_punteros == 2)
        {
            return (((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))/NPUNTEROS);
        }
        else if (nivel_punteros == 1)
        {
            return (((nblogico-INDIRECTOS1)%(NPUNTEROS*NPUNTEROS))%NPUNTEROS);
        }
    }
    
    fprintf(stderr, RED"ERROR EN obtener_indice()\n"RESET);
    return FALLO;
}


/**
 * Obtiene el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 */
int traducir_bloque_inodo(struct inodo *inodo, unsigned int nblogico, unsigned char reservar)
{
    unsigned int ptr = 0, ptr_ant = 0, buffer[NPUNTEROS];
    int nRangoBL, nivel_punteros, indice;

    nRangoBL = obtener_nRangoBL(inodo, nblogico, &ptr);
    if (nRangoBL == FALLO)
    {
        fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
        return FALLO;
    }
    nivel_punteros = nRangoBL;

    while (nivel_punteros > 0)
    {
        if (ptr == 0)
        {
            if (reservar == 0)
            {
                //fprintf(stderr,ROJO_T"ERROR EN traducir_bloque_inodo()\n"RESET);
                return FALLO;
            }

            ptr = reservar_bloque();
            if (ptr == FALLO)
            {
                fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
                return FALLO;
            }
            inodo->numBloquesOcupados++;
            inodo->ctime = time(NULL);

            if (nivel_punteros == nRangoBL)
            {
                inodo->punterosIndirectos[nRangoBL-1] = ptr;
                #if DEBUG_NIVEL4 || DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
                    fprintf(stderr, GRAY"[traducir_bloque_inodo()→  inodo.punterosIndirectos[%d] = %d (reservado BF %d para punteros_nivel%d)]\n"RESET, nRangoBL-1, ptr, ptr, nRangoBL);
                #endif
            }
            else
            {
                buffer[indice] = ptr;
                #if DEBUG_NIVEL4 || DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
                    fprintf(stderr, GRAY"[traducir_bloque_inodo()→  inodo.punteros_nivel%d[%d] = %d (reservado BF %d para punteros_nivel%d)]\n"RESET, nRangoBL, indice, ptr, ptr, nRangoBL-1);
                #endif
                
                if (bwrite(ptr_ant, buffer) == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
                    return FALLO;
                }
            }

            memset(buffer, 0, BLOCKSIZE);
        }
        else
        {
            if (bread(ptr, buffer) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
                return FALLO;
            }
        }

        indice = obtener_indice(nblogico, nivel_punteros);
        if (indice == FALLO)
        {
            fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
            return FALLO;
        }

        ptr_ant = ptr;
        ptr = buffer[indice];
        nivel_punteros--;
    }

    
    if (ptr == 0)
    {
        if (reservar == 0)
        {
            //fprintf(stderr,ROJO_T"ERROR EN traducir_bloque_inodo()\n"RESET);
            return FALLO;
        }
    
        ptr = reservar_bloque();
        if (ptr == FALLO)
        {
            fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
            return FALLO;
        }
        inodo->numBloquesOcupados++;
        inodo->ctime = time(NULL);

        if (nRangoBL == 0)
        {
            inodo->punterosDirectos[nblogico] = ptr;
            #if DEBUG_NIVEL4 || DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
                fprintf(stderr, GRAY"[traducir_bloque_inodo()→  inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n"RESET, nblogico, ptr, ptr, nblogico);
            #endif
        }
        else
        {
            buffer[indice] = ptr;
            #if DEBUG_NIVEL4 || DEBUG_NIVEL5 || DEBUG_NIVEL6 || DEBUG_ENTREGA1
                fprintf(stderr, GRAY"[traducir_bloque_inodo()→  inodo.punteros_nivel1[%d] = %d (reservado BF %d para BL %d)]\n"RESET, indice, ptr, ptr, nblogico);
            #endif
            
            if (bwrite(ptr_ant, buffer) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN traducir_bloque_inodo()\n"RESET);
                return FALLO;
            }
        }
    }

    return ptr;
}


/**
 * Liberar un inodo implica por un lado, que tal inodo pasará a la cabeza de la lista de inodos libres.
 */
int liberar_inodo(unsigned int ninodo)
{
    struct superbloque SB;
    struct inodo inodo;
    int bloquesLiberados;
    
    if (leer_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_inodo()\n"RESET);
        return FALLO;
    }
    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, RED"ERROR: No hay permisos de escritura\n"RESET);
        return FALLO;
    }

    bloquesLiberados = liberar_bloques_inodo(0, &inodo);
    if (bloquesLiberados == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_inodo()\n"RESET);
        return FALLO;
    }

    inodo.numBloquesOcupados -= bloquesLiberados;
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    if (bread(posSB, &SB) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_inodo()\n"RESET);
        return FALLO;
    }

    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre; 
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;
    
    if (bwrite(posSB, &SB) == FALLO)
    {
        fprintf(stderr,RED"ERROR EN liberar_inodo()\n"RESET);
        return FALLO;
    }

    inodo.ctime = time(NULL);
    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        fprintf(stderr, RED"ERROR EN liberar_inodo()\n"RESET);
        return FALLO;
    }

    return ninodo;
}


/**
 * libera todos los bloques ocupados a partir del bloque lógico indicado.
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int bloques_punteros[3][NPUNTEROS];
    unsigned char bufAux_punteros[BLOCKSIZE];
    unsigned int nivel_punteros, indice, ptr, nBL, ultimoBL;
    int nRangoBL, ptr_nivel[3], indices[3], liberados = 0;

    if (inodo->tamEnBytesLog == 0)
    {
        return 0;
    }
    
    if (inodo->tamEnBytesLog%BLOCKSIZE == 0)
    {
        ultimoBL = ((inodo->tamEnBytesLog)/BLOCKSIZE)-1;
    }
    else
    {
        ultimoBL = (inodo->tamEnBytesLog)/BLOCKSIZE;
    }

    memset(bufAux_punteros, 0, BLOCKSIZE);
    ptr = 0;
    #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
        fprintf(stderr, "[liberar_bloques_inodo()→  primerBL: %d, ultimoBL: %d]\n", primerBL, ultimoBL);
        unsigned int nBL_aux = primerBL-1;
        int breads = 0, bwrites = 0;
    #endif

    for (nBL=primerBL; nBL<=ultimoBL; nBL++)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr);
        if (nRangoBL == FALLO)
        {
            fprintf(stderr, RED"ERROR EN liberar_bloques_inodo()\n"RESET);
            return FALLO;
        }

        nivel_punteros = nRangoBL;
        while(ptr > 0 && nivel_punteros > 0)
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == FALLO)
            {
                fprintf(stderr, RED"ERROR EN liberar_bloques_inodo()\n"RESET);
                return FALLO;
            }

            if ((indice == 0) || (nBL == primerBL))
            {
                if (bread(ptr, bloques_punteros[nivel_punteros-1]) == FALLO)
                {
                    fprintf(stderr, RED"ERROR EN liberar_bloques_inodo()\n"RESET);
                    return FALLO;
                }
                
                #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
                    breads++;
                #endif
            }

            ptr_nivel[nivel_punteros-1] = ptr;
            indices[nivel_punteros-1] = indice;
            ptr = bloques_punteros[nivel_punteros-1][indice];
            nivel_punteros--;
        }
        
        
        if (ptr > 0)
        {
            #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
                if (nBL != nBL_aux+1)
                {
                    fprintf(stderr, BLUE"\n[liberar_bloques_inodo()→  Estamos en el BL%d y saltamos hasta el BL%d\n"RESET, nBL_aux+1, nBL-1);
                }
            #endif
            
            if (liberar_bloque(ptr) == FALLO)
            {
                fprintf(stderr, RED"ERROR EN liberar_bloques_inodo()\n"RESET);
                return FALLO;
            }
            liberados++;

            #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
                fprintf(stderr, GRAY"[liberar_bloques_inodo()→  liberado BF %d de datos para BL %d]\n"RESET, ptr, nBL);
                nBL_aux = nBL;
            #endif
            
            if(nRangoBL == 0)
            {
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros-1];
                    bloques_punteros[nivel_punteros-1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros-1];

                    if (memcmp(bloques_punteros[nivel_punteros-1], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        if (liberar_bloque(ptr) == FALLO)
                        {
                            fprintf(stderr, RED"ERROR EN liberar_bloques_inodo()\n"RESET);
                            return FALLO;
                        }
                        liberados++;

                        #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
                            fprintf(stderr, GRAY"[liberar_bloques_inodo()→  liberado BF %d de punteros_nivel%u correspondiente al BL %d]\n"RESET, ptr, nivel_punteros, nBL_aux);
                        #endif

                        switch (nivel_punteros-1)
                        {
                            case 0:
                                nBL += NPUNTEROS-indices[nivel_punteros-1] -1;
                                break;
                            
                            case 1:
                                nBL += NPUNTEROS*(NPUNTEROS-indices[nivel_punteros-1]) -1;
                                break;
                            
                            case 2:
                                nBL += NPUNTEROS*NPUNTEROS*(NPUNTEROS-indices[nivel_punteros-1]) -1;
                                break;
                        }

                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL-1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else
                    {
                        if (bwrite(ptr, bloques_punteros[nivel_punteros-1]) == FALLO)
                        {
                            return FALLO;
                        }
                        
                        #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
                            fprintf(stderr, ORANGE"[liberar_bloques_inodo()→ salvado BF %d de punteros_nivel%u correspondiente al BL %d\n"RESET, ptr, nivel_punteros, nBL_aux);
                            bwrites++;
                        #endif

                        nivel_punteros = nRangoBL+1;
                    }
                }
            }
        }
        else
        {
            switch (nivel_punteros-1)
            {
                case 1:
                    nBL += NPUNTEROS-1;
                    break;
                
                case 2:
                    nBL += NPUNTEROS*NPUNTEROS-1;
                    break;
            }
        }
    }

    #if DEBUG_NIVEL6 || DEBUG_ENTREGA1
        fprintf(stderr, "\n[liberar_bloques_inodo()→  total bloques liberados: %d, total breads: %d, total bwrites: %d]\n", liberados, breads, bwrites);
    #endif
    
    return liberados;
}
