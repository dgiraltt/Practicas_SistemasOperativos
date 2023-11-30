/* @author Daniel Giralt Pascual */

#include "nivel1.h"


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
    parse_args(args, line);
    
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
    fprintf(stderr, GRIS_T"[internal_cd()→ Esta función cambiará de directorio]\n"RESET);
    return EXITO;
}


/**
 * Asigna valores a variables de entorno.
 */
int internal_export(char **args)
{
    fprintf(stderr, GRIS_T"[internal_export()→ Esta función asignará valores a variables de entorno]\n"RESET);
    return EXITO;
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
