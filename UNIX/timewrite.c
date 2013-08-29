/*
 * 同步与异步write的效率比较
 * 刘硕
 * xmuliushuo@gmail.com
 * xmuliushuo.com
 */

#include "apue.h"
#include <sys/times.h>
#include <fcntl.h>

#define DEBUG

void paramError();
ssize_t Write(int filedes, const void *buf, size_t nbyte);
off_t Lseek(int filedes, off_t offset, int whence);

int main(int argc, char *argv[])
{
    char *outfilePath;  // 输出文件
    int outfile;        // 输出文件的描述符
    int fileLength;     // 输入文件的长度
    char *buf;          // 输入文件缓冲区
    int buffsize;       // 写缓冲区大小
    int offset;         // 用于写文件时记录
    int isSync = 0;     // 是否用同步的方式写文件

    // 以下变量用于计时
    struct tms sStart, sEnd;
    clock_t clStart, clEnd;
    int loopCount;
    int tick = sysconf(_SC_CLK_TCK);

    if (argc == 2) {
        outfilePath = argv[1];
    }
    else if (argc == 3) {
        outfilePath = argv[1];
        if (strcmp(argv[2], "sync") == 0) {
            isSync = 1;
        }
        else {
            paramError();
        }
    }
    else {
        paramError();
    }

    // 判断输入文件长度
    fileLength = Lseek(STDIN_FILENO, 0, SEEK_END);
    Lseek(STDIN_FILENO, 0, SEEK_SET);
    #ifdef DEBUG
    printf("the input file length: %d\n", fileLength);
    #endif

    if ((buf = (char *)malloc(sizeof(char) * fileLength)) == NULL) {
        err_sys("malloc error");
    }
    if (read(STDIN_FILENO, buf, fileLength) != fileLength) {
        err_sys("read error");
    }
    if (isSync == 1) {
        if ((outfile = open(outfilePath, O_RDWR | O_CREAT | O_TRUNC | O_SYNC)) < 0) {
            err_sys("open error");
        }
    }
    else if ((outfile = open(outfilePath, O_RDWR | O_CREAT | O_TRUNC)) < 0) {
        err_sys("open error");
    }

    printf("BUFFSIZE\tuser_cpu\tsys_cpu\tclock_time\tloop_count\n");
    buffsize = 1024;
    while (buffsize <= 131072) {
        offset = 0;
        loopCount = 0;
        Lseek(outfile, 0, SEEK_SET);
        clStart = times(&sStart);
        while (1) {
            loopCount++;
            if (fileLength < buffsize + offset) {
                Write(outfile, buf + offset, fileLength - offset);
                break;
            }
            Write(outfile, buf + offset, buffsize);
            offset += buffsize;
        }
        clEnd = times(&sEnd);
        printf("%d\t\t", buffsize);
        printf("%.2f\t\t", (float)(sEnd.tms_utime - sStart.tms_utime) / tick);
        printf("%.2f\t", (float)(sEnd.tms_stime - sStart.tms_stime) / tick);
        printf("%.2f\t\t", (float)(clEnd - clStart) / tick);
        printf("%d\n", loopCount);
        buffsize *= 2;
    }
    printf("Done!\n");
    return 0;
}

/*
 * 输入参数出错时运行
 */
void paramError()
{
    printf("input error!\n");
    printf("usage:\n");
    printf("timewrite <ourfile> [sync]\n");
    exit(1);
}

/*
 * write函数的包裹函数
 */
ssize_t Write(int filedes, const void *buf, size_t nbyte)
{
    ssize_t temp;
    if ((temp = write(filedes, buf, nbyte)) != nbyte) {
        err_sys("write error");
    }
    return temp;
}

/*
 * lseek函数的包裹函数
 */
off_t Lseek(int filedes, off_t offset, int whence) {
    off_t temp;
    if ((temp = lseek(filedes, offset, whence)) == -1) {
        err_sys("lseek error");
    }
    return temp;
}
