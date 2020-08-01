#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/socket.h>
#define PORTNUM 19174
#define LEN 1000
#define oops(msg)   { perror(msg); exit(1); }

extern int connect_to_server(char *, int);
void talk_with_server(int, char *);
void read_from_server(int);
void write_to_server(int, char *);
void show_problem(int);
void sanitize(char *);
int quiz_count = 0;

int main(int ac, char *av[])
{
    int fd;
    fd = connect_to_server(av[1], PORTNUM);
    if (fd == -1)
        exit(1);

    talk_with_server(fd, av[2]);
    close(fd);
}
void talk_with_server(int fd, char *id)
{
    int n_read, i = 1;
    char num[LEN];
    char sub[LEN];
    char stage[LEN];
    char ans[LEN];
    char buf[LEN];
    
    write_to_server(fd, id);
   
    read_from_server(fd);
    
    read(0, num, LEN);
    fflush(stdin);
    num[1] = '\0';
    write_to_server(fd, num);
    
    read_from_server(fd);
    
    read(0, sub, LEN);
    fflush(stdin);
    write_to_server(fd, sub);

    read_from_server(fd);

    read(0, stage, LEN);
    fflush(stdin);
    write_to_server(fd, stage);

    show_problem(fd);
}
void show_problem(int fd){
    char ans[LEN];

    quiz_count++;

    read_from_server(fd);
    memset(ans, 0, LEN);
    read(0, ans, LEN);
    sanitize(ans);
    ans[strlen(ans)] = '\0';
    write_to_server(fd, ans);
    memset(ans, 0, LEN);

    read_from_server(fd);

    sleep(0.2);
    read_from_server(fd);
    fflush(stdout);
    fflush(stdin);
}
void write_to_server(int fd, char *input){
    if(write(fd, input, strlen(input))== -1)
        oops("write");
    if(write(fd, "\n", 1)== -1)
        oops("write");
}
void read_from_server(int fd){
    char buf[LEN];
    int n;
    if((n = read(fd, buf, LEN))<0)
        oops("read");
    if(write(1, buf, n) == -1)
        oops("write");
    fflush(stdout);
}
void sanitize(char *str)
{
    char *src, *dest;
    for(src = dest = str; *src; src++){
        if(isalnum(*src))
            *dest++ = *src;
    }
    *dest = '\0';
}
