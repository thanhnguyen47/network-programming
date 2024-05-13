#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *thread_proc(void *);

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }
    // Ý tưởng: tạo luồng con để thực hiện gửi tin nhắn, luồng cha để thực hiện nhận tin nhắn.
    pthread_t tid;
    pthread_create(&tid, NULL, thread_proc, &client);
    // &tid : lưu id luồng trả về
    // NULL : tham số tạo luồng, mặc định NULL
    // thread_proc: hàm thực thi
    // &client: tham số truyền vào hàm thực thi
    // --> trả về 0 nếu thành công.

    char buf[256];
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        if (buf[ret - 1] == '\n')
            buf[ret - 1] = 0;
        else
            buf[ret] = 0;
        puts(buf);
    }

    close(client);
    return 0;
}

void *thread_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];
    while (1)
    {
        fgets(buf, sizeof(buf), stdin);
        send(client, buf, strlen(buf), 0);
        if (strncmp(buf, "exit", 4) == 0)
            break;
    }

    close(client);
}