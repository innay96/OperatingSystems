#include<stdio.h>
#include<stdlib.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int file1, file2;
    char buf_file1, buf_file2;
    if(argc != 3){
        perror("doesnt have 2 files\n");
        return 1;
    }
    file1 = open(argv[1], O_RDONLY);
    file2 = open(argv[2], O_RDONLY);
    if(file1 < 1 || file2 < 1){
        perror("open error\n");
        return 1;
    }
    int read1 = read(file1, &buf_file1, 1);
    int read2 = read(file2, &buf_file2, 1);
    while(read1 == 1 && read2 == 1){
        if(buf_file1 != buf_file2){
            printf("not equal\n");
            return 1;
        }
        read1 = read(file1, &buf_file1, 1);
        read2 = read(file2, &buf_file2, 1);
    }
    if(read1 == -1 || read2 == -1){
        perror("read error\n");
        return 1;
    }
        
    if(read1 != read2){
        printf("not equal\n");
        return 1;
    }
    printf("they are equal\n");
    return 2;

}