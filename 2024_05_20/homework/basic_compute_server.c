#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void *client_proc(void *);

int main()
{
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 10))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[2048];
    // Nhan du lieu tu client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0)
    {
        close(client);
        pthread_exit(NULL);
    }
    buf[ret] = 0;
    printf("Received from %d: %s", client, buf);
    char *start_pos = buf;
    if (strncmp(buf, "POST ", 5) == 0)
    {
        start_pos = (char *)memmem(buf, ret, "\r\n\r\n", 4) + 4;
    }

    char *pos1_a = strstr(start_pos, "a=") + 2;
    char *pos2_a = strchr(pos1_a, '&');
    char tmp_a[16] = {0};
    memcpy(tmp_a, pos1_a, pos2_a - pos1_a);
    int a = atoi(tmp_a);

    char tmp_b[16] = {0};
    char *pos1_b = strstr(pos2_a, "&b=") + 3;
    char *pos2_b = strchr(pos1_b, '&');
    memcpy(tmp_b, pos1_b, pos2_b - pos1_b);
    int b = atoi(tmp_b);

    char cmd[10] = {0};
    char *pos1_cmd = strstr(pos2_b, "&cmd=") + 5;
    char *pos2_cmd;
    if (strncmp(buf, "POST ", 5) != 0)
    {
        pos2_cmd = strchr(pos1_cmd, ' ');
    }
    else
    {
        pos2_cmd = strchr(pos1_cmd, '\0');
    }
    memcpy(cmd, pos1_cmd, pos2_cmd - pos1_cmd);
    // memcpy(cmd, pos1_cmd, 3);

    // printf("a = %s\n", tmp_a);
    // printf("b = %s\n", tmp_b);
    // printf("cmd = %s\n", cmd);

    int res;
    if (strncmp(cmd, "add", 3) == 0)
    {
        res = a + b;
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%d</h1>", res);
    }
    else if (strncmp(cmd, "sub", 3) == 0)
    {
        res = a - b;
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%d</h1>", res);
    }
    else if (strncmp(cmd, "mul", 3) == 0)
    {
        res = a * b;
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%d</h1>", res);
    }
    else if (strncmp(cmd, "div", 3) == 0)
    {
        res = a / b;
        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%d</h1>", res);
    }
    else
    {
        strcpy(buf, "400 Bad Request");
    }
    send(client, buf, strlen(buf), 0);

    close(client);
    pthread_exit(NULL);
}