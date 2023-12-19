/* @author Daniel Giralt Pascual */

#include "nivel1.h"
static int n_tokens;


/**
 * Main del programa.
 * 
 * @return EXITO.
 */
int main()
{
    char line[COMMAND_LINE_SIZE];

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
 * 
 * @param line: Cadena de carácteres en la que leeremos la línea de comandos.
 * @return: Línea de comandos guardada en line.
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

    line[strlen(line)-1] = '\0';

    return line;
}


/**
 * Ejecuta la instrucción del comando.
 * 
 * @param line: Línea del comando a ejecutar.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int execute_line(char *line)
{
    char *args[ARGS_SIZE];
    if ((n_tokens = parse_args(args, line)) == 0)
    {
        return FALLO;
    }
    
    return check_internal(args);
}


/**
 * Trocea la línea con los argumentos del comando.
 * 
 * @param args: Array de cadenas de carácteres.
 * @param line: Línea del comando a ejecutar.
 * @return: Número de tokens.
 */
int parse_args(char **args, char *line)
{
    int nTokensAux = 0;
    char *token;

    #if DEBUGN1
        int corregido = 0;
    #endif

    token = strtok(line, " \t\n\r");
    while(token != NULL)
    {
        args[nTokensAux] = token;
        #if DEBUGN1
            fprintf(stderr, GRIS_T"[parse_args()→ token %i: %s]\n"RESET, nTokensAux, args[nTokensAux]);
        #endif
        
        if(args[nTokensAux][0] == '#')
        {
            token = NULL;
            #if DEBUGN1
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
    #if DEBUGN1
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
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario 
 * y 1 si no es un comando interno.
 */
int check_internal(char **args)
{
    if (args[0] == NULL) return FALLO;
    else if (!strcmp(args[0], "cd")) return internal_cd(args);
    else if (!strcmp(args[0], "export")) return internal_export(args);
    else if (!strcmp(args[0], "source")) return internal_source(args);
    else if (!strcmp(args[0], "jobs")) return internal_jobs(args);
    else if (!strcmp(args[0], "fg")) return internal_fg(args);
    else if (!strcmp(args[0], "bg")) return internal_bg(args);
    else if (!strcmp(args[0], "exit"))
    {
        fprintf(stderr, GRIS_T NEGRITA"¡HASTA LA PRÓXIMA!\n"RESET);
        exit(0);
    }

    return 1;
}


/**
 * Cambia de directorio.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_cd(char **args)
{
    fprintf(stderr, GRIS_T"[internal_cd()→ Esta función cambiará de directorio]\n"RESET);
    return EXITO;
}


/**
 * Asigna valores a variables de entorno.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_export(char **args)
{
    fprintf(stderr, GRIS_T"[internal_export()→ Esta función asignará valores a variables de entorno]\n"RESET);
    return EXITO;
}


/**
 * Ejecuta un fichero de la línea de comandos.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_source(char **args)
{
    fprintf(stderr, GRIS_T"[internal_source()→ Esta función ejecutará un fichero de líneas de comandos]\n"RESET);
    return EXITO;
}


/**
 * Muestra el PID de los procesos que no estén en foreground.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_jobs(char **args)
{
    fprintf(stderr, GRIS_T"[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n"RESET);
    return EXITO;
}


/**
 * Lleva los procesos más recientes a primer plano.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_fg(char **args)
{
    fprintf(stderr, GRIS_T"[internal_fg()→ Esta función lleva los procesos más recientes a primer plano]\n"RESET);
    return EXITO;
}


/**
 * Enseña los procesos parados o en segundo plano.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO.
 */
int internal_bg(char **args)
{
    fprintf(stderr, GRIS_T"[internal_bg()→ Esta función enseña los procesos parados o en segundo plano]\n"RESET);
    return EXITO;
}
