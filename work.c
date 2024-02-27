#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
/*
DIR结构体：DIR结构体用于表示目录流。它是一个不透明的结构，通常由目录操作函数返回。在程序中，你不需要直接访问它的成员，只需将它作为参数传递给相关的目录操作函数即可。
  d_ino：    inode 号码，是文件系统中唯一标识文件或目录的值。
  d_off：    目录项在目录文件中的偏移量。
  d_reclen： 目录项长度。
  d_type：   文件类型。
  d_name：   文件或目录的名称，是一个以 null 结尾的字符串。
struct dirent结构体：struct dirent结构体用于存储目录中的一个边界的信息。它包含了文件或子目录的名称等信息。
opendir函数：opendir函数用于打开一个目录，并返回一个指向DIR结构体的指针。
readdir函数：readdir函数用于读取目录流中的下一个边界，并返回一个指向struct dirent结构体的指针。
closedir函数：closedir用于关闭目录流的函数，释放相关的资源。
*/
typedef struct{
    char *filename;
    struct stat info;
} Fileinfo;
void do_ls();
void list_files(const char *dirname, int has_a, int has_t, int has_r, int has_R);
int compare();
int compare_t();
void print_fileinfo();
void print_filename();
int has_a = 0;
int has_l = 0;
int has_R = 0;
int has_t = 0;
int has_r = 0;
int has_i = 0;
int has_s = 0; 
int main(int argc, char **argv){
    for (int i = 1; i < argc; i++){
        if (argv[i][0] == '-'){
            for (int j = 1; j < strlen(argv[i]); j++){
                switch (argv[i][j]){
                case 'a':
                    has_a = 1;
                    break;
                case 'l':
                    has_l = 1;
                    break;
                case 'R':
                    has_R = 1;
                    break;
                case 't':
                    has_t = 1;
                    break;
                case 'r':
                    has_r = 1;
                    break;
                case 'i':
                    has_i = 1;
                    break;
                case 's':
                    has_s = 1;
                    break;
                }
            }
        }
    }
    int flag = 0;
    for (int i = 1; i < argc; i++){
        if (argv[i][0] != '-'){
            flag = 1;
            char path[256];
            realpath(argv[i],path);
            do_ls(path);
        }
    }
    if (flag == 0)
        do_ls(".");

    printf("\n");
    return 0;
}
void do_ls(char *dirname){
    Fileinfo *fileinfo = malloc(sizeof(Fileinfo) * 10000);
    if (fileinfo == NULL){
        perror("内存分配失败");
        exit(EXIT_FAILURE);
    }
    int file_cnt = 0;
    DIR *dir_ptr;
    struct dirent *cur_dirent;
    if ((dir_ptr = opendir(dirname)) == NULL){
        perror("打开文件夹失败");
        exit(EXIT_FAILURE);
    }
    else{
        while ((cur_dirent = readdir(dir_ptr)) != NULL){
            if (!has_a && *(cur_dirent->d_name) == '.')
                continue;
            fileinfo[file_cnt++].filename = strdup(cur_dirent->d_name);
        }
    }
    for (int i = 0; i < file_cnt; i++){
        char pathname[512];
        snprintf(pathname, sizeof(pathname), "%s/%s", dirname, fileinfo[i].filename); 
        if (lstat(pathname, &fileinfo[i].info) == -1){
            perror("获取信息失败");
            continue;
        }
    }
    if (has_r){
        int left = 0, right = file_cnt - 1;
        while (left < right){
            Fileinfo temp = fileinfo[left];
            fileinfo[left++] = fileinfo[right];
            fileinfo[right--] = temp;
        }
    }
    if (has_t)
        qsort(fileinfo, file_cnt, sizeof(Fileinfo), compare_t);
    else
        qsort(fileinfo, file_cnt, sizeof(Fileinfo), compare);
    
    for (int i = 0; i < file_cnt; i++)
        print_fileinfo(fileinfo[i]);
    if (has_R){
        list_files(dirname, has_a, has_t, has_r, has_R);
    }
    closedir(dir_ptr);
    
    for (int i = 0; i < file_cnt; ++i)
        free(fileinfo[i].filename);
    free(fileinfo);
}
void list_files(const char *dirname, int has_a, int has_t, int has_r, int has_R) {
    DIR *dir_ptr;
    struct dirent *cur_dirent;

    if ((dir_ptr = opendir(dirname)) == NULL) {
        perror("打开文件夹失败");
        exit(EXIT_FAILURE);
    }

    while ((cur_dirent = readdir(dir_ptr)) != NULL) {
        char pathname[512];
        snprintf(pathname, sizeof(pathname), "%s/%s", dirname, cur_dirent->d_name);

        if (!has_a && *(cur_dirent->d_name) == '.')
            continue;

        struct stat statbuf;
        if (lstat(pathname, &statbuf) == -1) {
            perror("获取信息失败");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(cur_dirent->d_name, ".") != 0 && strcmp(cur_dirent->d_name, "..") != 0) {
                printf("\n%s:\n", pathname);
                do_ls(pathname);
            }
        }
    }
    closedir(dir_ptr);
}
int compare(const void *a, const void *b){
    Fileinfo *i = (Fileinfo *)a;
    Fileinfo *j = (Fileinfo *)b;
    return strcmp(i->filename, j->filename);
}
int compare_t(const void *a, const void *b){
    Fileinfo *i = (Fileinfo *)a;
    Fileinfo *j = (Fileinfo *)b;
    return i->info.st_mtime < j->info.st_mtime;
}
void mode_letters(mode_t num, char *mode) {
    strcpy(mode, "----------");
    switch (num & __S_IFMT){
    case __S_IFREG: 
        mode[0] = '-';
        break;
    case __S_IFDIR: 
        mode[0] = 'd';
        break;
    case __S_IFCHR: 
        mode[0] = 'c';
        break;
    case __S_IFBLK: 
        mode[0] = 'b';
        break;
    case __S_IFIFO:
        mode[0] = 'p';
        break;
    case __S_IFSOCK: 
        mode[0] = 's';
        break;
    case __S_IFLNK:
        mode[0] = 'l';
        break;
    }
    if (num & S_IRUSR)
        mode[1] = 'r';
    if (num & S_IWUSR)
        mode[2] = 'w';
    if (num & S_IXUSR)
        mode[3] = 'x';
    if (num & S_IRGRP)
        mode[4] = 'r';
    if (num & S_IWGRP)
        mode[5] = 'w';
    if (num & S_IXGRP)
        mode[6] = 'x';
    if (num & S_IROTH)
        mode[7] = 'r';
    if (num & S_IWOTH)
        mode[8] = 'w';
    if (num & S_IXOTH)
        mode[9] = 'x';
    mode[10] = '\0';
}
void print_fileinfo(const Fileinfo fileinfo){
    if (has_i)
        printf("%-8lu  ", fileinfo.info.st_ino);
    if (has_s)
        printf("%-8ld  ", (long)fileinfo.info.st_size/1024);
    if (has_l){
        char mode[11];
        mode_letters(fileinfo.info.st_mode, mode);
        printf("%s  ", mode);
        printf("%-2d  ", (int)fileinfo.info.st_nlink);
        struct passwd *user;
        user = getpwuid(fileinfo.info.st_uid);
        printf("%s  ", user->pw_name); 
        struct group *gp;
        gp = getgrgid(fileinfo.info.st_gid);
        printf("%s  ", gp->gr_name);
        printf("%-10ld  ", fileinfo.info.st_size);             
        printf("%.12s  ", ctime(&fileinfo.info.st_mtime) + 4); 
    }
    print_filename(fileinfo.filename, fileinfo.info.st_mode);
    printf("\n");
}
void print_filename(char *filename, mode_t filemode) {
    if (S_ISDIR(filemode))
        printf("\033[01;34m%s\033[0m", filename);
    else if (S_ISCHR(filemode))
        printf("\033[40;33m%s\033[0m", filename);
    else if (S_ISBLK(filemode))
        printf("\033[40;33m%s\033[0m", filename);
    else if (S_ISLNK(filemode))
        printf("\033[30;42m%s\033[0m", filename);
    else if (S_ISREG(filemode)){
        if (filemode & S_IXUSR || filemode & S_IXGRP || filemode & S_IXOTH)
            printf("\033[01;32m%s\033[0m", filename);
        else
            printf("%s", filename);
    }
    else
        printf("%s", filename);
}