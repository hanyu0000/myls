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
                default:
                    perror("ls:不适用的选项\n请尝试执行 \"ls --help\" 来获取更多信息");
                }
            }
        }
    }
    // 遍历目录名，针对每个目录做ls操作
    int flag = 0;
    for (int i = 1; i < argc; i++){
        if (argv[i][0] != '-'){
            flag = 1;
            printf("%s:\n", argv[i]);
            do_ls(argv[i]);
        }
    }
    if (flag == 0)
        do_ls(".");
    printf("\n");
    return 0;
}
void do_ls(char *dirname){
    Fileinfo fileinfo[4096];
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
            fileinfo[file_cnt++].filename = cur_dirent->d_name;
        }
    }
    // 存储信息
    for (int i = 0; i < file_cnt; i++){
        char pathname[256];
        strcpy(pathname, dirname);
        strcat(pathname, "/");
        strcat(pathname, fileinfo[i].filename);
        if (lstat(pathname, &fileinfo[i].info) == -1){
            perror("获取信息失败");
            exit(EXIT_FAILURE);
        }
    }
    // 排序
    if (has_t)
        qsort(fileinfo, file_cnt, sizeof(Fileinfo), compare_t);
    else
        qsort(fileinfo, file_cnt, sizeof(Fileinfo), compare);
    if (has_r){
        int left = 0, right = file_cnt - 1;
        while (left < right){
            Fileinfo temp = fileinfo[left];
            fileinfo[left++] = fileinfo[right];
            fileinfo[right--] = temp;
        }
    }
    // 打印信息
    for (int i = 0; i < file_cnt; i++)
        print_fileinfo(fileinfo[i]);
    if (has_R){
        for (int i = 0; i < file_cnt; i++){
            if (S_ISDIR(fileinfo[i].info.st_mode)){
                if (!strcmp(fileinfo[i].filename, ".") || !strcmp(fileinfo[i].filename, ".."))
                    continue;
                char pathname[256];
                strcpy(pathname, dirname);
                strcat(pathname, "/");
                strcat(pathname, fileinfo[i].filename);
                printf("\n%s: \n", pathname);
                do_ls(pathname);
            }
        }
    }
    closedir(dir_ptr);
}
//比较函数
int compare(const void *a, const void *b){
    Fileinfo *_a = (Fileinfo *)a;
    Fileinfo *_b = (Fileinfo *)b;
    return strcmp(_a->filename, _b->filename);
}
int compare_t(const void *a, const void *b){
    Fileinfo *_a = (Fileinfo *)a;
    Fileinfo *_b = (Fileinfo *)b;
    return _a->info.st_mtime < _b->info.st_mtime;
}
// 将权限转换为字符串
void mode_letters(mode_t num, char *mode) {
    strcpy(mode, "----------");
    switch (num & __S_IFMT){
    case __S_IFREG: /* Regular file.  */
        mode[0] = '-';
        break;
    case __S_IFDIR: /* Directory.  */
        mode[0] = 'd';
        break;
    case __S_IFCHR: /* Character device.  */
        mode[0] = 'c';
        break;
    case __S_IFBLK: /* Block device.  */
        mode[0] = 'b';
        break;
    case __S_IFIFO: /* FIFO.  */
        mode[0] = 'p';
        break;
    case __S_IFSOCK: /* Socket.  */
        mode[0] = 's';
        break;
    case __S_IFLNK: /* Symbolic link.  */
        mode[0] = 'l';
        break;
    }
    // 权限-i
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
        printf("%-8lu ", fileinfo.info.st_ino);
    if (has_s)
        printf("%-8ld ", (long)fileinfo.info.st_size/1024);
    if (has_l){
        char mode[11];
        mode_letters(fileinfo.info.st_mode, mode);
        printf("%s ", mode);

        printf("%-2d ", (int)fileinfo.info.st_nlink); // 打印链接数

        struct passwd *user;
        user = getpwuid(fileinfo.info.st_uid);
        printf("%s ", user->pw_name); // 打印用户名

        struct group *gp;
        gp = getgrgid(fileinfo.info.st_gid);
        printf("%s ", gp->gr_name); // 打印组名

        printf("%-10ld ", fileinfo.info.st_size);             // 打印文件大小
        printf("%.12s ", ctime(&fileinfo.info.st_mtime) + 4); // 打印时间
    }

    print_filename(fileinfo.filename, fileinfo.info.st_mode);
    printf("\n");
}
// 染色文件名
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