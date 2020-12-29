#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <termios.h>
#include <limits.h>
#include <dirent.h> 
#include <fcntl.h>

#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_COMMAND_LEN 25  // sets the max possible length of one single command like ls -l
#define CREATE_FLAGS (O_WRONLY | O_CREAT | O_APPEND)
#define CREATE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/************************************ Datastructures ************************************/

/* A process is a single process.  */
typedef struct process
{
  struct process *next;       /* next process in pipeline */
  char **argv;                /* for exec */
  pid_t pid;                  /* process ID */
  char completed;             /* true if process has completed */
  char stopped;               /* true if process has stopped */
  int status;                 /* reported status value */
} process;

/* A job is a pipeline of processes.  */
typedef struct job
{
  struct job *next;           /* next active job */
  char *command;              /* command line, used for messages */
  process *first_process;     /* list of processes in this job */
  pid_t pgid;                 /* process group ID */
  char notified;              /* true if user told about stopped job */
  struct termios tmodes;      /* saved terminal modes */
  int in, out, err;  /* standard i/o channels */
} job;

/* The active jobs are linked into a list.  This is its head.   */
job *first_job = NULL;

process* create_process(process* next, char **argv)
{
    process* new_node = (process*)malloc(sizeof(process));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    new_node->argv = argv;
    new_node->next = next;
 
    return new_node;
}

process* append_process(process* head, process *p)
{
    /* go to the last node */
    process *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;
 
    /* create a new node */
    //process* new_node =  create_process(NULL,argv);
    process* new_node = p;
    cursor->next = new_node;
 
    return head;
}

job* create_job(job* next, process* first_process, int in, int out, int err)
{
    job* new_node = (job*)malloc(sizeof(job));
    if(new_node == NULL)
    {
        printf("Error creating a new job.\n");
        exit(0);
    }
    new_node->in = in;
    new_node->out = out;
    new_node->err = err;
    new_node->next = next;
    new_node->first_process = first_process;
    return new_node;
}

void append_job(job* head, job *j)
{
    /* go to the last node */
    job *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;
 
    /* create a new node */
    //job* new_node =  create_job(NULL, first_process, in, out, err);
    job* new_node = j;
    cursor->next = new_node;
 
    //return head;
}


/* Find the active job with the indicated pgid.  */
job *
find_job (pid_t pgid)
{
  job *j;

  for (j = first_job; j; j = j->next)
    if (j->pgid == pgid)
      return j;
  return NULL;
}

/* Return true if all processes in the job have stopped or completed.  */
int
job_is_stopped (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed && !p->stopped)
      return 0;
  return 1;
}

/* Return true if all processes in the job have completed.  */
int
job_is_completed (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    if (!p->completed)
      return 0;
  return 1;
}

/* Keep track of attributes of the shell.  */

pid_t shell_pgid;
struct termios shell_tmodes;
int shell_terminal;
int shell_is_interactive;

/*************************************** Shell ***************************************/

/* Make sure the shell is running interactively as the foreground job
   before proceeding. */

void
init_shell ()
{

  /* See if we are running interactively.  */
  shell_terminal = STDIN_FILENO;
  shell_is_interactive = isatty (shell_terminal);

  if (shell_is_interactive)
    {
      /* Loop until we are in the foreground.  */
      while (tcgetpgrp (shell_terminal) != (shell_pgid = getpgrp ()))
        kill (- shell_pgid, SIGTTIN);

      /* Ignore interactive and job-control signals.  */
      signal (SIGINT, SIG_IGN);
      signal (SIGQUIT, SIG_IGN);
      signal (SIGTSTP, SIG_IGN);
      signal (SIGTTIN, SIG_IGN);
      signal (SIGTTOU, SIG_IGN);
      signal (SIGCHLD, SIG_IGN);

      /* Put ourselves in our own process group.  */
      shell_pgid = getpid ();
      if (setpgid (shell_pgid, shell_pgid) < 0)
        {
          perror ("Couldn't put the shell in its own process group");
          exit (1);
        }

      /* Grab control of the terminal.  */
      tcsetpgrp (shell_terminal, shell_pgid);

      /* Save default terminal attributes for shell.  */
      tcgetattr (shell_terminal, &shell_tmodes);
    }
}

int
mark_process_status (pid_t pid, int status)
{
  job *j;
  process *p;

  if (pid > 0)
    {
      /* Update the record for the process.  */
      for (j = first_job; j; j = j->next)
        for (p = j->first_process; p; p = p->next)
          if (p->pid == pid)
            {
              p->status = status;
              if (WIFSTOPPED (status))
                p->stopped = 1;
              else
                {
                  p->completed = 1;
                  if (WIFSIGNALED (status))
                    fprintf (stderr, "%d: Terminated by signal %d.\n",
                             (int) pid, WTERMSIG (p->status));
                }
              return 0;
             }
      fprintf (stderr, "No child process %d.\n", pid);
      return -1;
    }
  else if (pid == 0 || errno == ECHILD)
    /* No processes ready to report.  */
    return -1;
  else {
    /* Other weird errors.  */
    perror ("waitpid");
    return -1;
  }
}

/* Check for processes that have status information available,
   without blocking.  */

void
update_status (void)
{
  int status;
  pid_t pid;

  do
    pid = waitpid (WAIT_ANY, &status, WUNTRACED|WNOHANG);
  while (!mark_process_status (pid, status));
}

/* Check for processes that have status information available,
   blocking until all processes in the given job have reported.  */

void
wait_for_job (job *j)
{
  int status;
  pid_t pid;

  do
    pid = waitpid (WAIT_ANY, &status, WUNTRACED);
  while (!mark_process_status (pid, status)
         && !job_is_stopped (j)
         && !job_is_completed (j));
}

/* Format information about job status for the user to look at.  */

void
format_job_info (job *j, const char *status)
{
  fprintf (stderr, "%ld (%s): %s\n", (long)j->pgid, status, j->command);
}

/* Notify the user about stopped or terminated jobs.
   Delete terminated jobs from the active job list.  */

void
do_job_notification (void)  // ps_all
{
  job *j, *jlast, *jnext;

  /* Update status information for child processes.  */
  update_status ();

  jlast = NULL;
  for (j = first_job; j; j = jnext)
    {
      jnext = j->next;

      /* If all processes have completed, tell the user the job has
         completed and delete it from the list of active jobs.  */
      if (job_is_completed (j)) {
        format_job_info (j, "completed");
        if (jlast)
          jlast->next = jnext;
        else
          first_job = jnext;
        //free_job (j);  // tbd delete from list
      }

      /* Notify the user about stopped jobs,
         marking them so that we won’t do this more than once.  */
      else if (job_is_stopped (j) && !j->notified) {
        format_job_info (j, "stopped");
        j->notified = 1;
        jlast = j;
      }

      /* Don’t say anything about jobs that are still running.  */
      else
        jlast = j;
    }
}

/* Put job j in the foreground.  If cont is nonzero,
   restore the saved terminal modes and send the process group a
   SIGCONT signal to wake it up before we block.  */

void
put_job_in_foreground (job *j, int cont)
{
  /* Put the job into the foreground.  */
  tcsetpgrp (shell_terminal, j->pgid);

  /* Send the job a continue signal, if necessary.  */
  if (cont)
    {
      tcsetattr (shell_terminal, TCSADRAIN, &j->tmodes);
      if (kill (- j->pgid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }

  /* Wait for it to report.  */
  wait_for_job (j);

  /* Put the shell back in the foreground.  */
  tcsetpgrp (shell_terminal, shell_pgid);

  /* Restore the shell’s terminal modes.  */
  tcgetattr (shell_terminal, &j->tmodes);
  tcsetattr (shell_terminal, TCSADRAIN, &shell_tmodes);
}

/* Put a job in the background.  If the cont argument is true, send
   the process group a SIGCONT signal to wake it up.  */

void
put_job_in_background (job *j, int cont)
{
  /* Send the job a continue signal, if necessary.  */
  if (cont)
    if (kill (-j->pgid, SIGCONT) < 0)
      perror ("kill (SIGCONT)");
}

void
mark_job_as_running (job *j)
{
  process *p;

  for (p = j->first_process; p; p = p->next)
    p->stopped = 0;
  j->notified = 0;
}

/* Continue the job J.  */

void
continue_job (job *j, int foreground)
{
  mark_job_as_running (j);
  if (foreground)
    put_job_in_foreground (j, 1);
  else
    put_job_in_background (j, 1);
}

char* find_given_command(char* args[], int background) {
    int i = 0;
	char *ch, ampersand[] = "&";
	if (background){
		while ( &ch != NULL){
			//printf("%d\n", i);
			ch = args[i];
			//printf("%s\n", ch);
			if (!strcmp(ch,ampersand)){
				//printf("hello we are in!!! -- %s-- \n", ampersand);
				break;
			}
			i++;
		}
		args[i] = '\0';
	}
	char* path = getenv("PATH");
	FILE* file;
	char* token = strtok(path, ":");
	char* my_command = args[0];
	char* filename;
	int is_exists = 0;
	while (token != NULL) {
		filename = malloc(strlen(token) + strlen(my_command) + 2);
		strcpy(filename, token);
		strcat(filename, "/");
		strcat(filename, my_command);

		if (file = fopen(filename, "r")) {
			printf("Your file is here %s\n", filename);
			is_exists = 1;
			return filename;
		}
		token = strtok(NULL, ":");
	}
}

void
launch_process (process *p, pid_t pgid,
                int infile, int outfile, int errfile,
                int foreground)
{
  pid_t pid;

  if (shell_is_interactive)
    {
      /* Put the process into the process group and give the process group
         the terminal, if appropriate.
         This has to be done both by the shell and in the individual
         child processes because of potential race conditions.  */
      pid = getpid ();
      if (pgid == 0) pgid = pid;
      setpgid (pid, pgid);
      if (foreground)
        tcsetpgrp (shell_terminal, pgid);

      /* Set the handling for job control signals back to the default.  */
      signal (SIGINT, SIG_DFL);
      signal (SIGQUIT, SIG_DFL);
      signal (SIGTSTP, SIG_DFL);
      signal (SIGTTIN, SIG_DFL);
      signal (SIGTTOU, SIG_DFL);
      signal (SIGCHLD, SIG_DFL);
    }

  /* Set the standard input/output channels of the new process.  */
  if (infile != STDIN_FILENO)
    {
      dup2 (infile, STDIN_FILENO);
      close (infile);
    }
  if (outfile != STDOUT_FILENO)
    {
      dup2 (outfile, STDOUT_FILENO);
      close (outfile);
    }
  if (errfile != STDERR_FILENO)
    {
      dup2 (errfile, STDERR_FILENO);
      close (errfile);
    }

  /* Exec the new process.  Make sure we exit.  */
  // path = find_given_command_path(args);
  // printf("the Path is : %s\n", path);
  // execv(path, args);
  char *path;
  path = find_given_command(p->argv, !foreground);

  execv(path, p->argv);  // tbd replace with execv and call find command
  perror ("execv");
  exit (1);
}

void
launch_job (job *j, int foreground)
{
  process *p;
  pid_t pid;
  int mypipe[2], infile, outfile;

  infile = j->in;
  for (p = j->first_process; p; p = p->next)
    {
      /* Set up pipes, if necessary.  */
      if (p->next)
        {
          if (pipe (mypipe) < 0)
            {
              perror ("pipe");
              exit (1);
            }
          outfile = mypipe[1];
        }
      else
        outfile = j->out;

      /* Fork the child processes.  */
      pid = fork ();
      if (pid == 0)
        /* This is the child process.  */
        launch_process (p, j->pgid, infile,
                        outfile, j->err, foreground);
      else if (pid < 0)
        {
          /* The fork failed.  */
          perror ("fork");
          exit (1);
        }
      else
        {
          /* This is the parent process.  */
          p->pid = pid;
          if (shell_is_interactive)
            {
              if (!j->pgid)
                j->pgid = pid;
              setpgid (pid, j->pgid);
            }
        }

      /* Clean up after pipes.  */
      if (infile != j->in)
        close (infile);
      if (outfile != j->out)
        close (outfile);
      infile = mypipe[0];
    }

  format_job_info (j, "launched");

  if (!shell_is_interactive)
    wait_for_job (j);
  else if (foreground)
    put_job_in_foreground (j, 0);
  else
    put_job_in_background (j, 0);
}

/*********************************************************************************************/
/*************************************** Main & Others ***************************************/
/*********************************************************************************************/

void setup(char inputBuffer[], char *args[],int *background){
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct;     /* index of where to place the next parameter into args[] */
    
    ct = 0;
        
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    start = -1;
    if (length == 0)
        exit(0);            /* ^d was entered, end of user command stream */

    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
        exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<",inputBuffer);
        for (i=0;i<length;i++){ /* examine every character in the inputBuffer */

            switch (inputBuffer[i]){
            case ' ':
            case '\t' :               /* argument separators */
                if(start != -1){
                    args[ct] = &inputBuffer[start];    /* set up pointer */
                    ct++;
                }
                inputBuffer[i] = '\0'; /* add a null char; make a C string */
                start = -1;
                break;

            case '\n':                 /* should be the final char examined */
                if (start != -1){
                    args[ct] = &inputBuffer[start];     
                    ct++;
                }
                inputBuffer[i] = '\0';
                args[ct] = NULL; /* no more arguments to this command */
                break;

            default :             /* some other character */
                if (start == -1)
                    start = i;
                if (inputBuffer[i] == '&'){
                    *background  = 1;
                    inputBuffer[i-1] = '\0';
                }
            } /* end of switch */
        }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++)
		printf("args %d = %s\n",i,args[i]);
} /* end of setup routine */

/*************************************** Search ***************************************/

void searchSubDir(const char *name, int indent, char *str)
{
    DIR *dir;
    struct dirent *entry;
    char* filename;
    FILE* fp;                   
    int flag = 0;
	int line_num;
	int find_result = 0;
	char temp[512];
    char cwd[PATH_MAX];

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) { // if it is a dirctory
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            // printf("%*s[%s]\n", indent, "", entry->d_name);
            searchSubDir(path, indent + 2, str);
        } else { //if it is a file
            // printf("%*s- %s\n", indent, "", entry->d_name);
            if ((entry->d_name[strlen(entry->d_name) - 1] == 'h') ||  
                (entry->d_name[strlen(entry->d_name) - 1] == 'c') ||
                (entry->d_name[strlen(entry->d_name) - 1] == 'H') || 
                (entry->d_name[strlen(entry->d_name) - 1] == 'C')) {
                filename = entry->d_name;
                // printf("%s\n", filename);
                getcwd(cwd, sizeof(cwd));
                // printf("cwd : s%s\n", cwd);
                // printf("filename : s%s\n", filename);
                strcat(cwd, "/");
                strcat(cwd, filename);
                // printf("searchin in %s\n", filename);
                fp = fopen(filename, "r");

                if (fp == NULL) {
                    printf("Error Occured.\n");
                    exit(1);
                }
                line_num = 1;
                while(fgets(temp, 512, fp) != NULL) {
                    if((strstr(temp, str)) != NULL) {
                        flag = 1;
                        printf("\t%d: ./%s -> %s", line_num, filename, temp);
                        find_result++;
                    }
                    line_num++;
                }
                //Close the file if still open.
                if(fp) {
                    fclose(fp);
                }
            } // end of if
        }
    }
    closedir(dir);
    if(flag == 0) {
        printf("No Match Found in this directory.\n");
    }
}

int search(char *string, int R){ 
    // printf("in search function\n");
    // TODO extensive commenting 
    // TODO error checking 
    char* filename;
    DIR *d, *sd;
    struct dirent *dir;
    FILE* fp;                   
    int flag = 0;
	int line_num;
	int find_result = 0;
	char temp[512];
    d = opendir(".");
    char cwd[PATH_MAX];
    
    if (R == 0) { // run with option -r
        searchSubDir(".", 0, string);
    } else { // run without option -r
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if ((dir->d_name[strlen(dir->d_name) - 1] == 'h') ||  
                    (dir->d_name[strlen(dir->d_name) - 1] == 'c') ||
                    (dir->d_name[strlen(dir->d_name) - 1] == 'H') || 
                    (dir->d_name[strlen(dir->d_name) - 1] == 'C')) {
                    filename = dir->d_name;
                    // printf("%s\n", filename);
                    getcwd(cwd, sizeof(cwd));
                    // printf("cwd : s%s\n", cwd);
                    // printf("filename : s%s\n", filename);
                    strcat(cwd, "/");
                    strcat(cwd, filename);
                    // printf("searchin in %s\n", filename);
                    fp = fopen(filename, "r");

                    if (fp == NULL) {
                        printf("Error Occured.\n");
                        return 1;
                    }
                    line_num = 1;
                    while(fgets(temp, 512, fp) != NULL) {
                        if((strstr(temp, string)) != NULL) {
                            flag = 1;
                            printf("\t%d: ./%s -> %s", line_num, filename, temp);
                            find_result++;
                        }
                        line_num++;
                    }
                    //Close the file if still open.
                    if(fp) {
                        fclose(fp);
                    }
                } // end of if
            } // end of while
            closedir(d);
        } // end of if d
        if(flag == 0) {
            printf("No Match Found in this directory.\n");
        }
    } // end of if option -r
}

/*************************************** Main ***************************************/

void execute_command(char* args[], int background){
    int i = 0; // count = 0, j= 0; 
    char *ch = "";
    char *tempBuffer[MAX_COMMAND_LEN] = {0};  // ls -l | wc -l < infile >> outfile
    process *p = NULL; // first process
    job *j; 
    while (args[i] != NULL){
        ch = args[i];
        // if not | or << or < or > or >> or 2>
        if (!strcmp(ch, "|")){
            char *copyOfTemp;
            memcpy(copyOfTemp, &tempBuffer, MAX_COMMAND_LEN);
            process *new_process = create_process(NULL, copyOfTemp);
            if (p == NULL)
                p = new_process;
            else
                append_process(p, new_process);
            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else if (!strcmp(ch, "<<")){

            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else if (!strcmp(ch, "<")){

            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else if (!strcmp(ch, ">>")){

            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else if (!strcmp(ch, ">")){

            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else if (!strcmp(ch, "2>")){
            
            memset(tempBuffer, 0, sizeof(tempBuffer));
        }else{
          //bookmark
            tempBuffer[i] = args[i];
        }
        i++;
    }
    if (p == NULL)
        p = create_process(NULL, args);
    j = create_job(first_job, p, 0, 1, 2);
    launch_job(j, !background);
    memset(tempBuffer, 0, sizeof(tempBuffer));
}

int main(void)
{
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    //char *path;
    //pid_t childpid;
    int count; // count of items in args 
    int last;

    while (1){
        //init_shell();
        background = 0;
        count = 0;
        printf("myshell: ");
        /*setup() calls exit() when Control-D is entered */
        setup(inputBuffer, args, &background);
        while(args[count] != NULL){ // counting items in args 
            count++;
        }
        if (!strcmp(args[0], "ps_all")) {
            printf("calling ps_all\n");
        } 
        else if (!strcmp(args[0], "search")) {
            last = strlen(args[1]) - 1;
            /* removing quotation marks from input string*/ 
            memmove(&args[1][last], &args[1][last + 1], strlen(args[1]) - last);
            memmove(&args[1][0], &args[1][0 + 1], strlen(args[1]) - 0);

            if (count == 3){
                if (!strcmp(args[2], "-r")) {
                    printf("calling search recurisve\n");
                    search(args[1], 0);
                }
            } else if (count == 2) {
                printf("calling search \n");
                search(args[1], 1);
            }
        } 
        else if (!strcmp(args[0], "bookmark")){
            printf("calling bookmark\n");
        } 
        else if (!strcmp(args[0], "^z")){
            printf("calling ^z\n");
            // send SIGSTOP signal
        } 
        else if (!strcmp(args[0], "exit")){ // calling exit 
            printf("calling exit\n");
            // check for background processes 
            // if none then exits
            if (0) {
                printf(" ");
            } else {
                exit(1);
            }
        } 
        else {
            execute_command(args, background);
        }
    } // end of while
}
