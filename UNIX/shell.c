/*
 * UNIX程序设计实验一
 * 实现带参数的简单shell
 * 编译执行 "gcc shell.c error2e.c -o shell"
 * 
 * 刘硕
 * xmuliushuo@gmail.com
 * xmuliushuo.com
 * 2012.02.23
 */
 
#include "apue.h"
#include <sys/wait.h>

// 仅用于调试
//#define DEBUG

int
main(void)
{
    char buf[MAXLINE];
    pid_t pid;
    int status;
    
    extern char **environ;

    // 以下变量用于分割输入的命令，获得参数表
    char bufCopy[MAXLINE];
    char *argv[MAXLINE];
    char delims[] = " ";
    char *result = NULL;
    int count = 0;

    // 用于获得命令的地址
    char *pathEnvp = NULL;
    char path[MAXLINE];

    printf("%% ");
    while (fgets(buf, MAXLINE, stdin) != NULL) {
        if (buf[strlen(buf) - 1] == '\n')
            buf[strlen(buf) - 1] = 0;

        if ((pid = fork()) < 0) {
            err_sys("fork error");
        }
        else if (pid == 0) {
            // 这里分割输入的命令
            strcpy(bufCopy, buf);
            result = strtok(bufCopy, delims);
            while (result != NULL) {
                argv[count++] = result;
                result = strtok(NULL, delims);
            } 
            argv[count] = NULL;  
            
            // 这里获得输入命令的地址
            strcpy(path, argv[0]);
            
            if (access(path, F_OK) < 0) {   
                pathEnvp = getenv("PATH"); 
                char pathEvnpCopy[MAXLINE];
                
                // 这里发现了一个有意思的现象，如果不复制pathEvnp，
                // 直接对pathEvnp调用strtok函数，编译运行后，
                // 会发现简单shell的系统环境变量PATH的值被修改了（因为strtok会修改pathEvnp的内容），
                // 从而导致一些依赖于PATH值的命令执行出错，比如"gcc xxx.c"
                // shell会提示"gcc: error trying to exec 'ccl': execvp: 没有那个文件或目录"
                strcpy(pathEvnpCopy, pathEnvp);
                  
                delims[0] = ':';
                result = strtok(pathEvnpCopy, delims);
                while (result != NULL) {
                    strcpy(path, result);
                    path[strlen(path) + 1] = 0;
                    path[strlen(path)] = '/';
                    strcat(path, argv[0]);
                    result = strtok(NULL, delims);
                    
                    if (access(path, F_OK) >= 0)
                        break;
                }
            }
            
            #ifdef DEBUG
            printf("path: %s \n", path);
            char **varTest;
            for (varTest = argv; *varTest != NULL; varTest++) {
                printf("args: %s\n", *varTest);
            }
            #endif
            
            // 这里调用execve
            execve(path, argv, environ);
            err_ret("couldn't execute: %s", buf);
            exit(127);
        }
        
        if ((pid = waitpid(pid, &status, 0)) < 0)
            err_sys("waitpid error");
        printf("%% ");
    }
    
    return 0;
}
