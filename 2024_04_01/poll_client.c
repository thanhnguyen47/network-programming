#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

int main()
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));

    struct pollfd fds[2];
    fds[0].fd = STDIN_FILENO; // mô tả của thiết bị nhập dữ liệu
    fds[0].events = POLLIN;
    fds[1].fd = client; // mô tả của socket client
    fds[1].events = POLLIN;

    char buf[256];
    while (1)
    {
        int ret = poll(fds, 2, -1); 
        // fds: tập các mô tả cần đợi sự kiện
        // nfds = 2: số lượng các mô tả, không vượt quá RLIMIT_NOFILE
        // timeout = -1: thời gian chờ theo ms, =-1 tức là chỉ trả về kết quả khi có sự kiện.
        if (fds[0].revents & POLLIN)
        {
            // nhận dữ liệu từ bàn phím
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }
        if (fds[1].revents && POLLIN)
        {
            // Nhận dữ liệu từ server
            ret = recv(client, buf, sizeof(buf), 0);
            if (ret <= 0)
                break;
            buf[ret] = 0;
            puts(buf);
        }
    }

    close(client);
    return 0;
}