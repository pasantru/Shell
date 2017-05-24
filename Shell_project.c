/**
 UNIX Shell Project
 **/

#include "job_control.h"   // remember to compile with module job_control.c
#include <string.h>

#define MAX_LINE 256 /* 256 chars per line, per command, should be enough. */

// -----------------------------------------------------------------------
//                            MAIN
// -----------------------------------------------------------------------
/*
  suspended -- stopped
  When a process is suspended with control z its status must be changed to Stopped
  Delete -- con las posiciones de la listaAux
  IF pid_wait == pgid

  Add a part of code in the controller that lets us control the
*/
job* list;

void manejador(){
    int status;
    int info;
    enum status status_res;
    int pid_wait;
    int perfNext = 0;

    job * aux;
    job * listaAux = list;
    // printf("\n¡Signal SIGCHILD launched!\n");
    block_SIGCHLD();
    if(empty_list(listaAux)){
        printf("The process list is empty!\n");
    }else{
      listaAux = listaAux->next; //Skips the first element as its not a job
        while(listaAux != NULL){ //Iterate the list
            pid_wait = waitpid(listaAux->pgid,&status, WNOHANG | WUNTRACED); //Returns the nohang
            if(pid_wait == (listaAux->pgid)){ //If the process has the same pgid as the one we are looking for
              status_res = analyze_status(status, &info); //Analizes the status of the process of the current process
              if(status_res == SUSPENDED){ //If the status is suspended then change its state to stoppeed
                //Change status
                listaAux->state = STOPPED;
              }
              if(status_res == SIGNALED || status_res == EXITED){
                aux = listaAux;
                printf("\nDeleting process pid: %d, Command: %s, %s, Info: %d\n",aux->pgid,aux->command,status_strings[status_res],info);
                perfNext = 1;
                listaAux = listaAux->next;
                delete_job(list,aux);
                free_job(aux);
              }
            }
          if(perfNext == 0) listaAux = listaAux->next;
          else perfNext = 0;
        }
    }
    unblock_SIGCHLD();
    // fflush(stdout);
}


int main(void)
{
    char inputBuffer[MAX_LINE]; /* buffer to hold the command entered */
    int background;             /* equals 1 if a command is followed by '&' */
    char *args[MAX_LINE/2];     /* command line (of 256) has max of 128 arguments */
    // probably useful variables:
    int pid_fork, pid_wait; /* pid for created and waited process */
    int status;             /* status returned by wait */
    enum status status_res; /* status processed by analyze_status() */
    int info;				/* info processed by analyze_status() */


    list = new_list("ListaProc"); //Initialise the list
    signal(SIGCHLD,manejador);

    while (1)   /* Program terminates normally inside get_command() after ^D is typed*/
    {
        ignore_terminal_signals();

        printf("COMMAND->");
        fflush(stdout);
        get_command(inputBuffer, MAX_LINE, args, &background);  /* get next command */

        if(args[0]==NULL){
           continue; // if empty command
        }else if(strcmp(args[0],"cd")== 0){
          //Separate if for "cd" as it has to check the directory beore executing the program
          if(chdir(args[1])==-1){
            perror(args[1]);
          }

          continue;
        }else if(strcmp(args[0],"jobs")== 0){
          if(empty_list(list)){
            printf("The list is empty!\n");
          }else{
            block_SIGCHLD();
            print_job_list(list);
            unblock_SIGCHLD();
          }
          continue;
        }else if(strcmp(args[0],"fg")== 0){ //Comparison for fg
          if(empty_list(list)){
            printf("The list is empty!\n");
          }else{
            //Code for fg
            if(args[1] != NULL){
              int num;
              sscanf (args[1],"%d",&num);
              job* item = get_item_bypos(list, num);
      	      item->state=FOREGROUND;
              set_terminal(item->pgid);
	            killpg(item->pgid, SIGCONT);
              pid_wait = waitpid(item->pgid, &status, WUNTRACED); //esperamos a la señal de terminación.
            }else{
              job * item = get_item_bypos(list, 1); //Gets the first element in the list
              item->state = FOREGROUND; //Changes its state to foreground
              set_terminal(item->pgid); //Gives the process the control over the terminal
	      	    killpg(item->pgid,SIGCONT);
              pid_wait = waitpid(item->pgid, &status, WUNTRACED); //esperamos a la señal de terminación.
            }
            continue;

          }
        }else if(strcmp(args[0], "bg") == 0){ //Comparison for bg
          if(args[1] != NULL){ //This block of code if for fg with arguments
            int num;
            sscanf (args[1],"%d",&num); //Conversion of char * to int
            job* item = get_item_bypos(list, num); //Creates an item with the one stored in the list in pos num
            item->state=BACKGROUND; //Changes the state of the process to background
            killpg(item->pgid, SIGCONT); //Kills the process group associated to the processs and sends the signal SIGCONT
          }else{ //fg without arguments
            job * item = get_item_bypos(list, 1); //Gets the first element of the list
            item->state = BACKGROUND; //Changes its state to bg
	      	  killpg(item->pgid,SIGCONT);
          }
          continue;
        }else{
            pid_fork = fork();
            if(pid_fork == 0){//Chlild process
                restore_terminal_signals(); //restore terminal signals.
                pid_fork = getpid(); //pid_fork pierde el 0 y se le asocia el pid real del proceso creado.
                new_process_group(pid_fork); // crea un grupo de procesos independientes.
                if(background == 0){
                    set_terminal(pid_fork); //asignación del terminal si el proceso es fg.
                }
                execvp(args[0],args); // This executes processes in fg and bg
                perror("Error, command not found");
                exit(-1);
            }else{ //proceso padre.
                new_process_group(pid_fork);
                if(background == 0){
                    set_terminal(pid_fork); //sets terminal to child
                    pid_wait = waitpid(pid_fork, &status, WUNTRACED); //esperamos a la señal de terminación.
                    status_res = analyze_status(status, &info); //analizamos como ha terminado el proceso
                    if(status_res==SUSPENDED){ //This is when you do a ctrl + z
                      block_SIGCHLD(); //Blocks the signal to acces the list
                      job * item = new_job(pid_fork,args[0],STOPPED);
                      add_job(list,item);
                      unblock_SIGCHLD(); //Unblocks the isgnal
                    }
                    printf("\nForeground pid %d, command: %s, %s, info: %d\n", pid_fork, args[0], status_strings[status_res], info);
                    set_terminal(getpid()); //la shell recupera el terminal.
                }else{ //proceso en background
                    printf("\nBackground job running... pid: %d, command: %s\n", pid_fork,args[0]);
                    block_SIGCHLD(); //Blocks the signal to acces the list
                    job * item = new_job(pid_fork,args[0],background); //Creates the job when executed with an &
                    add_job(list,item);
                    unblock_SIGCHLD(); //Unblocks the isgnal
                }
            }
        }
    } // end while
}
