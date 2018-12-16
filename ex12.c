
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <libio.h>

typedef struct student{
    char path[100];
    char pathToCFile[100];
    char name[40];
    char reason_of_grade[45];
    int grade;
}student;

int static numOfStudents = 0;

//checks if its a directory or just a file
int reread(const char *path){
    struct stat statbuf;
    if(stat(path, &statbuf) != 0)
        return 0;
    return S_ISDIR(statbuf.st_mode);
}

//checks if it has a "c" ext
const char *c_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

//checks if there is a c file inside the folder
 char* has_c_file(char* path2Dir){
    if(reread(path2Dir)){
        DIR *dir;
        struct dirent *dirnt;
        student* students = (student*)malloc(sizeof(student));
        if ((dir = opendir (path2Dir)) == NULL) {
            perror ("cannot open directory");
            exit(1);
        }
        else {
            while ((dirnt = readdir (dir)) != NULL) {
                if(strcmp(dirnt->d_name,".") && strcmp(dirnt->d_name,"..")){
                    char* path = (char*)malloc(strlen(path2Dir)+strlen(dirnt->d_name)+2);
                    strcpy(path,path2Dir);
                    strcat(path,"/");
                    strcat(path,dirnt->d_name);
                    if(!reread(path)){ 
                        if(!strcmp(c_ext(path),"c")){
                            return path;
                        }
                    }
                    else{
                        char* returnValue = has_c_file(path);
                        if(strcmp(returnValue,"NO_C_FILE"))
                            return returnValue;
                    }
                    free(path);
                }
            }
            closedir (dir);
        }
    }
    return "NO_C_FILE";
}

void check(student* students){
    for(size_t i = 0; i < numOfStudents; i++){
        strcpy((students + i)->pathToCFile, has_c_file((students + i)->path));
        if(!strcmp((students + i)->pathToCFile, "NO_C_FILE")){
            strcpy((students + i)->reason_of_grade,"NO_C_FILE");
        }
    }
}

void compile(student* students){
    int stat;
    for(size_t i = 0 ; i< numOfStudents; i++){
        if(strcmp((students + i)->pathToCFile,"NO_C_FILE")){
            pid_t pid = fork();
            int pidStatus;
            if(pid){
                waitpid(pid,&pidStatus,0);
                if (WIFEXITED(pidStatus)&& WEXITSTATUS(pidStatus)) {
                    strcpy((students + i)->reason_of_grade,"COMPILATION_ERROR");
                    (students + i)->grade = 0;
                }
            }
            else if(pid < 0){
                perror("ERROR cant fork");
                exit(1);
            }
            else if(!pid){
                char name[25] = "";
                strcpy(name,(students + i)->name);
                strcat(name,".out");
                char *args[]={"gcc",(students + i)->pathToCFile,"-o",name,NULL};
                execvp(args[0],args);
            }
            wait(&stat);
            if(stat == 0){
                printf("compiled!\n");
            }
        }
    }
}

//inserts the input file into the plus.c of the students
void insertAndExec(student* students, char* inputFile){
    for(size_t i = 0 ; i< numOfStudents; i++){      
        if(!strcmp((students + i)->reason_of_grade,"")){//if student has no comment yet
            pid_t pid = fork();
            int pidStatus;
            if(pid){
                waitpid(pid,&pidStatus,0);
            }
            else{
                char cmd[25] = "./";
                strcat(cmd,(students + i)->name);
                strcat(cmd,".out");
                char filename[25] = "";
                strcpy(filename,(students + i)->name);
                strcat(filename,".txt");
                char a[80], b[80];

                int inp = open(inputFile,O_RDONLY,0);
                int out = open(filename, O_WRONLY | O_CREAT, S_IRUSR| S_IWUSR);
                if (inp < 0){
                    perror("Error : cannot open input file");
                    exit(1);
                }
                if (out < 0){
                    perror("Error : cannot open output file");
                    exit(1);
                }
                if ((dup2(inp,0)) < 0){
                    perror("Error : cannot redirect");
                    exit(1);
                }
                if ((dup2(out,1)) < 0){
                    perror("Error : cannot redirect");
                    exit(1);
                }

                char *args[]={cmd,NULL};
                execvp(args[0],args);
                scanf("%s" , a);
                scanf("%s" , b);
                printf("%s\n%s\n",a, b);
                if(close(inp) < 0){
                    perror("Error : file cannot close");
                    exit(1);
                }
                if(close(out) < 0){
                    perror("Error : file cannot close");
                    exit(1);
                }

            }
        }
    }
}

//compares the output of the students plus.c and the output of the confFile.txt
void compareOutputs(student* students, char* outputFile){
    for(size_t i = 0 ; i< numOfStudents; i++){
        if(!strcmp((students + i)->reason_of_grade,"")){
            pid_t child_pid = fork();
            int childSt;
            char studentOutput[25] = "";
            strcpy(studentOutput,(students + i)->name);
            strcat(studentOutput,".txt");
            int outNeeded = open(outputFile,O_RDONLY,0);
            if(child_pid){
                waitpid(child_pid,&childSt,0);
                if (WIFEXITED(childSt)) {
                    if(WEXITSTATUS(childSt) == 1){
                        strcpy((students + i)->reason_of_grade,"BAD_OUTPUT");
                    }
                    else if(WEXITSTATUS(childSt) == 2){
                        strcpy((students + i)->reason_of_grade,"GREAT_JOB");
                        (students + i)->grade = 100;
                    }
                    else{
                        perror("Error: this is not 1 and not 2");
                        exit(1);
                    }
                    char cmpP[40] = "";
                    strcat(cmpP,(students + i)->name);
                    strcat(cmpP,".out");
                   if(unlink(studentOutput) < 0){
                        perror("Error : file cannot deleted");
                        exit(1);
                    }
                    if(unlink(cmpP) < 0){
                        perror("Error : file cannot deleted");
                        exit(1);
                    }
                }
            }
            else{
                char *args[]={"./comp.out",studentOutput,outputFile,NULL};
                execvp(args[0],args);
            }
        }
    }
}

//writes csv file
void writeCsv(student* students){
    int csv_FD = open("results.csv", O_APPEND | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if(csv_FD < 0){
        perror("Error : cannot open results.csv");
        exit(1);
    }
    for(size_t i = 0 ; i< numOfStudents; i++){
        char line[50]="";
        char student_num[5];
        sprintf(student_num,"%d",(students + i)->grade);
        strcat(line,(students + i)->name);
        strcat(line,", ");
        strcat(line,student_num);
        strcat(line,", ");
        strcat(line,(students + i)->reason_of_grade);
        strcat(line,"\n");
        if(!write(csv_FD,line,strlen(line))){
            perror("Error : cannot write to results.csv");
            exit(1);
        }
    }
    if(close(csv_FD) < 0){
        perror("Error : File cannot close");
        exit(1);
    }
}

int main(int argc, char* argv[]){
    if(argc != 2){
        perror("error: have more than 1 path\n");
        exit(1);
    }
    else{
       int fd = open(argv[1], O_RDONLY);
        char path[80], input[80], output[80];
        if(fd < 1){
            perror("open error\n");
            exit(1);
        }
        if((fd = dup2(fd, 0)) < 0){
            perror("error: cannt redirect");
            exit(1);
        }
        scanf("%s" , path);
        scanf("%s" , input);
        scanf("%s" , output);

        DIR *dir = opendir(path);
        DIR *dirIn;
        struct dirent *folder;
        struct dirent *file;
        char inDirName[80];
        student* students = (student*)malloc(sizeof(student));
        int i = 0;
        if(reread(path)){
            while((folder = readdir(dir)) != NULL){
                if(!strcmp(folder->d_name, ".") || (!strcmp(folder->d_name, "..")))
                    continue;
                else{
                    printf("name= %s\n",folder->d_name);
                    strcpy(inDirName,path);
                    strcat(inDirName,"/");
                    strcat(inDirName,folder->d_name);
                    numOfStudents++;
                    dirIn = opendir(inDirName);
                    if(reread(inDirName)){
                        strcpy((students + i)->name, folder->d_name);
                        printf("inside name student = %s\n",(students + i)->name);
                        strcpy((students + i)->path, inDirName);
                        printf("inside path student = %s\n",(students + i)->path);
                        i++;
                        students = (student*)realloc(students,((i+1)*sizeof(student)));
                    }
                }
            }
        }
        closedir(dirIn);
        closedir(dir);
        printf("num %d\n",numOfStudents );
        check(students);
        compile(students);
        insertAndExec(students,input);
        compareOutputs(students,output);
        writeCsv(students);
        free(students);
       
        if(close(fd)<0){
            perror("Error : file cannot closed");
            exit(1);
        }
     }
    return 0;
}