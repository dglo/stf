/* chkpt.c, make sure that the program
 * has quit -- if not, kill it...
 *
 * should time be configurable?
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int caught = 0;

static void sighnd(int sig) { caught = sig; }

int main(int argc, char *argv[]) {
   char **nargv;
   int i, alrmtime = 120, ai=1;
   pid_t ch;

   if (argc<2) {
      fprintf(stderr, "usage: chkpt [seconds] program ...\n");
      return 1;
   }

   if ((nargv = (char **) malloc( argc* sizeof(char *)))==NULL) {
      fprintf(stderr, "can't allocate memory!\n");
      return 1;
   }

   if (isdigit(argv[1][0]) && atoi(argv[1])>0) {
      alrmtime=atoi(argv[1]);
      ai = 2;
   }

   for (i=ai; i<argc; i++) nargv[i-ai] = argv[i];
   nargv[argc-ai] = NULL;

   signal(SIGCHLD, sighnd);
   signal(SIGALRM, sighnd);

   if ((ch=fork())<0) {
      perror("fork");
      return 1;
   }
   else if (ch==0) {
      if (execvp(argv[ai], nargv)) {
#if 0
	 int i=0;
	 printf("path: '%s'\n", getenv("PATH"));
	 printf("ai: %d, argv[%d]='%s'\n", ai, ai, argv[ai]);
	 while (nargv[i]!=NULL) {
	    printf("nargv[%d] = '%s'\n", i, nargv[i]);
	    i++;
	 }
	 fflush(stdout);
#endif
	 perror("exec");
	 return 1;
      }
   }

   alarm(alrmtime);
   pause();
   
   {  int status;
   
      if (caught==0) {
	 kill(ch, SIGTERM);
      }
      else if (caught==SIGCHLD) {
	 /* child died, we can exit normally... */
      }
      else if (caught==SIGALRM) {
	 kill(ch, SIGTERM);
      }

      wait(&status);
      return (WIFSIGNALED(status)) ? WTERMSIG(status) : WEXITSTATUS(status);
   }

   return 0;
}






