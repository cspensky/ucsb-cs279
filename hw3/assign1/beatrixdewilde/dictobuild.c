/* Beatrix De Wilde, beatrixdewilde@umail.ucsb.edu */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define PATH_MAX 1024

int main(int argc, char **argv)
{
  // Stores directory to search from
  char directory[PATH_MAX];
  strcpy(directory, getcwd(NULL, 1024));
  // Stores output file - default standard output  
  char output[PATH_MAX]  = "";
  int flag;
  // Goes through command line arguments given - none required
  while ((flag = getopt (argc, argv, "d:o:")) != -1){
    switch (flag)
    {
      case 'd':
        strcpy(directory,optarg);
        break;
      case 'o':
	      strcat(output," > ");
        strcat(output,optarg);
     	break;
    }
  }
  // Gets any extra extensions
  char *dictoext = getenv("DICTOEXT"); 
  char extensions[PATH_MAX];
  char *ext;
  char colon[2] = ":";
  int doc = 0;
  if (dictoext != NULL) {
    ext = strtok(dictoext, colon);
    while (ext != NULL) {
      if (!strcmp(ext, "doc")) {
        doc = 1;
      } else {
        if (strlen(extensions) != 0) {
        strcat(extensions, " -o -name '*."); 
        } else {
          strcat(extensions, " -name '*."); 
        }
        strcat(extensions, ext);
        strcat(extensions, "' ");
      } 
      ext = strtok(NULL, colon);  
    }
  } else {
    strcpy(extensions," -name '*.md' -o -name '*.txt' ");
  }
  if (doc) {
    // Needs catdoc - parses doc files
    char doc_command[PATH_MAX];
    sprintf(doc_command, "find %s \\( -name '*.doc' \\) -exec catdoc {} \\; | tr -cs 'A-Za-z' '\\n' | tr 'A-Z' 'a-z' | sort | uniq %s", directory, output);
    printf("Command: %s \n", doc_command);
    system(doc_command);
  }
  char command[PATH_MAX];
  /* Finds all files in given directory with given extensions, translates all the words in the file,
   *  sorts them, then reduces duplicates then if output given tunnels to output */
  sprintf(command, "find %s \\( %s \\) -exec cat {} \\; | tr -cs 'A-Za-z' '\\n' | tr 'A-Z' 'a-z' | sort | uniq %s", directory, extensions, output);
  printf("Command: %s \n", command);
  system(command);
  return 0;
}