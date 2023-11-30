/* @author Daniel Giralt Pascual */

#include "my_shell.h"
static int n_tokens, n_pids;
static char mi_shell[COMMAND_LINE_SIZE]; 
static struct info_job jobs_list [N_JOBS];


/**
 * Main del programa.
 */
int main(int argc, char *argv[])
{
    signal(SIGCHLD, reaper);
    signal(SIGINT, ctrlc);
    signal (SIGTSTP, ctrlz);
    
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
    #if DEBUG1 || DEBUG2 || DEBUG_MY_SHELL
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


    int isBackground = is_background(args);

    strcpy(jobs_list[0].cmd, line);
    jobs_list[0].status = 'E';

    pid_t id = fork();
    if (id == 0)        //Hijo
    {
        signal(SIGCHLD, SIG_DFL);
        signal(SIGINT, SIG_IGN);
        signal(SIGTSTP, SIG_IGN);

        int output_redirection = is_output_redirection(args);
        if (output_redirection < 0)
        {
            return FALLO;
        }
        
        if (execvp(args[0], args) < 0)
        {
            fprintf(stderr, ROJO_T"execvp: %s\n"RESET, strerror(errno));
            exit(-1);
        }

        if (output_redirection > 0)
        {
            exit(-1);
        }

        exit(0);
    }
    else if (id > 0)    //Padre
    {
        #if DEBUG4 || DEBUG5
            fprintf(stderr, GRIS_T"[execute_line(): PID padre: %d (%s)]\n"RESET, getpid(), mi_shell);
            fprintf(stderr, GRIS_T"[execute_line(): PID hijo: %d (%s)]\n"RESET, id, line);
        #endif

        if (isBackground == 0)
        {
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
            jobs_list_add(id, 'E', line);
            fprintf(stderr, "[%d] %d\t%c\t%s\n", n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
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
    for(int i=1; i<=n_pids; i++)
    {
        fprintf(stderr, "[%d] %d\t%c\t%s\n", i, jobs_list[i].pid, jobs_list[i].status, jobs_list[i].cmd);
    }

    return EXITO;
}


/**
 * Lleva los procesos más recientes a primer plano.
 */
int internal_fg(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: fg <nº de trabajo>\n"RESET);
        return FALLO;
    }

    int pos = atoi(args[1]);
    if ((pos > n_pids) || (pos <= 0))
    {
        fprintf(stderr, ROJO_T"fg %d: no existe ese trabajo\n"RESET, pos);
        return FALLO;
    }

    if (jobs_list[pos].status == 'D')
    {
        kill(jobs_list[pos].pid, SIGCONT);
    }

    #if DEBUG6
        fprintf(stderr, GRIS_T"[internal_fg()→ Señal 18 (SIGCONT) enviada a %d (%s)]\n"RESET, jobs_list[pos].pid, jobs_list[pos].cmd);
    #endif

    jobs_list[pos].status = 'E';

    for (int i=0; i<strlen(jobs_list[pos].cmd); i++)
    {
        if (jobs_list[pos].cmd[i] == '&')
        {
            jobs_list[pos].cmd[i-1] = '\0';
        }
    }

    jobs_list[0] = jobs_list[pos];
    jobs_list_remove(pos);

    fprintf(stderr, "%s\n", jobs_list[0].cmd);
    
    while (jobs_list[0].pid > 0)
    {
        pause();
    }

    return EXITO;
}


/**
 * Enseña los procesos parados o en segundo plano.
 */
int internal_bg(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, ROJO_T"Error de sintaxis. Uso: bg <nº de trabajo>\n"RESET);
        return FALLO;
    }

    int pos = atoi(args[1]);
    if ((pos > n_pids) || (pos <= 0))
    {
        fprintf(stderr, ROJO_T"bg %d: no existe ese trabajo\n"RESET, pos);
        return FALLO;
    }

    if (jobs_list[pos].status == 'E')
    {
        fprintf(stderr, ROJO_T"bg %d: el trabajo ya está en segundo plano\n"RESET, pos);
        return FALLO;
    }

    jobs_list[pos].status = 'E';
    strcat(jobs_list[pos].cmd, " &\0");
    kill(jobs_list[pos].pid,SIGCONT);

    #if DEBUG6
        fprintf(stderr, GRIS_T"[internal_bg()→ Señal 18 (SIGCONT) enviada a %d (%s)]\n"RESET, jobs_list[pos].pid, jobs_list[pos].cmd);
    #endif
    
    fprintf(stderr,"[%d] %d\t%c\t%s\n", pos, jobs_list[pos].pid, jobs_list[pos].status, jobs_list[pos].cmd);   
    
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
        #if DEBUG5
            fprintf(stderr, GRIS_T"\n[reaper()→ recibida señal %d (SIGCHLD)]\n"RESET, signum);
        #endif  

        if (ended == jobs_list[0].pid)
        {
            if (WEXITSTATUS(status) != 0)
            {
                fprintf(stderr, ROJO_T"%s: no se encontró la orden\n"RESET, jobs_list[0].cmd);
            }
            
            #if DEBUG4 || DEBUG5
                if (WIFSIGNALED(status))
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) en foreground finalizado por la señal %d]\n"RESET, ended, jobs_list[0].cmd, status);
                }
                else
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) en foreground finalizado con exit code 0]\n"RESET, ended, jobs_list[0].cmd);
                }
            #endif

            jobs_list[0].pid = 0;
            jobs_list[0].status = 'F';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        }
        else
        {
            if (WEXITSTATUS(status) != 0)
            {
                fprintf(stderr, ROJO_T"%s: no se encontró la orden\n"RESET, jobs_list[0].cmd);
            }

            int pos;
            if ((pos = jobs_list_find(ended)) < 0)
            {
                fprintf(stderr, ROJO_T"ERROR buscando la posición del trabajo\n"RESET);
            }

            #if DEBUG4 || DEBUG5
                if (WIFSIGNALED(status))
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) en background finalizado por la señal %d]\n"RESET, ended, jobs_list[pos].cmd, status);
                }
                else
                {
                    fprintf(stderr, GRIS_T"[reaper()→ Proceso hijo %d (%s) en background finalizado con exit code 0]\n"RESET, ended, jobs_list[pos].cmd);
                }
            #endif

            fprintf(stderr, "\nTerminado PID %d (%s) en job_list[%d] con status %d\n", ended, jobs_list[pos].cmd, pos, status);
            
            jobs_list_remove(pos);
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
    
    #if DEBUG4 || DEBUG5
        fprintf(stderr, GRIS_T"\n[ctrlc()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n"RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
        fprintf(stderr, GRIS_T"[ctrlc()→ recibida señal %i (SIGINT)]\n"RESET, signum);
    #endif

    if (jobs_list[0].pid > 0)
    {
        if (strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            kill(jobs_list[0].pid, SIGTERM);
            
            #if DEBUG4 || DEBUG5
                fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 (SIGTERM) enviada a %d (%s) por %d (%s)]"RESET, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            #endif

        }
        else
        {
            #if DEBUG4 || DEBUG5
                fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 (SIGTERM) no enviada a %d (%s) debido a que su proceso en foreground es el shell]"RESET, getpid(), mi_shell);
            #endif
        }
    }
    else
    {
        #if DEBUG4 || DEBUG5
            fprintf(stderr, GRIS_T"[ctrlc()→ Señal 15 (SIGTERM) no enviada por %d (%s) debido a que no hay proceso en foreground]"RESET, getpid(), mi_shell);
        #endif
    }
    
    fprintf(stderr, "\n");
    fflush(stdout);
}


/**
 * Manejador propio para la señal SIGSTP (Ctrl+Z).
 */
void ctrlz(int signum)
{
    signal(SIGTSTP, ctrlz);

    #if DEBUG5
        fprintf(stderr, GRIS_T"\n[ctrlz()→ Soy el proceso con PID %d (%s), el proceso en foreground es %d (%s)]\n"RESET, getpid(), mi_shell, jobs_list[0].pid, jobs_list[0].cmd);
        fprintf(stderr, GRIS_T"[ctrlz()→ recibida señal %i (SIGTSTP)]\n"RESET, signum);
    #endif

    if(jobs_list[0].pid > 0)
    {
        if(strcmp(jobs_list[0].cmd, mi_shell) != 0)
        {
            kill(jobs_list[0].pid, SIGSTOP);

            #if DEBUG5
                fprintf(stderr, GRIS_T"[ctrlz()→ Señal 19 (SIGSTOP) enviada a %d (%s) por %d (%s)]"RESET, jobs_list[0].pid, jobs_list[0].cmd, getpid(), mi_shell);
            #endif

            jobs_list[0].status = 'D';
            jobs_list_add(jobs_list[0].pid, jobs_list[0].status, jobs_list[0].cmd);
            
            fprintf(stderr, "\n[%d] %d\t%c\t%s", n_pids, jobs_list[n_pids].pid, jobs_list[n_pids].status, jobs_list[n_pids].cmd);
            
            jobs_list[0].pid = 0;
            jobs_list[0].status = 'N';
            memset(jobs_list[0].cmd, '\0', COMMAND_LINE_SIZE);
        }
        else
        {
            #if DEBUG5
                fprintf(stderr, GRIS_T"[ctrlz()→ Señal 19 (SIGSTOP) no enviada a %d (%s) debido a que su proceso en foreground es el shell]"RESET, getpid(), mi_shell);
            #endif
        }
    }
    else
    { 
        #if DEBUG5
            fprintf(stderr,GRIS_T"[ctrlz()→ Señal 19 (SIGSTOP) no enviada por %d (%s) debido a que no hay proceso en foreground]"RESET, getpid(), mi_shell);
        #endif
    }

    fprintf(stderr, "\n");
    fflush(stdout);
}


/**
 * Comprueba si un trabajo está en "background" o "foreground".
 */
int is_background(char **args)
{
    int nTokensAux = n_tokens;
    for (int i=0; i<nTokensAux; i++)
    {
        if (strcmp(args[i], "&") == 0)
        {
            args[i] = NULL;
            n_tokens--;
            return FALLO;
        }
    }
    
    return EXITO;
}



/**
 * Añade un nuevo trabajo a la lista de trabajos.
 */
int jobs_list_add(pid_t pid,char status, char *cmd)
{
    n_pids++;
    
    if (n_pids >= N_JOBS)
    {
        fprintf(stderr, ROJO_T"ERROR: Número máximo de trabajos asolidos\n"RESET);
        return FALLO;
    }

    jobs_list[n_pids].status = status;
    jobs_list[n_pids].pid = pid;
    strcpy(jobs_list[n_pids].cmd, cmd);
    
    return EXITO;
}


/**
 * Comprueba si el PID de un trabajo se encuentra en la lista.
 */
int jobs_list_find(pid_t pid)
{
    for(int i=1; i<N_JOBS; i++)
    {
        if (jobs_list[i].pid == pid)
        {
            return i;
        }
    }

    return FALLO;
}


/**
 * Elimina un trabajo de una posición en específico.
 */
int  jobs_list_remove(int pos)
{
    jobs_list[pos] = jobs_list[n_pids];
    
    jobs_list[n_pids].pid = 0;
    jobs_list[n_pids].status = '\0';
    memset(jobs_list[n_pids].cmd, '\0', COMMAND_LINE_SIZE);
    
    n_pids--;
    return EXITO;
}



/**
 * Elimina un trabajo de la en específico.
 */
int is_output_redirection(char **args)
{
    for (int i=0; i<n_tokens; i++)
    {
        if (strcmp(args[i], ">") == 0)
        {
            args[i] = NULL;
            
            int fd = open(args[i+1],  O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
            if (fd < 0)
            {
                fprintf(stderr, ROJO_T"open: %s\n"RESET, strerror(errno));
                return FALLO;
            }

            if (dup2(fd, 1) < 0)
            {
                fprintf(stderr, ROJO_T"dup2: %s\n"RESET, strerror(errno));
                return FALLO;
            }

            if (close(fd) < 0)
            {
                fprintf(stderr, ROJO_T"close: %s\n"RESET, strerror(errno));
                return FALLO;
            }

            return EXITO;
        }
    }

    return EXITO;
}
