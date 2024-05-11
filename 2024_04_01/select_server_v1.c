#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

int main()
{
    // Tạo socket cho kết nối
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Khai báo địa chỉ server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5))
    {
        perror("listen() failed");
        return 1;
    }

    fd_set fdread; // cấu trúc chứa mảng các bit mô tả các socket gắn vào để thăm dò sự kiện.
    // FD_CLR(int fd, fd_set *set) => xóa fd khỏi tập set
    // FD_ISSET(int fd, fd_set *set) => kiểm tra sự kiện fd xảy ra với tập set
    // FD_SET(int fd, fd_set *set) => gắn fd vào tập set
    // FD_ZERO(fd_set *set) => xóa tất cả các socket ra khỏi tập set.

    int clients[FD_SETSIZE];
    int numClients = 0;

    int maxdp; // lưu giá trị socket lớn nhất sử dụng trong hàm select()
    char buf[256];

    while (1)
    {
        // Khởi tạo tập fdread
        FD_ZERO(&fdread);

        // Gắn các socket vào tập fdread
        FD_SET(listener, &fdread);
        maxdp = listener;

        for (int i = 0; i < numClients; ++i)
        {
            FD_SET(clients[i], &fdread);
            if (clients[i] > maxdp)
                maxdp = clients[i];
        }

        // Gọi hàm select cho sự kiện
        int ret = select(maxdp + 1, &fdread, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("socket() failed");
            break;
        }

        // Kiểm tra sự kiện của socket nào
        if (FD_ISSET(listener, &fdread))
        {
            // Có kết nối mới
            int client = accept(listener, NULL, NULL);
            if (client >= FD_SETSIZE)
            {
                close(client);
            }
            else
            {
                // Lưu socket vào mảng
                clients[numClients] = client;
                numClients++;
                printf("New client connected: %d\n", client);
            }
        }

        // Xử lý sự kiện nhận dữ liệu từ client
        for (int i = 0; i < numClients; ++i)
        {
            if (FD_ISSET(clients[i], &fdread))
            {
                ret = recv(clients[i], buf, sizeof(buf), 0);
                if (ret <= 0)
                {
                    // Kết nối bị ngắt
                    printf("Socket closed %d\n", clients[i]);
                    close(clients[i]);

                    // Xóa socket tại vị trí i
                    if (i < numClients - 1)
                    {
                        clients[i] = clients[numClients - 1];
                    }
                    --numClients;
                    --i;

                    continue;
                }
                else
                {
                    // Xử lý khi nhận dữ liệu
                    buf[ret] = 0;
                    printf("Received from %d: %s\n", clients[i], buf);
                }
            }
        }
    }

    return 0;
}