#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

int main()
{
    // Tạo socket cho kết nối
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
    FD_ZERO(&fdread);

    // Gắn socket listener vào fdread
    FD_SET(listener, &fdread);

    int client_sockets[FD_SETSIZE];
    int num_clients = 0;

    char buf[256];
    while (1)
    {
        fdtest = fdread;
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret == -1)
            break;

        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &fdtest))
            {
                if (i == listener)
                {
                    // Có kết nối
                    int client = accept(listener, NULL, NULL);
                    if (client >= FD_SETSIZE)
                    {
                        close(client);
                    }
                    else
                    {
                        FD_SET(client, &fdread);
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
                        continue;
                    }

                    buf[ret] = 0;
                    printf("Received from %d : %s\n", client, buf);

                    int j = 0;
                    for (; j < num_clients; ++j)
                    {
                        if (client_sockets[j] == client)
                            break;
                    }

                    if (j == num_clients)
                    {
                        // chua co dang nhap
                        char user[32], passwd[32], tmp[65], line[65];
                        int n = sscanf(buf, "%s %s %s", user, passwd, tmp);
                        if (n != 2)
                        {
                            char *msg = "Sai cu phap. Hay nhap lai.\n";
                            send(client, msg, strlen(msg), 0);
                        }
                        else
                        {
                            // Kiểm tra thông tin đăng nhập
                            sprintf(tmp, "%s %s\n", user, passwd);
                            FILE *f = fopen("accounts.txt", "r");
                            int found = 0;

                            while (fgets(line, sizeof(line), f) != NULL)
                            {
                                if (strcmp(tmp, line) == 0)
                                {
                                    found = 1;
                                    break;
                                }
                            }

                            if (found)
                            {
                                char *msg = "Dang nhap thanh cong.\n";
                                send(client, msg, strlen(msg), 0);

                                // Luu thong tin dang nhap
                                client_sockets[num_clients] = client;
                                ++num_clients;
                            }
                            else
                            {
                                char *msg = "Dang nhap that bai. Hay nhap lai.\n";
                                send(client, msg, strlen(msg), 0);
                            }
                            fclose(f);
                        }
                    }
                    else
                    {
                        // Đã đăng nhập
                        char cmd[300];

                        // Xóa ký tự xuống dòng ở cuối buf
                        if (buf[strlen(buf) - 1] == '\n')
                        {
                            buf[strlen(buf) - 1] = 0;
                        }

                        // Trả lại kết quả cho client
                        FILE *f = fopen("out.txt", "rb");
                        while(1) {
                            int n = fread(buf, 1, sizeof(buf), f);
                            if (n <= 0) break;

                            send(client, buf, n, 0);
                        }
                        fclose(f);
                    }
                }
            }
        }
    }

    return 0;
}