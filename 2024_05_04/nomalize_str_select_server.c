#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <ctype.h>

int remove_client(int client, int *client_socket, char **name_clients, int *num_clients)
{
    // Xoa client khoi danh sach client da dang nhap
    int i = 0;
    for (; i < *num_clients; i++)
    {
        if (client_socket[i] == client)
        {
            break;
        }
    }
    if (i < *num_clients)
    {
        if (i < *num_clients - 1)
        {
            client_socket[i] = client_socket[*num_clients - 1];
            strcpy(name_clients[i], name_clients[*num_clients - 1]);
        }
        free(name_clients[*num_clients - 1]);
        (*num_clients)--;
    }
}

void normalize_str(char *str)
{
    char normalized_str[256];
    int i = 0, j = 0;
    while (str[i] != '\0')
    {
        // Tim ky tu khac dau cach dau tien tu vi tri hien tai
        while (str[i] == ' ' && str[i] != '\0')
            i++;
        if(str[i] == '\0')
            break;

        // Copy ky tu khac dau cach vao normalized_str
        int first_char = 1;
        while (str[i] != ' ' && str[i] != '\0')
        {
            if (first_char)
            {
                normalized_str[j++] = toupper(str[i++]);
                first_char = 0;
            }
            else
            {
                normalized_str[j++] = tolower(str[i++]);
            }
        } 

        if(str[i] == '\0')
            break;

        // Them dau cach vao normalized_str
        if(str[i] == ' ')
            normalized_str[j++] = str[i++];
    }

    // Kiem tra ky tu cuoi cung co phai la dau cach khong
    if(normalized_str[j-1] == ' ')
        normalized_str[j-1] = '\0';
    else
        normalized_str[j] = '\0';
    
    strcpy(str, normalized_str);
}

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

    // Khai báo tập fdread chứa các socket và tập fdtest để thăm dò sự kiện
    fd_set fdread, fdtest;
    FD_ZERO(&fdread);

    // Gan cac socket vao tap fread
    FD_SET(listener, &fdread);

    char buf[256];
    struct timeval tv;

    int client_sockets[FD_SETSIZE]; // Danh sách các client
    char *name_clients[FD_SETSIZE]; // Danh sách tên của các client
    int num_clients = 0; // Số lượng client hiện tại

    while (1)
    {
        // Giữ nguyên các socket trong tập fdread
        fdtest = fdread;

        // Khởi tạo cấu trúc thời gian
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Gọi hàm select() chờ đến khi sự kiện xảy ra hoặc hết giờ
        int ret = select(FD_SETSIZE, &fdread, NULL, NULL, 0);
        if (ret == -1)
        {
            printf("select() failed.\n");
            break;
        }
        if (ret == 0)
        {
            printf("Timed out.\n");
            continue;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            // Kiem tra su kien cua socket nao
            if (FD_ISSET(i, &fdtest))
            {
                // Socket listener có sự kiện yêu cầu kết nối
                if (i == listener)
                {
                    // Co ket noi moi
                    int client = accept(listener, NULL, NULL);
                    if (client >= FD_SETSIZE) // Đã vượt quá số kết nối tối đa
                    {
                        close(client);
                    }
                    else // Chưa vượt quá số kết nối tối đa
                    {
                        // Thêm socket vào tập sự kiện 
                        FD_SET(client, &fdread);
                        printf("New client connected: %d\n", client);
                        client_sockets[num_clients] = client; // fix
                        num_clients++; 

                        char msg[256];
                        sprintf(msg, "Dang co %d client ket noi den server\n", num_clients);
                        send(client, msg, strlen(msg), 0);
                    }
                }
                else 
                {
                    // Socket client có sự kiện nhận dữ liệu 
                    int client = i;
                    int ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0)
                    {
                        close(client);
                        FD_CLR(client, &fdread); // Xoá socket ra khỏi tập sự kiện 
                        printf("Client %d disconnected\n", client);

                        //Xoa client khoi danh sach
                        remove_client(client, client_sockets, name_clients, &num_clients);
                        continue;
                    }
                    else
                    {

                        buf[ret] = 0;
                        printf("Received data from %d: %s\n", client, buf);

                        if(strncmp( buf, "exit", 4) == 0){
                            char msg[256];
                            sprintf(msg, "Goodbye %s\n", name_clients[client]);
                            send(client, msg, strlen(msg), 0);
                            close(client);
                            FD_CLR(client, &fdread); // Xoá socket ra khỏi tập sự kiện 
                            printf("Client %d disconnected\n", client);

                            //Xoa client khoi danh sach
                            remove_client(client, client_sockets, name_clients, &num_clients);
                            continue;
                        }

                        // Chuẩn hóa chuỗi
                        normalize_str(buf);
                        send(client, buf, strlen(buf), 0);
                    }
                }
            }
        }
    }

    close(listener);

    return 1;
}