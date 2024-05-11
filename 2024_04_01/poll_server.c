#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>

#define MAX_FDS 2048

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

    struct pollfd fds[MAX_FDS];
    int nfds = 0;

    // Gắn socket listener vào tập thăm dò
    fds[0].fd = listener;
    fds[0].events = POLLIN;
    ++nfds;

    char buf[256];
    while (1)
    {
        int ret = poll(fds, nfds, -1);
        if (ret == -1)
        {
            break;
        }

        if (ret == 0)
        {
            printf("Timed out.\n");
            continue;
        }

        for (int i = 0; i < nfds; ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i].fd == listener)
                {
                    // có kết nối mới
                    int client = accept(listener, NULL, NULL);
                    if (client >= MAX_FDS)
                    {
                        // vượt quá số lượng kết nối tối đa, từ chối
                        close(client);
                    }
                    else
                    {
                        // Thêm vào mảng thăm dò sự kiện
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;

                        printf("New client connected: %d\n", client);
                    }
                }
                else
                {
                    // Có dữ liệu từ client truyền đến
                    int client = fds[i].fd;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        // Kết nối bị đóng ==> xóa khỏi mảng
                        // fds[i] = fds[nfds - 1];
                        // --nfds;
                        // --i;
                    }
                    else
                    {
                        // Xử lý dữ liệu nhận được
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", client, buf);

                        // Chuyển tiếp dữ liệu cho các client khác
                        for (int j = 0; j < nfds; j++)
                        {
                            if (fds[j].fd != listener && fds[j].fd != client)
                            {
                                send(fds[j].fd, buf, strlen(buf), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}