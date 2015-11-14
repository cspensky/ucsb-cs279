/* Chris Salls, salls@cs.ucsb.edu */
#include <ftw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

const int MAX_EXT = 100;
char **extensions = NULL;
char *ofile = "dict.txt";
int num_files = 0;
char command[10000];

int check_ext(const char *fpath) {
  char *p = strrchr(fpath, '.');
  if (p == NULL)
    return 0;
  p += 1;
  int i;
  for(i = 0; i < MAX_EXT; i++) {
    char *ext = extensions[i];
    if (ext == NULL)
      return 0;
    if (!strcmp(ext, p))
      return 1;
  }

  return 0;
}

int handle_file(const char *fpath, const struct stat *sb,
                int typeflag) {
  if (typeflag != FTW_F) {
    return 0;
  }
  
  if (!check_ext(fpath))
    return 0;
  
  snprintf(command, 9999, "cat %s | tr -cs 'a-zA-Z0-9' '\n' > %s.tmp", fpath, ofile);
  system(command);
  printf(command);
  // remove dups
  snprintf(command, 9999, "cat %s.tmp | sort -u  >>%s ", ofile, ofile);
  system(command);
  // remove old
  snprintf(command, 9999, "rm %s.tmp", ofile);
  
  num_files += 1;
  return 0;
}

void print_usage(name) {
    printf("Usage: dictobuild -d <directory> -o <outputfile>\n");
}

void get_extensions() {
  char* dictext;
  dictext = getenv("DICTOEXT");
  extensions = malloc(sizeof(char *)*MAX_EXT);
  if (dictext == NULL) {
    extensions[0] = "txt";
    extensions[1] = "md";
    return;
  }
  
  memset(extensions, '\0', sizeof(char *)*MAX_EXT);
  int count = 0;
  int i = 0;
  char *curr = dictext;
  int len = strlen(dictext);
  for(i = 0; i < len+1 && count < MAX_EXT; i++) {
    if (dictext[i] == ':' || dictext[i] == '\0') {
      dictext[i] = '\0';
      extensions[count] = curr;
      curr = dictext + i + 1;
      count++;
    }
  }
}

int main(int argc, char *argv[])
{
    int c;
    char *dir = ".";
    extern char *optarg;
    extern int optind, optopt;

    while ((c = getopt(argc, argv, "d:o:")) != -1) {
        switch(c) {
        case 'd':
            dir = optarg;
            break;
        case 'o':
            ofile = optarg;
            break;
        default: 
            print_usage(); 
            exit(1);
        }
    }

    get_extensions();
    
    // empty the old outfile
    snprintf(command, 9999, "echo '' >  %s", ofile);
    system(command);
    // iterate over the all files in the directory and its subdirectories
    ftw(dir, handle_file, 20);
    // remove duplicates
    snprintf(command, 9999, "cat %s | sort -u  > %s.tmp", ofile, ofile);
    system(command);
    // move it
    snprintf(command, 9999, "mv %s.tmp %s", ofile, ofile);
    system(command);
    // remove empty lines
    snprintf(command, 9999, "sed -i '/^$/d' %s", ofile);
    system(command);
    
    printf("handled %d files\n", num_files);

    return 0;
}
