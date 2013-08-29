/* 
 * xmuliushuo
 * xmuliushuo@gmail.com
 * xmuliushuo.com
 */
#include "apue.h"
#include <dirent.h>
#include <limits.h>
#include <fcntl.h>
#include <libgen.h>

//#define DEBUG

typedef int Myfunc(const char *, const struct stat *, int);
static Myfunc myfunc, myfunc2, myfunc3;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *);

static long nreg, nreg_small, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
static const int filelen = 4096; /* 统计小于filelen的文件个数 */
static const int bufsize = 4096; /* 读取文件的缓冲区大小 */
static struct stat filestat; /* -comp 用于保存输入文件的信息 */
static char *filename; /* -comp 输入文件的路径 */
static int strnum; /* -name 输入的文件名个数 */
static char **str;/* -name 指向输入的文件名 */

int main(int argc, char *argv[])
{
    int ret;
    if (argc == 2) { 
        /* 输出各类文件数 */
        ret = myftw(argv[1], myfunc);
        ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock + ntot;
        if (ntot == 0)
            ntot = 1;
        printf("regular files    \t= %7ld, %5.2f %%\n", nreg, nreg * 100.0 / ntot);
        printf("regular files that\n");
        printf("no larger than %d\t= %7ld, %5.2f %%\n", filelen, nreg_small, nreg_small * 100.0 / ntot);
        printf("directories      \t= %7ld, %5.2f %%\n", ndir, ndir * 100.0 / ntot);
        printf("block special    \t= %7ld, %5.2f %%\n", nblk, nblk * 100.0 / ntot);
        printf("char special     \t= %7ld, %5.2f %%\n", nchr, nchr * 100.0 / ntot);
        printf("FIFOs            \t= %7ld, %5.2f %%\n", nfifo, nfifo * 100.0 / ntot);
        printf("symbolic links   \t= %7ld, %5.2f %%\n", nslink, nslink * 100.0 / ntot);
        printf("sockets          \t= %7ld, %5.2f %%\n", nsock, nsock * 100.0 / ntot);
    }
    else if (argc == 4 && (strcmp(argv[2], "-comp") == 0)) {
        /* 输出在<pathname>目录子树之下，
        所有与<filename>文件内容一致的文件的绝对路径名 */
        if (lstat(argv[3], &filestat) < 0) {
            err_quit("stat error");
        }
        filename = argv[3];
        ret = myftw(argv[1], myfunc2);
    }
    else if (argc >= 4 && (strcmp(argv[2], "-name") == 0)) {
        /* 命令输出在<pathname>目录子树之下，
        所有与<str>…序列中文件名相同的文件的绝对路径名。*/
        strnum = argc - 3;
        str = &argv[3];
        ret = myftw(argv[1], myfunc3);   
    }
    else {
        err_quit("usage: myfind  <pathname>  [-comp <filename> | -name <str>...]");
    }
    exit(ret);
}
#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4 /* file that we can't stat */

static char *fullpath;

static int myftw(char *pathname, Myfunc *func)
{
    int len;
    fullpath = path_alloc(&len);
    strncpy(fullpath, pathname, len);
    fullpath[len - 1] = 0;
    return (dopath(func));
}

static int dopath(Myfunc *func)
{
    struct stat statbuf;
    struct dirent *dirp;
    DIR *dp;
    int ret;
    char *ptr;

    if (lstat(fullpath, &statbuf) < 0)
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0) /* 不是目录 */
        return(func(fullpath, &statbuf, FTW_F));
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return ret;

    ptr = fullpath + strlen(fullpath);
    *ptr++ = '/';
    *ptr = 0;

    if ((dp = opendir(fullpath)) == NULL)
        return(func(fullpath, &statbuf, FTW_DNR));

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;
        strcpy(ptr, dirp->d_name);
        if ((ret = dopath(func)) != 0)
            break;
    }
    ptr[-1] = 0;
    closedir(dp);
    //if (closedir(dp) < 0)
        //err_ret("can't close directoty %s", fullpath);
    return ret;
}

static int myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:
            nreg++;
            /* 普通文件，判断是否小于等于4096 */
            if (statptr->st_size <= filelen)
                nreg_small++;
            break;
        case S_IFBLK: nblk++; break;
        case S_IFCHR: nchr++; break;
        case S_IFIFO: nfifo++; break;
        case S_IFLNK: nslink++; break;
        case S_IFSOCK: nsock++; break;
        case S_IFDIR:
            /* 目录的话应该是FTW_D */
            //err_dump("for S_IFDIR for %s", pathname);
            break;
        }
    case FTW_D: ndir++; break;
    case FTW_DNR:
        //err_ret("can't read directroy %s", pathname);
        break;
    case FTW_NS:
        //err_ret("stat error for %s", pathname);
        break;
    default:
        //err_dump("unkown type %d for pathname %s", type, pathname);
        break;
    }
    return 0;
}
/* 
 * 
 */
static int myfunc2(const char *pathname, const struct stat *statptr, int type)
{
    int fd1, fd2;
    char *buf1, *buf2;
    char *resolved_path = path_alloc(NULL);
    int i;
    buf1 = malloc(sizeof(char) * bufsize);
    buf2 = malloc(sizeof(char) * bufsize);
    if ((type == FTW_F) 
        && ((statptr->st_mode & S_IFMT) == S_IFREG) 
        && (filestat.st_size == statptr->st_size)) {
        /* 文件大小相等，比较文件内容 */
        if ((fd1 = open(filename, O_RDONLY)) < 0) {
            err_quit("cannot open inputfile");
        }
        if ((fd2 = open(pathname, O_RDONLY)) >= 0) {
            #ifdef DEBUG
            printf("now begin compare the two files\n");
            #endif
            /* 开始读取两个文件的内容并比较 */
            while ((read(fd1, buf1, bufsize) > 0) && (read(fd2, buf2, bufsize) > 0)) {
                for (i = 0; i < bufsize; i++) {
                    if (buf1[i] != buf2[i]) {
                        /* 文件内容不同，直接返回 */
                        free(buf1);
                        free(buf2);
                        close(fd1);
                        close(fd2);
                        return 0;
                    }
                }
            }
            /* 文件内容相同 */
            if (realpath(pathname, resolved_path) == NULL) {
                err_quit("realpath error");
            }
            printf("%s\n", resolved_path);
        }
        if (fd1 >= 0) {
            close(fd1);
        }
        if (fd2 >= 0){
            close(fd2);
        }
    }
    free(buf1);
    free(buf2);
    free(resolved_path);
    return 0;
}

/* 
 * 
 */
static int myfunc3(const char *pathname, const struct stat *statptr, int type)
{
    int i;
    char *resolved_path = path_alloc(NULL);
    /* 因为basename()函数可能改变pathname的值，因此先复制 */
    char *pathname_copy = path_alloc(NULL);
    strcpy(pathname_copy, pathname);
    char *name = basename(pathname_copy);
    if (type == FTW_F) {
        for (i = 0; i < strnum; i++) {
            if (strcmp(name, str[i]) == 0) {
                /* 两个文件名相同 */
                if (realpath(pathname, resolved_path) == NULL) {
                    err_quit("realpath error");
                }
                printf("%s\n", resolved_path);
                break;
            }
        }
    }
    free(pathname_copy);
    free(resolved_path);
    return 0;
}
