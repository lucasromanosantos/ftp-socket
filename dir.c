#include "header.h"
#include "dir.h"

int count_slashes(char* c, int len) {
    int i,count=0;
    for(i=0; i<len; ++i) {
        if(c[i] == '/') {
            count++;
        }
    }
    return count;
}

void print_matrix(char** c,int n) {
    int i;
    for(i=0; i<n; i++) {
        printf("\t");
        puts(c[i]);
    }
}

char** to_matrix(char *c, int n) {
    int i,j=0,k;
    char **tmp = malloc(sizeof(char*) * n);
    for(i=0; i<n; i++) {
        tmp[i] = malloc(sizeof(char*) * FILE_LEN);
        for(k=0; k<FILE_LEN; k++) {
            tmp[i][k] = '\0';
        }
    }
    //       Yes, its j, not i.
    for(i=0,k=0; j<n; i++,k++) {
        if(c[i] == '/') {
            k=-1; // It will be incremented to 0 in the end of the loop.
            j++;
        }
        else {
            tmp[j][k] = c[i];
        }
    }
    return tmp;
}

char* to_vector(char **c, int n) {
    int i,size = 1024;
    char *tmp = malloc(sizeof(char) * size);
    for(i=0; i<size; i++)
        tmp[i] = '\0';
    for(i=0; i<n; i++) {
        if(strlen(tmp) + strlen(c[i]) > size) { // To prevent overflows.
            size = size * 2;
            tmp = realloc(tmp,sizeof(char) * size);
        }
        strcat(tmp,c[i]);
        strcat(tmp,"/");
    }
    for(i=0; i<n; i++) {
        free(c[i]);
    }
    free(c);
    return tmp;
}

char* fix_dir(char *c, int length) {
    int i,j,n = count_slashes(c,length);
    char **tmp;
    tmp = to_matrix(c,n);
    // Now I have a matrix from the path, with n lines and unknown number of columns.
    for(i=2; i<n; i++) { // If tmp[0] == ../, theres nothing I can do with it.
        if(strcmp(tmp[i],"..") == 0 && strcmp(tmp[i-1],"..") != 0) {
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            i = 2; // So a/b/c/../../ will remove b and c and both ../, not only c.
        }
    }
    c = to_vector(tmp,n);
    return c;
}

int check_cd(char* c) {
/* This function will concatenate the path received with the global variable ADDR,
 * then it will fix it (every ../../) and will try to open this directory. If it does
 * not exist, or you do not have permission (which I doubt, cause you are sudo), error
 * (0) shall be returned, otherwise, (1) will be returned.
 */
    DIR *dir;
    char *tmp;
    int len;

    if(strlen(c) == 0)
        return 0;
    if((tmp = malloc(sizeof(char) * 1024)) == NULL) {
        puts("(check_cd) Error allocating memory.");
        exit(-1);
    }
    len = strlen(c);
    if(c[len-1] != '/') {
        c[len] = '/';
        c[len+1] = '\0';
    }
    strcpy(tmp,LocalPath);
    strcat(tmp,c);
    tmp = fix_dir(tmp,strlen(tmp));
    dir = opendir(tmp);
    if(dir == NULL) {
        // Problem. Server should send an error message (outside this function!).
        puts("(check_cd) Could not open this directory - it does not exit or you do not have permission.");
        free(tmp);
        return 0;
    }
    // If we got here, the path is correct. So, we should update ADDR.
    strcpy(LocalPath,tmp);
    if(Log == 1)
        printf("Final path (after CD): '%s'",LocalPath);
    closedir(dir);
    free(tmp);
    return 1;
}

char* ls(char* path, char* args) { // generic ls
    //Receives a path and arguments as param and returns a char* with the data requested
    //it has \n separator for every file
    DIR *dir;
    struct dirent *file;
    char *res;
    char this[1024]; // Aux variable

    res = malloc(sizeof(char));
    strcpy(res, "");

    if(Log == 1)
        puts(args);

    dir = opendir(path);
    if(dir == NULL) {
        printf("(ls_la) Error opening directory: %s\n", strerror(errno));
        return "";
    }

    if((strcmp(args,"") == 0) || (strcmp(args,"a") == 0)) {
        int total_length = 1;
        strcpy(this, "");            // Start the buffer
        while((file = readdir(dir))) {
            strcpy(this, "");
            if (strcmp(args,"") == 0) {
                if(file->d_name[0] != '.' &&
                    file->d_name[strlen(file->d_name) - 1] != '~') { // Eliminating hidden files and files ending in "~"
                    strcat(this, file->d_name);
                    strcat(this, "\n");
                }
            } else {
                strcat(this, file->d_name);
                strcat(this, "\n");
            }
            total_length += strlen(this);
            //printf("total length: %d\n", total_length);
            if((res = realloc(res,total_length)) == NULL) {
                printf("(ls_la) Unable to allocate memory.");
                exit(-1);
            }
            strcat(res,this);
        }
    }

    //else if(args == "l" || args == "la" || args == "al") {
    else if((strcmp(args,"l") == 0) || (strcmp(args,"la") == 0) || (strcmp(args,"al") == 0)) {
        char *fileName, aux[64], timebuf[64];
        struct stat fileStat;
        struct tm lt;
        struct passwd *pwd;
        struct group *grp;

        fileName = malloc(sizeof(char) * 1024); // max size of filename

        int total_length = 1;

        while((file = readdir(dir)) != NULL) {
            if (( (strcmp(args,"l") == 0) && (file->d_name[0] != '.' && file->d_name[strlen(file->d_name) - 1] != '~'))
                    || (strcmp(args, "la") == 0) || (strcmp(args, "al") == 0)) { // Eliminating hidden files and files ending in "~")
                strcpy(fileName, path);
                strcat(fileName,file->d_name);
                if(stat(fileName, &fileStat) != 0) {
                    printf("(ls) Erro na syscall stat!\n");
                    return "";
                }
                strcpy(this, ""); // reseting

                // Permissions
                strcat(this,(S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                strcat(this,(fileStat.st_mode & S_IRUSR) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWUSR) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXUSR) ? "x" : "-");
                strcat(this,(fileStat.st_mode & S_IRGRP) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWGRP) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXGRP) ? "x" : "-");
                strcat(this,(fileStat.st_mode & S_IROTH) ? "r" : "-");
                strcat(this,(fileStat.st_mode & S_IWOTH) ? "w" : "-");
                strcat(this,(fileStat.st_mode & S_IXOTH) ? "x | " : "-\'");

                // Number of Hardlinks
                sprintf(aux, "%d", (int)fileStat.st_nlink);
                strcat(this,aux);
                strcat(this,"\'");

                // File Owner
                pwd = getpwuid(fileStat.st_uid);
                if(pwd != 0) {
                    strcat(this,pwd->pw_name);
                    strcat(this,"\'");
                }

                // File Group
                grp = getgrgid(fileStat.st_gid);
                if(grp != 0) {
                    strcat(this,grp->gr_name);
                    strcat(this,"\'");
                }

                // File Size
                sprintf(aux, "%d",(int)fileStat.st_size);
                strcat(this,aux);
                strcat(this,"\'");

                // Modified Date
                time_t t = fileStat.st_mtime;
                localtime_r(&t, &lt);
                strftime(timebuf, sizeof(timebuf), "%c", &lt);
                strcpy(aux,timebuf+4);
                strcat(this,aux);
                strcat(this,"\'");

                // File Name
                strcat(this,file->d_name);
                strcat(this,"\n");

                // Concatenate into response
                total_length += strlen(this);
                //printf("total length: %d\n", total_length);
                if((res = realloc(res,total_length)) == NULL) {
                    printf("(ls_la) Unable to allocate memory.");
                    exit(-1);
                }
                strcat(res,this);
            }
        }
        printf("Complete. Sending result... \n");
    }

    else {
        printf("Invalid arguments \n");
        return "";
    }

    closedir(dir);
    return res;
}

void print_ls(char* data) {
    int i,len = strlen(data);
    for(i=0; i<len; i++) {
        if(data[i] != '\'')
            printf("%c",data[i]);
        else
            printf("   ");
    }
}