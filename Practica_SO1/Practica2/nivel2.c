/* @author Daniel Giralt Pascual */

#include "nivel2.h"
static int n_tokens;


/**
 * Main del programa.
 */
int main()
{
    while (1)
    {
        if (read_line(line))
        {
            execute_line(line);
        }
    }

    return EXITO;
}


/**
* Imprime el promt de la línea de comandos.
*/
void imprimir_prompt()
{
    fprintf(stderr, ROSA_T"%c "RESET, PROMPT);
}


/**
 * Lee la línea de comandos del shell.
 */
char *read_line(char *line)
{
    imprimir_prompt();
    fflush(stdout);
    memset(line, '\0', COMMAND_LINE_SIZE);

    if (fgets(line, COMMAND_LINE_SIZE, stdin) == NULL)
    {
        fprintf(stderr, "\r");
        if (feof(stdin))
        {
            fprintf(stderr, GRIS_T NEGRITA"\n¡HASTA LA PRÓXIMA!\n"RESET);
            exit(0);
        }
    }

    //Quitamos salto de línea
    char *aux = strchr(line, '\n');
    if (aux)
    {
        *aux = '\0';
    }
    
    return line;
}


/**
 * Ejecuta la instrucción del comando.
 */
int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    n_tokens = parse_args(args, line);
    
    if (args[0] != NULL)
    {
        check_internal(args);
        return EXITO;
    }

    return FALLO;
}


/**
 * Trocea la línea con los argumentos del comando.
 */
int parse_args(char **args, char *line)
{
    int nTokensAux = 0;
    char *token;

    #if DEBUG1
        int corregido = 0;
    #endif

    token = strtok(line, " \t\n\r");
    while(token != NULL)
    {
        args[nTokensAux] = token;
        #if DEBUG1
            fprintf(stderr, GRIS_T"[parse_args()→ token %i: %s]\n"RESET, nTokensAux, args[nTokensAux]);
        #endif
        
        if(args[nTokensAux][0] == '#')
        {
            token = NULL;
            #if DEBUG1
                corregido = 1;
            #endif
        }
        else
        {
            token = strtok(NULL, " \t\n\r");
            nTokensAux++;
        }
    }

    args[nTokensAux] = NULL;
    #if DEBUG1
        if (!corregido)
        {
            fprintf(stderr, GRIS_T"[parse_args()→ token %i: %s]\n"RESET, nTokensAux, args[nTokensAux]);
        }
        else
        {
            fprintf(stderr, GRIS_T"[parse_args()→ token %i corregido: %s]\n"RESET, nTokensAux, args[nTokensAux]);
        }
    #endif
    
    return nTokensAux;
}


/**
 * Comprueba si la instrucción pasada es un comando interno.
 */
int check_internal(char **args)
{
    if (args[0] == NULL)
    {
        return FALLO;
    }
    else if (!strcmp(args[0], "cd"))
    {
        internal_cd(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "export"))
    {
        internal_export(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "source"))
    {
        internal_source(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "jobs"))
    {
        internal_jobs(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "fg"))
    {
        internal_fg(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "bg"))
    {
        internal_bg(args);
        return EXITO;
    }
    else if(!strcmp(args[0], "exit"))
    {
        exit(0);
    }

    return FALLO;
}


/**
 * Cambia de directorio.
 */
int internal_cd(char **args)
{
    if (args[1] == NULL)
    {
        if (chdir(getenv("HOME")) != 0)
        {
            fprintf(stderr, ROJO_T"chdir: %s\n"RESET, strerror(errno));
            return FALLO;
        }
    }
    else
    {
        if (cd_avanzado(args) == FALLO)
        {
            return FALLO;
        }

        if (chdir(args[1]) != 0)
        {
            fprintf(stderr, ROJO_T"chdir: %s\n"RESET, strerror(errno));
            return FALLO;
        }
    }


    #if DEBUG2
        char cwd[COMMAND_LINE_SIZE];
        if (getcwd(cwd, COMMAND_LINE_SIZE) == NULL)
        {
            fprintf(stderr, ROJO_T"getcwd: %s\n"RESET, strerror(errno));
        }
        fprintf(stderr, GRIS_T"[internal_cd()→ PWD: %s]\n"RESET, cwd);
    #endif
    
    return EXITO;
}


/**
 * Asigna valores a variables de entorno.
 */
int internal_export(char **args)
{
    char *nombre = strtok(args[1], "="), *valor = strtok(NULL, " ");

    if ((nombre != NULL) && (valor != NULL))
    {
        #if DEBUG2
            fprintf(stderr, GRIS_T"[internal_export()→ nombre: %s]\n"RESET, nombre);
            fprintf(stderr, GRIS_T"[internal_export()→ valor: %s]\n"RESET, valor);
        #endif

        if (getenv(nombre) == NULL)
        {
            fprintf(stderr, ROJO_T"getenv: %s\n"RESET, strerror(errno));
        }
        else
        {
            #if DEBUG2
                fprintf(stderr, GRIS_T"[internal_export()→ antiguo valor para %s: %s]\n"RESET, nombre, getenv(nombre));
            #endif

            setenv(nombre, valor, 1);
            
            #if DEBUG2
                fprintf(stderr, GRIS_T"[internal_export()→ nuevo valor para %s: %s]\n"RESET, nombre, valor);
            #endif

            return EXITO;
        }
    }
    else if (nombre == NULL)
    {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: export Nombre=Valor\n"RESET);
    }
    else
    {
        #if DEBUG2
            fprintf(stderr, GRIS_T"[internal_export()→ nombre: %s]\n"RESET, nombre);
            fprintf(stderr, GRIS_T"[internal_export()→ valor = (null)]\n"RESET);
        #endif
        
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: export Nombre=Valor\n"RESET);
    }
    
    return FALLO;
}


/**
 * Ejecuta un fichero de la línea de comandos.
 */
int internal_source(char **args)
{
    fprintf(stderr, GRIS_T"[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
    return EXITO;
}


/**
 * Muestra el PID de los procesos que no estén en foreground.
 */
int internal_jobs(char **args)
{
    fprintf(stderr, GRIS_T"[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n"RESET);
    return EXITO;
}


/**
 * Lleva los procesos más recientes a primer plano.
 */
int internal_fg(char **args)
{
    fprintf(stderr, GRIS_T"[internal_fg()→ Esta función lleva los procesos más recientes a primer plano]\n"RESET);
    return EXITO;
}


/**
 * Enseña los procesos parados o en segundo plano.
 */
int internal_bg(char **args)
{
    fprintf(stderr, GRIS_T"[internal_bg()→ Esta función enseña los procesos parados o en segundo plano]\n"RESET);
    return EXITO;
}


/*****************************************************************************/


/**
 * Función auxiliar que permite tratar un directorio pasado entre comillas
 * dobles, simple, y el carácter \, permitiendo acceder a directorios que
 * contienen un espacio (" ") en su nombre.
 */
int cd_avanzado(char **args)
{
    char *token = args[1], *tokenAux;
    char comillas;

    if (strchr(token, 92) != NULL)          /*92 es \ en ASCII*/
    {
        for (int i=0; i<strlen(token); i++)
        {
            if (token[i] == 92)
            {
                token[i] = ' ';
            }
        }
        strcat(token, args[2]);
        
        return EXITO;
    }
    else if (strchr(token, 34) != NULL)     /*34 es "" en ASCII*/
    {
        comillas = 34;
    }
    else if (strchr(token,39) != NULL)      /*39 es ' en ASCII*/
    {
        comillas = 39;
    }
    else
    {
        return EXITO;
    }


    int i;
    if (token[strlen(token)-1] == comillas)     //1 sola palabra
    {
        for (i=0; i<(strlen(token)-2); i++)
        {
            token[i] = token[i+1];
        }
        token[i] = '\0';
        
        return EXITO;
    }

    if ((tokenAux = args[2]) == NULL)
    {
        fprintf(stderr, ROJO_T"ERROR: Faltan comillas\n"RESET);
        return FALLO;
    }


    int nTokensAux = 3;
    while ((nTokensAux <= n_tokens) && (strchr(tokenAux, comillas) == NULL))
    {
        token[strlen(token)] = ' ';
        tokenAux = args[nTokensAux++];

        if ((nTokensAux > n_tokens))
        {
            fprintf(stderr, ROJO_T"ERROR: Faltan comillas\n"RESET);
            return FALLO;
        }
    }
    token[strlen(token)] = ' ';

    for (i=0; i<(strlen(token)-2); i++)
    {
        token[i] = token[i+1];
    }
    token[i] = '\0';

    return EXITO;
}
