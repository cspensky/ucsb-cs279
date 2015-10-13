/* Chad Spensky, cspensky@cs.ucsb.edu */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

// Debug?
const int DEBUG = 0;

// Some globals
char **fileExtensions = (char*[]){"txt","md"};
int EXTENSIONS_COUNT = sizeof(fileExtensions)/sizeof(*fileExtensions);
FILE *OUTPUT_FILE;
//
// Detect if the file matches any of our "text" extensions.
//
int is_text_file(char *filename) {
	const char delim[] = ".";
	char* extension;

	// Get extension
	int i;
	for (i = strlen(filename)-1; i >= 0; i--) {
		if (filename[i] == *delim) {
			i += 1;
			break;
		}
	}
	
	extension = filename+i;

	for (i = 0; i < sizeof(fileExtensions)/sizeof(*fileExtensions); i++) {
		if (strcmp(fileExtensions[0],extension) == 0) {
			if (DEBUG)
				printf("Found %s, matched %s\n", filename, extension);
			return 1;
		}
	}

	return 0;
}

int extract_words(const char* path, char* filename) {
	// Construct our full filepath name
	char filepath[strlen(path)+strlen(filename)];
	filepath[0] = '\0';
	strcat(filepath, path);
	strcat(filepath, "/");
	strcat(filepath, filename);

	if (DEBUG)                                                                  
        printf("Extracting words from %s...\n", filepath);   	
	
	// Open our file
	FILE * fp;
	size_t len = 0;
    ssize_t read;
	char * line = NULL;
	// Token stuff
//    const char delim[2] = " ";                                                  
//    char *token;

    fp = fopen(filepath, "r");
	// Could it be opened?
    if (fp == NULL)
		return -1;
	
	// Read every line
    while ((read = getline(&line, &len, fp)) != -1) {
        if (DEBUG)
			printf("line: %s", line);

		int i = 0;
		int wasChar = 0;
		for (i = 0;i < strlen(line);i++) {
			if ((line[i] >= 0x30 && line[i] <= 0x39) || 
				(line[i] >= 0x41 && line[i] <= 0x5a) || 
				(line[i] >= 0x61 && line[i] <= 0x7a)) {
				fprintf(OUTPUT_FILE,"%c", line[i]);
				wasChar = 1;
			} else if (wasChar) {
				fprintf(OUTPUT_FILE,"\n");
				wasChar = 0;
			}
		}	
		if (wasChar) {
			fprintf(OUTPUT_FILE, "\n");
		}
	
	   /* Using tokens
		
	   // Tokenize that line and get the words
	   token = strtok(line, delim);
   
	   // Go through all words
	   while( token != NULL ) 
	   {
	      printf("%s\n", token );
		
	      token = strtok(NULL, delim);
	   }
		*/
	}

	// clean up
    fclose(fp);
    if (line)
		free(line);
	return 1;
}

/*
 * Ref: http://stackoverflow.com/questions/8436841/how-to-recursively-list-directories-in-c-on-linux
 */
void listdir(const char *name, int level)
{
    DIR *dir;
    struct dirent *entry;

	// Make sure we can actually open it
    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

	// Loop over every entry in the directory
    do {

		// Directory
        if (entry->d_type == DT_DIR) {

            char path[1024]; // UNIX filename limit? 
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;

			// Ignore 'special' directories
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

//            printf("%*s[%s]\n", level*2, "", entry->d_name);
            listdir(path, level + 1);

        } else {
			// Only look at text files (i.e. they match our extension)
			if (is_text_file(entry->d_name)) {

				extract_words(name, entry->d_name);
				// We found a file!
			}
	
		}
    } while ((entry = readdir(dir)));
    closedir(dir);
}


int main(int arglen, char** args) {
	OUTPUT_FILE = stdout;
	char* directory = ".";
	char* extensions;
	char* token;
	char ext_delim[2] = ":";
	int file_types = 0;

	extensions = getenv("DICTOEXT");

	if (extensions != NULL) {
		
		// Count the number of arguments
		int i;
		for (i = 0; i < strlen(extensions); i++) {
			if (extensions[i] == ext_delim[0]) {
				file_types++;
			}
		}

		// Make space for our file types
		fileExtensions = (char **)malloc(sizeof(char*)*file_types);

		// Split our input on the delimiter (e.g. :)
		int ext_idx = 0;
		token = strtok(extensions, ext_delim);
		printf("Parsing the following extensions:\n");
		while( token != NULL )                                                   
		{                                                                        
			printf("  %s\n", token );                                                                                                                       

			// Copy extension name into our array
			fileExtensions[ext_idx] = (char *)malloc(strlen(token));
			strncpy(fileExtensions[ext_idx],token,strlen(token));                                      
            token = strtok(NULL, ext_delim);                                    
		}   
	}
	// Do we have arguments	
    if (arglen >= 3) {
		int i;
		for (i = 1; i < arglen; i++) {
			// Did the user define a start directory?
			if (strcmp(args[i],"-d") == 0) {
				if (arglen > i+1) {
					directory = args[i+1];
					i++;
				}
			}
			// Did the user define an output directory?
			if (strcmp(args[i], "-o") == 0) {
				if (arglen > i+1) {                                             
                    OUTPUT_FILE = fopen(args[i+1],"w+");                                      
                    i++;                         
                               
					// Make sure the file is open
					if (OUTPUT_FILE == NULL) {                                                  
					    fprintf(stderr, "Could not open output file!\n");                        
						return -1;                                                              
				    }  
                }         
			}
		}
	}
	listdir(directory,0);
	return 0;	
}
