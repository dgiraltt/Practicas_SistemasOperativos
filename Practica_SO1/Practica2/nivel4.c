/* @author Daniel Giralt Pascual */

#include "nivel4.h"
static int n_tokens;
static char mi_shell[COMMAND_LINE_SIZE]; 
static struct info_job jobs_list [N_JOBS];


/**
 * Main del programa.
 */
int main(int argc, char *argv[])
{
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    
    jobs_list[0].pid = 0;
    jobs_list[0].status = 'N';
    memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);

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
    #if DEBUG1 || DEBUG2
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
    char lineAux[strlen(line)+1];
    strcpy(lineAux, line);
    
    char *args[ARGS_SIZE];
    if ((n_tokens = parse_args(args, lineAux)) < 1)
    {
        return FALLO;
    }

    if (check_internal(args) == 0)  //Comando interno
    {
        return EXITO;
    }


    strcpy(jobs_list[0].cmd, line);
    jobs_list[0].status = 'E';

    pid_t id = fork();
    if (id == 0)        //Hijo
    {
        signal(SIGCHLD, SIG_DFL);
        signal(SIGINT, SIG_IGN);

        if (execvp(args[0], args) < 0)
        {
            fprintf(stderr, ROJO_T"execvp: %s\n"RESET, strerror(errno));
            exit(-1);
        }

        exit(0);
    }
    else if (id > 0)    //Padre
    {
        #if DEBUG4
            fprintf(stderr, GRIS_T"[execute_line(): PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
            fprintf(stderr, GRIS_T"[execute_line(): PID hijo: %d (%s)]\n"RESET, id, line);
        #endif
        
        jobs_list[0].pid = id;
        strcpy(jobs_list[0].cmd, line);
        jobs_list[0].status = 'E';

        while (jobs_list[0].pid > 0)
        {
            pause();
        }
    }
    else
    {
        fprintf(stderr, ROJO_T"fork: %s\n"RESET, strerror(errno));
        exit(-1);
    }

    return EXITO;
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

        #if DEBUG3
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
 */
int internal_jobs(char **args)
{
    printf(GRIS_T "[internal_jobs()→ Esta función mostrará el PID de los procesos que no estén en foreground]\n" RESET);
    return EXITO;
}


/**
 * Lleva los procesos más recientes a primer plano.
 */
int internal_fg(char **args)
{
    printf(GRIS_T "[internal_fg()→ Esta función lleva los procesos más recientes a primer plano]\n" RESET);
    return EXITO;
}


/**
 * Enseña los procesos parados o en segundo plano.
 */
int internal_bg(char **args)
{
    printf(GRIS_T "[internal_bg()→ Esta función enseña los procesos parados o en segundo plano]\n" RESET);
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
        fprintf(stderr, "ERROR: Faltan comillas\n");
        return FALLO;
    }


    int nTokensAux = 3;
    while ((nTokensAux <= n_tokens) && (strchr(tokenAux, comillas) == NULL))
    {
        token[strlen(token)] = ' ';
        tokenAux = args[nTokensAux++];

        if ((nTokensAux > n_tokens))
        {
            fprintf(stderr, "ERROR: Faltan comillas\n");
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


/**
 * Manejador propio para la señal SIGCHLD.
 */
void reaper(int signum)
{
    signal(SIGCHLD, reaper);
    pid_t ended;
    int status;
    
    while ((ended = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, ROJO_T"%s: no se encontró la orden\n"RESET, jobs_list[0].cmd);
        }
        
        if (ended == jobs_list[0].pid)
        {
            #if DEBUG4
                if (WIFSIGNALED(status))
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) finalizado por la señal %d]\n"RESET, ended, jobs_list[0].cmd, status);
                }
                else
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) finalizado con exit code 0]\n"RESET, ended, jobs_list[0].cmd);
                }
            #endif

            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        }
    }

    fflush(stdout);
}


/**
 * Manejador propio para la señal SIGINT (Ctrl+C).
 */
void ctrlc(int signum)
{
    signal(SIGINT, ctrlc);
    
    #if DEBUG4
        fprintf(stderr, GRIS_T"\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n"RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
    #endif

    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            kill(jobs_list[0].pid, SIGTERM);
            
            #if DEBUG4
                fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 enviada a %d (%s) por %d (%s)]"RESET, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            #endif

        }
        else
        {
            #if DEBUG4
                fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 no enviada a %d (%s) debido a que su proceso en foreground es el shell]"RESET, getpid(), mi_shell);
            #endif
        }
    }
    else
    {
        #if DEBUG4
            fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 no enviada por %d (%s) debido a que no hay proceso en foreground]"RESET, getpid(), mi_shell);
        #endif
    }

    fprintf(stderr, "\n");
    fflush(stdout);
}
