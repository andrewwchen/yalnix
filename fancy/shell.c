// #include<ctype.h>
// #include<errno.h>
#include <yuser.h>

#define MAX_LENGTH 1024  /* Should be long enough to hold any line */

int
main(int argc, char *argv[])
{
  char buf[MAX_LENGTH];
  char * cmd_argv[MAX_LENGTH];  /* An overkill for expediency */
  char * prompt = "yalnix> ";
  char separators[4];
  int termno;
  int err;
  separators[0] = ' ';
  separators[1] = '\t';
  separators[2] = '\n';
  separators[3] = '\0';

  if (argc < 2) {
    Exit(-2);
  }
  termno = atoi(argv[1]);
  /* In a real OS NUM_TERMINALS could be obtained from a lib call
   * or a syscall
   */
  if (termno < 0 || termno > 3) {  
    Exit(-3);
  }

  TtyPrintf(termno, "Starting shell....\n");

  while(1)
    {
      int n, i, pid;
      int res;
      int j = 0;
      bzero(buf, MAX_LENGTH);
      TtyPrintf(termno, prompt);
      n = TtyRead(termno, buf, MAX_LENGTH);
      if(n == 0 || n >= MAX_LENGTH)		/* line too big */
	continue;
      buf[n] = '\0';
      if(!(cmd_argv[j++] = strtok(buf, separators)))
	continue;
      if(strcmp(cmd_argv[0], "exit") == 0) {
	TtyPrintf(termno, "Exitting shell....\n");
	Exit(0);
      }
      while(cmd_argv[j++] = strtok(NULL, separators))
	;
      if(pid = Fork())
	{
	  if(pid == -1)
	    {
	      TracePrintf(0,"Shell: Cannot fork process\n");
	      continue;
	    }
	  if ((pid = Wait(&res)) != -1) {
	    TtyPrintf(termno, "PID %d exit status = %d\n", pid, res);
	  }
	  else {
	    TtyPrintf(termno, "PID %d aborted by kernel.\n", pid);
	  }
	}
      else
	{
	  Exec(cmd_argv[0], cmd_argv);
	  TtyPrintf(termno, "Could not exec `%s'.\n", buf);
	  Exit(-2);
	}
    }
}


