char* ls_la(char* param);

//------------
// Path stuff.

int count_slashes(unsigned char* c, int len) {
    int i,count=0;
    for(i=0; i<len; ++i) {
        if(c[i] == '/') {
            count++;
        }
    }
    return count;
}

void print_matrix(unsigned char** c,int n) {
    int i;
    for(i=0; i<n; i++) {
        printf("\t");
        puts(c[i]);
    }
}

unsigned char** to_matrix(unsigned char *c, int n) {
    int i,j=0,k;
    unsigned char **tmp = malloc(sizeof(unsigned char*) * n);
    for(i=0; i<n; i++) {
        tmp[i] = malloc(sizeof(unsigned char*) * FILE_LEN);
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

unsigned char* to_vector(unsigned char **c, int n) {
    int i,size = 1024;
    unsigned char *tmp = malloc(sizeof(unsigned char) * size);
    tmp[0] = '\0';
    for(i=0; i<n; i++) {
        if(strlen(tmp) + strlen(c[i]) > size) { // To prevent overflows.
            size = size * 2;
            tmp = realloc(tmp,sizeof(unsigned char) * size);
        }
        strcat(tmp,c[i]);
        strcat(tmp,"/");
    }
    return tmp;
}

unsigned char* fix_dir(unsigned char *c, int length) {
    int i,j,n = count_slashes(c,length);
    unsigned char **tmp;
    tmp = to_matrix(c,n);
    // Now I have a matrix from the path, with n lines and unknown number of columns.
    for(i=1; i<n; i++) { // If tmp[0] == ../, theres nothing I can do with it.
        if(strcmp(tmp[i],"..") == 0 && strcmp(tmp[i-1],"..") != 0) {
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            for(j=i; j<n; j++) {
                tmp[j-1] = tmp[j];
            }
            n--;
            i = 1; // So a/b/c/../../ will remove b and c and both ../, not only c.
        }
    }
    c = to_vector(tmp,n);
    return c;
}
/*
int main() {
    unsigned char *addr = malloc(sizeof(char) * 1024);
    int i,n;
    scanf("%d\n",&n);
    for(i=0; i<n; i++) {
        scanf("%s",addr);
        addr = fix_dir(addr,strlen(addr));
        printf("Addr: ");
        puts(addr);
    }
}
*/

int check_cd(unsigned char* c) {
/* This function will concatenate the path received with the global variable ADDR,
 * then it will fix it (every ../../) and will try to open this directory. If it does
 * not exist, or you do not have permission (which I doubt, cause you are sudo), error
 * (0) shall be returned, otherwise, (1) will be returned.
 */
 // Should this use LocalPath, RemPath or not hardcoded?
    DIR *dir;
    char *tmp;
    if((tmp = malloc(sizeof(unsigned char) * 1024)) == NULL)
        error("(check_cd) Error allocating memory.");
    //strcpy(tmp,ADDR);
    strcat(tmp,c);
    fix_dir(tmp,strlen(tmp));
    dir = opendir(tmp);
    if(dir == NULL) {
        // Problem. Server should send an error message (outside this function!).
        puts("(check_cd) Could not open this directory - it does not exit or you do not have permission.");
        free(tmp);
        return 0;
    }
    // If we got here, the path is correct. So, we should update ADDR.
    //strcpy(ADDR,tmp);
    closedir(dir);
    free(tmp);
    return 1;
}

/* LS Stuff */

int get_files(char *path, char *c) {
    DIR *dp;
    struct dirent *ep;
    dp = opendir(path);
    if(dp != NULL) {
        c = strcpy(c, ""); // Starting the buffer with something to use strcat.
        while(ep = readdir(dp)) {
            strcat(c, ep->d_name);
            strcat(c, "\'");
        }
        (void)closedir(dp);
        return 1;
    } else {
        puts("(get_files) Error! Could not open the directory");
        return 0;
    }
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

    puts(args);

    dir = opendir(path);
    if(dir == NULL) {
        printf("(ls_la) Error opening directory: %s\n", strerror(errno));
        return strerror(errno);
    }

    //if((dir[strlen(dir) -1]) != '/') { // To correct directory name //BEFORE OR AFTER
    //    strcat(dir, "/");
    //}

    //if(args == "" || args == "a") {       // No arguments or -a
    if((strcmp(args,"") == 0) || (strcmp(args,"a") == 0)) {
        int total_length = 1;
        strcpy(this, "");            // Start the buffer
        while(file = readdir(dir)) {
            strcpy(this, "");
            if (args == "") {
                if(file->d_name[0] != '.' &&
                    file->d_name[strlen(file->d_name) - 1] != '~') {
                    // Eliminating hidden files and files ending in "~"
                    strcat(this, file->d_name);
                    strcat(this, "\n");
                }
            }
            else {
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
            if ((args == "l" && (file->d_name[0] != '.' && file->d_name[strlen(file->d_name) - 1] != '~'))
                    || args == "la" || args == "al") { // Eliminating hidden files and files ending in "~")
                strcpy(fileName, path);
                strcat(fileName,file->d_name);
                printf("(ls) filename = %s \n", fileName);
                if(stat(fileName, &fileStat) != 0) {
                    printf("(ls) Erro na syscall stat!\n");
                    return;
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
                printf("total length: %d\n", total_length);
                if((res = realloc(res,total_length)) == NULL) {
                    printf("(ls_la) Unable to allocate memory.");
                    exit(-1);
                }
                strcat(res,this);
            }
        }
    }

    else {
        printf("Invalid arguments \n");
        return;
    }

    closedir(dir);
    return res;
}

void print_ls(char* data) {
    int i,len = strlen(data);
    for(i=0; i<len; i++) {
        printf("%c",data[i]);
    }
}

char* ls_la(char* param) {
    // Receives a path as param and returns a char* with all the ls -la data.
    // Ps: It has a \n separator for every file.
    char *fileName,aux[64],timebuf[64],this[1024],*res;
    int totalLength = 1;
    DIR *dir;
    struct dirent *file;
    struct stat fileStat;
    struct tm lt;
    struct passwd *pwd;
    struct group *grp;

    fileName = malloc(sizeof(char) * 1024);
    res = malloc(sizeof(char));

    if((dir = opendir(param)) == NULL) {
        printf("(ls_la) Error opening directory: %s\n",strerror(errno));
        return strerror(errno);
    }

    if((param[strlen(param) -1]) != '/') { // To correct directory name
        strcat(param,"/");
    }

    while((file = readdir(dir)) != NULL)
    {
        strcpy(fileName,param);
        strcat(fileName,file->d_name);
        if(stat(fileName, &fileStat) != 0)
            printf("(ls_la) Erro na syscall stat!\n");
        this[0] = '\0';
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
        strcat(this,(fileStat.st_mode & S_IXOTH) ? "x | " : "- | ");

        // Number of Hardlinks
        sprintf(aux, "%d", (int)fileStat.st_nlink);
        strcat(this,aux);
        strcat(this," | ");

        // File Owner
        pwd = getpwuid(fileStat.st_uid);
        if(pwd != 0) {
            strcat(this,pwd->pw_name);
            strcat(this," | ");
        }

        // File Group
        grp = getgrgid(fileStat.st_gid);
        if(grp != 0) {
            strcat(this,grp->gr_name);
            strcat(this," | ");
        }

        // File Size
        sprintf(aux, "%d",(int)fileStat.st_size);
        strcat(this,aux);
        strcat(this," | ");

        // Modified Date
        time_t t = fileStat.st_mtime;
        localtime_r(&t, &lt);
        strftime(timebuf, sizeof(timebuf), "%c", &lt);
        strcpy(aux,timebuf+4);
        strcat(this,aux);
        strcat(this," | ");

        // File Name
        strcat(this,file->d_name);
        strcat(this,"\n");

        // Concatenate into response
        totalLength += strlen(this);
        if((res = realloc(res,totalLength)) == NULL) {
            printf("(ls_la) Unable to allocate memory.");
            exit(-1);
        }
        strcat(res,this);
    }
    closedir(dir);
    return res;
}