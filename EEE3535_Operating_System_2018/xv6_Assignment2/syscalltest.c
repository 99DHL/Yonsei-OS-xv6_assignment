/* syscalltest.c */

/*************************************************
 * EEE3535-02 Fall 2018                          *
 * School of Electrical & Electronic Engineering *
 * Yonsei University, Seoul, South Korea         *
 *************************************************/

#include "syscall.h"
#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int fd[2];
    char buf[128];
    if(pipe(fd) != 0) { printf(1, "pipe() failed\n"); exit(); }
    int pid = fork();
    if(pid < 0) { printf(1, "fork() failed\n"); exit(); }
    else if(pid > 0) {
        strcpy(buf, "EEE3535-02 Operating Systems - Assignment #2: Syscall");
        close(fd[0]);
        write(fd[1], buf, sizeof(buf));
        wait();
        printf(1, "parent process (pid = %d)\n",   getpid()); 
        

		printf(1, "    count(SYS_fork) = %d\n",   count(SYS_fork));
        printf(1, "    count(SYS_exit) = %d\n",   count(SYS_exit));
        printf(1, "    count(SYS_wait) = %d\n",   count(SYS_wait));
        printf(1, "    count(SYS_pipe) = %d\n",   count(SYS_pipe));
        printf(1, "    count(SYS_read) = %d\n",   count(SYS_read));
        printf(1, "    count(SYS_kill) = %d\n",   count(SYS_kill));
        printf(1, "    count(SYS_exec) = %d\n",   count(SYS_exec));
        printf(1, "    count(SYS_fstat) = %d\n",  count(SYS_fstat));
        printf(1, "    count(SYS_chdir) = %d\n",  count(SYS_chdir));
        printf(1, "    count(SYS_dup) = %d\n",    count(SYS_dup));
        printf(1, "    count(SYS_getpid) = %d\n", count(SYS_getpid));
        printf(1, "    count(SYS_sbrk) = %d\n",   count(SYS_sbrk));
        printf(1, "    count(SYS_sleep) = %d\n",  count(SYS_sleep));
        printf(1, "    count(SYS_uptime) = %d\n", count(SYS_uptime));
        printf(1, "    count(SYS_open) = %d\n",   count(SYS_open));
        printf(1, "    count(SYS_write) = %d\n",  count(SYS_write));
        printf(1, "    count(SYS_mknod) = %d\n",  count(SYS_mknod));
        printf(1, "    count(SYS_unlink) = %d\n", count(SYS_unlink));
        printf(1, "    count(SYS_link) = %d\n",   count(SYS_link));
        printf(1, "    count(SYS_mkdir) = %d\n",  count(SYS_mkdir));
        printf(1, "    count(SYS_close) = %d\n",  count(SYS_close));
        printf(1, "    count(SYS_count) = %d\n",  count(SYS_count));
	
    }
    else {
        close(fd[1]);
        read(fd[0], buf, sizeof(buf));
        printf(1, "%s\n", buf);
        printf(1, "child process (pid = %d)\n", getpid());
        
		printf(1, "    count(SYS_fork) = %d\n",   count(SYS_fork));
        printf(1, "    count(SYS_exit) = %d\n",   count(SYS_exit));
        printf(1, "    count(SYS_wait) = %d\n",   count(SYS_wait));
        printf(1, "    count(SYS_pipe) = %d\n",   count(SYS_pipe));
        printf(1, "    count(SYS_read) = %d\n",   count(SYS_read));
        printf(1, "    count(SYS_kill) = %d\n",   count(SYS_kill));
        printf(1, "    count(SYS_exec) = %d\n",   count(SYS_exec));
        printf(1, "    count(SYS_fstat) = %d\n",  count(SYS_fstat));
        printf(1, "    count(SYS_chdir) = %d\n",  count(SYS_chdir));
        printf(1, "    count(SYS_dup) = %d\n",    count(SYS_dup));
        printf(1, "    count(SYS_getpid) = %d\n", count(SYS_getpid));
        printf(1, "    count(SYS_sbrk) = %d\n",   count(SYS_sbrk));
        printf(1, "    count(SYS_sleep) = %d\n",  count(SYS_sleep));
        printf(1, "    count(SYS_uptime) = %d\n", count(SYS_uptime));
        printf(1, "    count(SYS_open) = %d\n",   count(SYS_open));
        printf(1, "    count(SYS_write) = %d\n",  count(SYS_write));
        printf(1, "    count(SYS_mknod) = %d\n",  count(SYS_mknod));
        printf(1, "    count(SYS_unlink) = %d\n", count(SYS_unlink));
        printf(1, "    count(SYS_link) = %d\n",   count(SYS_link));
        printf(1, "    count(SYS_mkdir) = %d\n",  count(SYS_mkdir));
        printf(1, "    count(SYS_close) = %d\n",  count(SYS_close));
        printf(1, "    count(SYS_count) = %d\n",  count(SYS_count));
    }

    exit();
}
