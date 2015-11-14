/*Peng Gu, peng_gu@ece.ucsb.edu*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<string.h>
#include<errno.h>
/* "readdir" etc. are defined here. */
#include<dirent.h>
/* limits.h defines "PATH_MAX". */
#include<limits.h>
#include<unistd.h>




void searchfile(char* file_path, char* out_path){
    FILE *fp, *fp2;
    char temp;
    int word=0;
    fp = fopen(file_path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open input file!\n");
        exit(1);
    }

    fp2 = fopen(out_path, "at");
    if ((fp2 == NULL) && (out_path!=NULL)) {
        fprintf(stderr, "Can't open output file!\n");
        exit(1);
    }

    while ((temp = fgetc(fp)) != EOF)
    {
        if (((temp>='0') & (temp <='9')) || ((temp>='a') & (temp <='z')) || ((temp>='A') & (temp <='Z')))
        {
            if (out_path!=NULL)
                //fprintf(fp2, "%c", temp);
                fputc(temp,fp2);
            else
                printf("%c",temp);
            word=0;
        }
        else if (word==0)
        {
            if (out_path!=NULL)
                //fprintf(fp2, "%c", temp);
                fputc('\n',fp2);
            else
                printf("\n");
            word=1;
        }
    }
    fclose(fp);
    if (out_path!=NULL)
        fclose(fp2);
    return;
}


const char *get_filename_ext(const char *filename){
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}


int valid_file_ext(const char *filename,const char *file_ext){
    char *ptr=strstr(file_ext,get_filename_ext(filename));
    printf("%s\n",filename);
    if (ptr==NULL)
        return 0; 
    else
        return 1;
}


void list_dir (const char * dir_name, char * out_dir)
{
    DIR * d;
    /* Open the directory specified by "dir_name". */
    d = opendir (dir_name);
    /* Check it was opened. */
    if (! d) {
        fprintf (stderr, "Cannot open directory '%s': %s\n",
                dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }

    const char* s = getenv("DICTOEXT");
    if (s==NULL){
        printf("DICTOEXT :%s\n",(s!=NULL)? s : "getenv returned NULL");
        exit (0);
    }


    while (1) {
        struct dirent * entry;
        char * d_name;
        /* "Readdir" gets subsequent entries from "d". */
        entry = readdir (d);
        if (! entry) {
            /* There are no more entries in this directory, so break
               out of the while loop. */
            break;
        }
        d_name = entry->d_name;
        /* Print the name of the file and directory. */
        //printf ("%s/%s\n", dir_name, d_name);
        //printf ("%s\n", d_name);

        char adr[2048];
        char a[]={""};



        if ((valid_file_ext(d_name,s)==1) && (strcmp(d_name,".")!=0) && (strcmp(d_name,"..")!=0) && (strcmp(d_name,"")!=0))//valid_file_ext(const char *filename,const char *file_ext)
        {
            strcat(adr,dir_name);
            strcat(adr,"/");
            strcat(adr,d_name);

            printf("***********************************************************\n");
            printf("Followings are the words from\n");
            printf("%s\n",adr);
            printf("***********************************************************\n");
            //printf("%s\n",dir_name);
            searchfile(adr,out_dir);
            printf("***********************************************************\nend\n");
            printf("***********************************************************\n\n\n");
        }
        strcpy(adr,a);


#if 0
        /* If you don't want to print the directories, use the
           following line: */

        if (! (entry->d_type & DT_DIR)) {
            printf ("%s/%s\n", dir_name, d_name);
        }
#endif /* 0 */
        if (entry->d_type & DT_DIR) {
            /* Check that the directory is not "d" or d's parent. */
            if (strcmp (d_name, "..") != 0 &&
                    strcmp (d_name, ".") != 0) {
                int path_length;
                char path[PATH_MAX];
                path_length = snprintf (path, PATH_MAX,
                        "%s/%s", dir_name, d_name);
                /*printf ("%s\n", path);*/
                if (path_length >= PATH_MAX) {
                    fprintf (stderr, "Path length has got too long.\n");
                    exit (EXIT_FAILURE);
                }
                /* Recursively call "list_dir" with the new path. */
                list_dir (path,out_dir);
            }
        }
    }
    /* After going through all the entries, close the directory. */
    if (closedir (d)) {
        fprintf (stderr, "Could not close '%s': %s\n",
                dir_name, strerror (errno));
        exit (EXIT_FAILURE);
    }
}




int main(int argc,char *argv[]){

    char *default_dir;
    char *default_out;

    if (argc==5){
        if (strcmp(argv[1],"-d")==0){
            default_dir=argv[2];
        }
        else if (strcmp(argv[3],"-d")==0){
            default_dir=argv[4];
        }

        if (strcmp(argv[1],"-o")==0){
            default_out=argv[2];
        }
        else if (strcmp(argv[3],"-o")==0){
            default_out=argv[4];
        }

    }
    else if (argc==3){
        if (strcmp(argv[1],"-d")==0){
            default_dir=argv[2];
        }
        else
        {
            default_dir=NULL;
        }
        if (strcmp(argv[1],"-o")==0){
            default_out=argv[2];
            printf("33\n");
        }
        else
        {
            default_out=NULL;
        }
    }
    else{
        default_dir=NULL;
        default_out=NULL;
    }

    if (default_dir==NULL){
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
            fprintf(stdout, "Current working dir: %s\n", cwd);
        else
            perror("getcwd() error");
        list_dir(cwd,default_out);

    }
    else{
        list_dir(default_dir,default_out);
    }




    /*searchfile("haha.txt");*/
    /*printf("%s\n",get_filename_ext(argv[1]));*/
    /*printf("%i\n",argc);*/
    return 0;
}

