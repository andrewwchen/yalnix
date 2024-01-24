#include <yuser.h>


#define MAX_LINE 1024

int
main(int argc, char *argv[])
{
  int console;
  char buf[MAX_LINE];

  TracePrintf(2, "Hello from the console, argc = %d\n", argc);
  

  if (argc < 2) {
    TtyPrintf(0, "CONSOLE requires a terminal id.\n");
    Exit(-1);
  }
  console = atoi(argv[1]);

  TracePrintf(2, "Hello\n");
  TtyPrintf(console, "YALNIX READY\n");
  TtyPrintf(console, "Type `halt' to halt Yalnix.\n");

  while (1) {
    int n;
    char * word;
    char * end_word;

    TtyPrintf(console, ">>> ");
    n = TtyRead(console, buf, MAX_LINE);
    if (n == 0 || n >= MAX_LINE) continue;
    buf[n] = '\0';
    for (word = buf; isspace(*word); word++);
    for (end_word = word; *end_word && !isspace(*end_word); end_word++);
    *end_word = '\0';
    if (!strcmp(word, "halt")) {
      TtyPrintf(console, "Halting....\n");
      Exit(0);
    }
    if (*word != '\0')
      TtyPrintf(console, "`%s': Command not recognized.\n", word);
  }

}
  
  
		
  
  
  


