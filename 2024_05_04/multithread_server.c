#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *client_proc(void *);

int clients[64];
int main()
{

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    while (1)
    {
        printf("Waiting for new client.\n");
        int client = accept(listener, NULL, NULL);

        printf("New client accepted, client = %d\n", client);

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client); // cho phép phục vụ nhiều client cùng một lúc
        pthread_detach(tid); // đánh dấu luồng ở trạng thái tách rời. Khi luồng kết thúc thì tài nguyên tự giải phóng.
    }
    return 0;
}

void *client_proc(void *arg)
{
    int client = *(int *)arg;
    char buf[256];

    // Nhan du lieu tu client
    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);
    }

    close(client);
}