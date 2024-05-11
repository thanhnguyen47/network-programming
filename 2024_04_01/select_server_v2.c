#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

// thực hiện đánh dấu client ngay trong tập fdread, không cần tạo thêm mảng clients để lưu lại
// duyệt chậm hơn do phải duyệt toàn bộ fdread.
int main()
{
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
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

    fd_set fdread, fdtest;
    FD_ZERO(&fdread); // giữa set fdread làm cơ sở, làm việc trên set fdtest còn lại.

    // Gán socket listener vào fdread
    FD_SET(listener, &fdread);
    // Nếu muốn gửi tin nhắn từ server thì gán thêm mô tả STDIN_FILENO vào là có thể kiểm tra sự kiện của STDIN_FILENO

    char buf[256];
    while (1)
    {
        fdtest = fdread;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret == -1)
        {
            break;
        }

        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            // kiểm tra các sự kiện của từng phần tử trên tập fdtest
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    // Có kết nối
                    int client = accept(listener, NULL, NULL);
                    if (client >= FD_SETSIZE)
                    {
                        // đã vượt qua số kế nối tối đa 
                        close(client);
                    }
                    else
                    {
                        FD_SET(client, &fdread); // đánh dấu trực tiếp vào fdread chứ không phải fdtest
                        printf("New client connected: %d\n", client);
                    }
                }
                else
                {
                    // Có dữ liệu truyền đến
                    int client = i;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        close(client);
                        FD_CLR(client, &fdread);
                    }
                    else
                    {
                        buf[ret] = 0;
                        printf("Received from %d: %s\n", client, buf);
                    }
                }
            }
        }
    }

    return 0;
}