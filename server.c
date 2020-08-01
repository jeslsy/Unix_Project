#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#define oops(msg)  { perror(msg); exit(1); }
#define PORTNUM 19174
extern int make_server_socket(int);
void process_request(int);
void child_waiter(int signum);
void sanitize(char *);
void write_file_to_client(int fd, int filed);
void show_file_time(struct stat *, FILE *);
void read_from_client(int, char*);
void answer_a_quiz(char *buf,char *stage, int i, int fd);

int score = 0, quiz_count = 0;
char ans[BUFSIZ];
int main(int ac, char *av[])
{
    int sock, fd;
    srand((unsigned)time(NULL));
    signal(SIGCHLD, child_waiter);
    sock = make_server_socket(PORTNUM);
    if (sock == -1)
        exit(1);

    while(1) {
        fd = accept(sock, NULL, NULL);
        if (fd == -1)
            break;

        process_request(fd);
        close(fd);
    }
}

void child_waiter(int signum)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
void process_request(int fd)
{
    char id[BUFSIZ], command[BUFSIZ], ans_command[BUFSIZ];
    FILE *sock_fpi, *fp, *file;
    char num[BUFSIZ], sub[BUFSIZ], buf[BUFSIZ], stage[BUFSIZ];
    char score_buf[BUFSIZ];
    char cmp[] = "1\n";
    char quizstart[BUFSIZ] = "You Quiz?\n1 : YES \n2 : NO\n번호를 입력하세요: ";
    int file_d, quiz_f, stage_f, i=1;
    struct stat infobuf;

    int pid = fork();
    switch(pid) {
        case -1:
            return;
        case 0:
            sock_fpi = fdopen(fd, "r");
            //id 파일 생성
            fgets(id, BUFSIZ, sock_fpi);
            sanitize(id);
            sprintf(id, "%s.txt", id);
            if((file = fopen(id, "w")) == NULL)
                oops("id file");
            if(stat(id, &infobuf) == -1){
                oops("stat");
            }

            printf("Client connect\n");
            write(fd, quizstart, strlen(quizstart));
            //read_from_client(fd, num);
            read(fd, num, BUFSIZ);

            if(!strcmp(num, cmp))
            {
                printf("Client said YES\n");
                file_d=open("quiz.txt", O_RDONLY);
                write_file_to_client(fd, file_d);
                read_from_client(fd, sub);
                fp = popen("ls quiz", "r");
                while(fgets(buf, 100, fp) != NULL){
                    if(sub[0] ==  buf[0]){
                        stage_f = open("quiz/stage.txt",O_RDONLY);
                        write_file_to_client(fd, stage_f);
                        read_from_client(fd, stage);
                        buf[strlen(buf)-1] = '\0';

                        i = rand()%3+1;

                        answer_a_quiz(buf, stage, i, fd);
                        sprintf(score_buf, "score = %d\n답변자의 응답 = %s", score, ans);
                        fputs(score_buf, file);

                        show_file_time(&infobuf, file);

                    }
                }
            }
            else
            {
                sprintf(score_buf, "퀴즈쇼에 참가하지 않음\n");
                fputs(score_buf, file);
                show_file_time(&infobuf, file);

            }
            close(file_d);
            close(stage_f);
            close(quiz_f);
            fclose(file);
            pclose(fp);
            fclose(sock_fpi);
            exit(0);
    }
}
void sanitize(char *str)
{
    char *src, *dest;
    for(src = dest = str; *src ; src++) {
        if(*src == '.' || *src == '_' || isalnum(*src))
            *dest++ = *src;
    }
    *dest = '\0';
}
void write_file_to_client(int fd, int file)
{
    int n;
    char buf[BUFSIZ];
    n = read(file, buf, BUFSIZ);
    write(fd, buf, n);
}
void read_from_client(int fd, char * input)
{
    read(fd, input, BUFSIZ);
    sanitize(input);
    printf("%s\n", input);
}
void show_file_time(struct stat * buf, FILE *file)
{
    char time[BUFSIZ];
    sprintf(time, "최근 접속 시간 = %.12s\n", 4+ctime(&buf->st_mtime));
    fputs(time, file);
}

void answer_a_quiz(char *buf,char *stage, int i, int fd)
{
    char command[BUFSIZ];
    char ans_command[BUFSIZ];
    char score_buf[BUFSIZ];
    char real_ans[BUFSIZ];
    int quiz_f,ans_p,ans_f;
    char *ptr;
    char right[BUFSIZ] = "맞았습나다!!\n";
    char wrong[BUFSIZ] = "틀렸습니다 ㅜㅜ\n";
    quiz_count++;

    sprintf(command, "quiz/%s/%s/quiz%d.txt", buf, stage, i);
    quiz_f = open(command, O_RDONLY);
    write_file_to_client(fd, quiz_f);

    read_from_client(fd,ans);
    sprintf(ans_command,"quiz/%s/%s/ans%d.txt", buf, stage, i);
    ans_p = open(ans_command, O_RDONLY);
    write_file_to_client(fd, ans_p);

    ans_f = open(ans_command, O_RDONLY);
    read(ans_f, real_ans, BUFSIZ);
    printf("%s", real_ans);
    ptr = strtok(real_ans, ":");
    ptr = strtok(NULL," ");
    ans[strlen(ans)] = '\n';
    printf("ptr:%s",ptr);
    printf("ans:%s", ans);

    if(!strcmp(ptr, ans))
    {
        write(fd, right, strlen(right));
        printf("%s\n", right);
        if(!strcmp(stage, "stage1"))
            score+=20;
        else if(!strcmp(stage, "stage2"))
            score+=30;
        else if(!strcmp(stage, "stage3"))
            score+=50;

    }
    else
    {
        write(fd, wrong, strlen(wrong));

        printf("%s\n", wrong);
    }

    printf("%d\n", score);
    sleep(0.5);
    sprintf(score_buf, "score = %d\n", score);
    write(fd, score_buf, strlen(score_buf));

    close(quiz_f);
    close(ans_p);
    close(ans_f);
}

