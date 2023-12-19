/* @author Daniel Giralt Pascual */

#include "nivel3.h"
static int n_tokens;
static char mi_shell[COMMAND_LINE_SIZE]; 
static struct info_job jobs_list[N_JOBS];


/**
 * Main del programa.
 * 
 * @param argc: Número de tokens de la línea de ejecución.
 * @param argv: Array con los diferentes tokens.
 * @return EXITO.
 */
int main(int argc, char *argv[])
{
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

    char line[COMMAND_LINE_SIZE];
    strcpy(mi_shell, argv[0]);
    
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
    #if DEBUGN1 || DEBUGN2
        fprintf(stderr, ROSA_T"%c "RESET, PROMPT);
    #else
        user = getenv("USER");
        home = getenv("HOME");

        char cwd[COMMAND_LINE_SIZE];
        if (getcwd(cwd, COMMAND_LINE_SIZE) == NULL)
        {
            fprintf(stderr, ROJO_T"getcwd: %s\n"RESET, strerror(errno));
        }
        fprintf(stderr, ROSA_T NEGRITA"%s:"RESET, user);
        fprintf(stderr, CYAN_T"MINISHELL"RESET);
        fprintf(stderr, BLANCO_T"%c "RESET, PROMPT);
    #endif
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
    int status;
    
    char lineAux[strlen(line)+1];
    strcpy(lineAux, line);
    
    char *args[ARGS_SIZE];
    if ((n_tokens = parse_args(args, lineAux)) == 0)
    {
        return FALLO;
    }

    if (check_internal(args) < 1)
    {
        return EXITO;
    }


    strcpy(jobs_list[0].cmd, line);
    jobs_list[0].status = 'E';

    pid_t id = fork();
    if (id == 0)
    {
        if (execvp(args[0], args) < 0)
        {
            //fprintf(stderr, ROJO_T"execvp: %s\n"RESET, strerror(errno));
            exit(-1);
        }
        exit(0);
    }
    else if (id > 0)
    {
        #if DEBUGN3
            fprintf(stderr, GRIS_T"[execute_line()→ PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
            fprintf(stderr, GRIS_T"[execute_line()→ PID hijo: %d (%s)]\n"RESET, getpid(), jobs_list[0].cmd);
        #endif
        
        jobs_list[0].pid = id;
    }
    else
    {
        fprintf(stderr, ROJO_T"fork: %s\n"RESET, strerror(errno));
        exit(-2);
    }

    
    wait(&status);
    if (WIFEXITED(status))
    {
        int statusAux = WEXITSTATUS(status);
        if (statusAux == 0)
        {
            #if DEBUGN3
                fprintf(stderr, GRIS_T"[execute_line(): Proceso hijo %d (%s) finalizado con exit(), status: %d]\n"RESET, jobs_list[0].pid, jobs_list[0].cmd, status);
            #endif
        }
        else
        {
            #if DEBUGN3
                fprintf(stderr, ROJO_T"%s: no se encontró la orden\n"RESET, jobs_list[0].cmd);
                fprintf(stderr, GRIS_T"[execute_line(): Proceso hijo %d (%s) finalizado con exit(), status: %d]\n"RESET, jobs_list[0].pid, jobs_list[0].cmd, statusAux);
            #endif
        }
    }
    
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

    return EXITO;
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
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
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


    #if DEBUGN2
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
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int internal_export(char **args)
{
    char *nombre = strtok(args[1], "="), *valor = strtok(NULL, " ");

    if ((nombre != NULL) && (valor != NULL))
    {
        #if DEBUGN2
            fprintf(stderr, GRIS_T"[internal_export()→ nombre: %s]\n"RESET, nombre);
            fprintf(stderr, GRIS_T"[internal_export()→ valor: %s]\n"RESET, valor);
        #endif

        if (getenv(nombre) == NULL)
        {
            fprintf(stderr, ROJO_T"getenv: %s\n"RESET, strerror(errno));
        }
        else
        {
            #if DEBUGN2
                fprintf(stderr, GRIS_T"[internal_export()→ antiguo valor para %s: %s]\n"RESET, nombre, getenv(nombre));
            #endif

            setenv(nombre, valor, 1);
            
            #if DEBUGN2
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
        #if DEBUGN2
            fprintf(stderr, GRIS_T"[internal_export()→ nombre: %s]\n"RESET, nombre);
            fprintf(stderr, GRIS_T"[internal_export()→ valor = (null)]\n"RESET);
        #endif
        
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: export Nombre=Valor\n"RESET);
    }
    
    return FALLO;
}


/**
 * Ejecuta un fichero de la línea de comandos.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int internal_source(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: source <nombre_fichero>\n"RESET);
        return FALLO;
    }

    char linea[COMMAND_LINE_SIZE];
    FILE *fic = fopen(args[1], "r");
    if (fic == NULL)
    {
        fprintf(stderr, ROJO_T"fopen: %s\n"RESET, strerror(errno));
        return FALLO;
    }

    while (fgets(linea, COMMAND_LINE_SIZE, fic) != NULL)
    {
        for (int i=0; i<COMMAND_LINE_SIZE; i++)
        {
            if (linea[i] == '\n')
            {
                linea[i] = '\0';
            }
        }

        #if DEBUGN3
            fprintf(stderr, GRIS_T"[internal_source()→ LINE: %s]\n"RESET, linea);
        #endif
        
        fflush(fic);
        execute_line(linea);
      
    }

    if (fclose(fic) < 0)
    {
        fprintf(stderr, ROJO_T"fclose: %s\n"RESET, strerror(errno));
        return FALLO;
    }

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


/*****************************************************************************/


/**
 * Función auxiliar que permite tratar un directorio pasado entre comillas
 * dobles, simple, y el carácter \, permitiendo acceder a directorios que
 * contienen un espacio (" ") en su nombre.
 * 
 * @param args: Tokens de la línea de comandos.
 * @return: EXITO tras la correcta ejecución, FALLO de lo contrario.
 */
int cd_avanzado(char **args)
{
    char *token = args[1], *tokenAux;
    char comillas;
    int i, nTokensAux;

    if (strchr(token, 92) != NULL)          /*92 es \ en ASCII*/
    {
        if ((tokenAux = args[2]) == NULL)
        {
            token[strlen(token)-1] = ' ';
            return EXITO;
        }
        
        nTokensAux = 3;
        while ((nTokensAux <= n_tokens) && (strchr(tokenAux, 92) != NULL))
        {
            token[strlen(token)] = ' ';
            tokenAux = args[nTokensAux++];
        }
        token[strlen(token)] = ' ';
        
        int aux = 0;
        for (i=0; i<(strlen(token)-n_tokens+2); i++)
        {
            if (token[i+aux] == 92) aux++;
            token[i] = token[i+aux];
        }
        token[i] = '\0';
        
        return EXITO;

    }
    else if (strchr(token, 34) != NULL)     /*34 es " en ASCII*/
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


    if (token[strlen(token)-1] == comillas)
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
        fprintf(stderr, ROJO_T"cd_avanzado: Faltan comillas al cerrar\n"RESET);
        return FALLO;
    }

    nTokensAux = 3;
    while ((nTokensAux <= n_tokens) && (strchr(tokenAux, comillas) == NULL))
    {
        token[strlen(token)] = ' ';
        tokenAux = args[nTokensAux++];

        if (nTokensAux > n_tokens)
        {
            fprintf(stderr, ROJO_T"cd avanzado: Faltan comillas al cerrar\n"RESET);
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
