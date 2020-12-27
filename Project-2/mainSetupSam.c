#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#include <limits.h>
#include <dirent.h> 
#include <sys/types.h> 

// Sam
/* The setup function below will not return any value, but it will just: read
in the next command line; separate it into distinct arguments (using blanks as
delimiters), and set the args array entries to point to the beginning of what
will become null-terminated, C-style strings. */

void bookmark(char *command, int flag) {
    printf("in bookmark function");
}

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

void setup(char inputBuffer[], char *args[],int *background) {
    int length, /* # of characters in the command line */
        i,      /* loop index for accessing inputBuffer array */
        start,  /* index where beginning of next command parameter is */
        ct = 0;     /* index of where to place the next parameter into args[] */
            
    /* read what the user enters on the command line */
    length = read(STDIN_FILENO,inputBuffer,MAX_LINE);  

    /* 0 is the system predefined file descriptor for stdin (standard input),
       which is the user's screen in this case. inputBuffer by itself is the
       same as &inputBuffer[0], i.e. the starting address of where to store
       the command that is read, and length holds the number of characters
       read in. inputBuffer is not a null terminated C-string. */

    start = -1;
    if (length == 0) {
        exit(0);            /* ^d was entered, end of user command stream */
    }

    /* the signal interrupted the read system call */
    /* if the process is in the read() system call, read returns -1
    However, if this occurs, errno is set to EINTR. We can check this  value
    and disregard the -1 value */
    if ( (length < 0) && (errno != EINTR) ) {
        perror("error reading the command");
	    exit(-1);           /* terminate with error code of -1 */
    }

	printf(">>%s<<\n",inputBuffer);
    for (i=0;i<length;i++){ /* examine every character in the inputBuffer */
        switch (inputBuffer[i]){
            case ' ':
            case '\t':               /* argument separators */
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
                if (start == -1) {
                    start = i;
                }
                if (inputBuffer[i] == '&'){
                    printf("wtf");
                    *background  = 1;
                    printf("bg : %d\n", *background);
                    inputBuffer[i-1] = '\0';
                }
	    } /* end of switch */
    }    /* end of for */
    args[ct] = NULL; /* just in case the input line was > 80 */

	for (i = 0; i <= ct; i++) {
        printf("args %d = %s\n",i,args[i]);
    }
} /* end of setup routine */

char* find_given_command_path(char* args[]) {

	char* path = getenv("PATH");
	FILE* file;
	char* token = strtok(path, ":");
	char* my_command = args[0];
	char* filename;
	int is_exists = 0;
    printf("Your file is here %s\n", path);
    // printf("PATH : %s\n", getenv("PATH"));
    // printf("HOME : %s\n", getenv("HOME"));
	while (token != NULL) {
		filename = malloc(strlen(token) + strlen(my_command) + 2);
		strcpy(filename, token);
		strcat(filename, "/");
		strcat(filename, my_command);
        printf("Your token is  %s\n", token);

		if (file = fopen(filename, "r")) {
			printf("Your file is here %s\n", filename);
			is_exists = 1;
			return filename;
		}
		token = strtok(NULL, ":");
	}
}

int main(void) {
    char inputBuffer[MAX_LINE]; /*buffer to hold command entered */
    int background; /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2 + 1]; /*command line arguments */
    char *path;
    pid_t childpid;
    int count; // count of items in args 
    int last;
    // printf("PATH : %s\n", getenv("PATH"));

    while (1){
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
            childpid = fork(); // Creating new process using fork()
            if (childpid == -1 ) {
                perror("failed to fork");
                return 1;
            }
            if (childpid == 0) { // in child process
                //execv(const char *path, char *const argv[]);
                path = find_given_command_path(args);
                printf("the Path is : %s\n", path);
                execv(path, args);
            } 
            if (background == 0) { // in Parent
                printf("waiting\n");
                while(wait(NULL)>0); // waiting for all children
            } else {
                printf("background process\n");
                // setup(inputBuffer, args, &background);		
            }
        }
    } // end of while
} // end of main
