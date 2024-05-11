#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main()
{
    // tao socket cho ket noi
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

    // sử dụng mảng tự tạo để quản lý các client
    int clients[64];
    int numClients = 0;

    char buf[256];
    while (1)
    {
        printf("Dang cho ket noi ...\n");
        int client = accept(listener, NULL, NULL); // mỗi client gửi đúng một tin nhắn rồi bắt kết nối với client khác :))

        clients[numClients] = client;
        numClients++;

        printf("Dang nhan du lieu tu cac client:\n");

        // chế độ đồng bộ làm việc theo thứ tự, chỉ có duy nhất 1 kết nối được phục vụ tại 1 thời điểm. 
        for (int i = 0; i < numClients; ++i)
        {
            int ret = recv(clients[i], buf, sizeof(buf), 0);
            if (ret <= 0)
                continue;
            buf[ret] = 0;
            printf("Received from %d: %s\n", clients[i], buf);
        }
    }

    close(listener);
    return 0;
}