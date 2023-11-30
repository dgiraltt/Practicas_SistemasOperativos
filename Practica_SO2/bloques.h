/********************************
 * Autores:
 *   - Marc Ferrer Fernández
 *   - Daniel Giralt Pascual
 *   - Francisco Hermosa Adame
 *******************************/

//LIBRERIAS
#include "semaforo_mutex_posix.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


//CONSTANTES
#define BLOCKSIZE 1024 // bytes
#define EXITO 0 //para gestión errores
#define FALLO -1 //para gestión errores


//DEBUG
#define DEBUG_ENTREGA1 0
#define DEBUG_ENTREGA2 0
#define DEBUG_ENTREGA3 1

#define DEBUG_NIVEL1 0
#define DEBUG_NIVEL2 0
#define DEBUG_NIVEL3 0
#define DEBUG_NIVEL4 0
#define DEBUG_NIVEL5 0
#define DEBUG_NIVEL6 0
#define DEBUG_NIVEL7 0
#define DEBUG_NIVEL8 0
#define DEBUG_NIVEL9 0
#define DEBUG_NIVEL10 0


//COLORES
#define BLACK   "\x1B[30m"
#define RED     "\x1b[31m"
#define GREEN   "\x1b[32m"
#define YELLOW  "\x1b[33m"
#define BLUE    "\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN    "\x1b[36m"
#define WHITE   "\x1B[37m"
#define ORANGE  "\x1B[38;2;255;128;0m"
#define ROSE    "\x1B[38;2;255;151;203m"
#define LBLUE   "\x1B[38;2;53;149;240m"
#define LGREEN  "\x1B[38;2;17;245;120m"
#define GRAY    "\x1B[38;2;176;174;174m"
#define RESET   "\x1b[0m"
#define BOLD    "\x1b[1m"


//FUNCIONES
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int nbloque, const void *buf);
int bread(unsigned int nbloque, void *buf);

void mi_waitSem();
void mi_signalSem();
