
/*
With this program, user type texts line by line, where each line has a maximum length of 127 characters.  
Each line is considered as "entered" (i.e., finalized) when the ENTER or RETURN key is clicked.
The program counts the number of characters entered for each minute.
When user types ":exit", the program exits.  
The Makefile should be changed to include this program and thus it can be run from the shell of xv6.
*/

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_LINE_SIZE 128
#define MAX_BUF_SIZE 1280
char buf[MAX_BUF_SIZE];


void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(1);
}

//create a new process
int
fork1(void)
{
  int pid;
  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

//create a pipe
void 
pipe1(int pipe_1[2])
{
 int rc = pipe(pipe_1);
 if(rc<0){
   panic("Fail to create a pipe.");
 }
}


//pull everything from pipe and return the size of the content
int
read1(int *pipe_1)
{
 char ch='a';
 write(pipe_1[1],&ch,1); //write something to the pipe so the next read will not block (because read operation is a blocking operation - if the pipe is empty, read will wait till something is put in)
 int len = read(pipe_1[0],buf,MAX_BUF_SIZE)-1;
 return len;
}


int
main(int argc, char *argv[])
{
   if( argc != 3 ) {
        printf("Two arguments expected.\n");
        exit(1);
   }
   int t_time = atoi(argv[1]);
   int interval = atoi(argv[2]);
   if (t_time == 0 || interval == 0) {
       printf("invalid arguments!\n");
       exit(1);
   }

   if (t_time % interval != 0) {
       printf("invalid arguments!\n");
       exit(1);
   }

  //create two pipe to share with child
  int pipe_1[2], pipe_2[2];
  pipe1(pipe_1); //a pipe from child to parent - child sends entered texts to parent for counting
  pipe1(pipe_2); //a pipe from child to parent - child lets parent stop (when user types :exit)

  int result = fork1();//create child process
  if(result==0){

     //child process:
     close(pipe_1[0]);
     close(pipe_2[0]);
     int num_iter = t_time / interval;
     for (int i = 0; i < num_iter; i++) {
         sleep(interval / 2);
         write(pipe_1[1], "Hello!", 6);
         sleep(interval / 2 + interval % 2);
     }
     char ch='a';
     write(pipe_2[1], &ch, 1);
     exit(0);

  } else {
     //parent process:
     while(1) {
         sleep(60);
         int len = read1(pipe_1);
         printf("\nIn last minute, %d characters were entered.\n", len);

         len = read1(pipe_2);
         if (len > 0) {
             //now to terminate
             wait(0);
             exit(0);
         }
     }
  }

  return 0;

}
