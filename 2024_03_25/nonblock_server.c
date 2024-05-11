#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <sys/ioctl.h> // dùng hàm ioctl để chuyển socket sang chế độ bất đồng bộ.

int main()
{
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // chuyển server socket sang chế độ bất đồng bộ
    unsigned long ul = 1;
    ioctl(listener, FIONBIO, &ul);

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

    int clients[64];
    int numClients = 0;

    char buf[256];
    while (1)
    {
        int client;
        if (numClients < 2) // giới hạn numClients lại để chuyển sang bước nhận dữ liệu
        { // printf("Dang cho ket noi ...\n");
            client = accept(listener, NULL, NULL);

            if (client != -1)
            {
                printf("Co ket noi moi: %d\n", client);

                // chuyển client socket sang chế độ bất đồng bộ
                ul = 1;
                ioctl(client, FIONBIO, &ul);

                clients[numClients] = client;
                ++numClients;
            }
            else
            {
                if (errno = EWOULDBLOCK)
                {
                    // lỗi do chưa có kết nối
                    // không xử lý gì thêm
                }
                else
                {
                    break;
                }
            }
        }

        // printf("Doi nhan du lieu tu cac client:\n");
        for (int i = 0; i < numClients; ++i)
        {
            int ret = recv(client, buf, sizeof(buf), 0);
            if (ret != -1)
            {
                if (ret == 0)
                    continue;

                buf[ret] = 0;
                printf("Received from %d: %s\n", clients[i], buf);
            }
            else
            {
                if (errno == EWOULDBLOCK)
                {
                    // Lỗi do chưa có dữ liệu
                    // Không xử lý gì thêm
                }
                else
                {
                    continue;
                }
            }
        }
    }
    close(listener);
    return 0;
}


/*
    Ứng dụng bất đồng bộ không hoạt động đúng ý
*/