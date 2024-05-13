/*
Sử dụng kỹ thuật đa luồng, lập trình ứng dụng chat nhóm 2 người như sau:
Mỗi client khi kết nối đến server sẽ được lưu vào hàng đợi, nếu số lượng client trong hàng đợi là 2 thì 2 
    client này sẽ được ghép cặp với nhau.
Khi 2 client đã được ghép cặp thì tin nhắn gửi từ client này sẽ được chuyển tiếp sang client kia và ngược lại.
Nếu 1 trong 2 client ngắt kết nối thì client còn lại cũng được ngắt kết nối.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_N 1024  //so luong client toi da

int queue[3];
int client_sockets[MAX_N];  //


void *client_proc(void *);

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    int num = 0;
    //luong chinh cho ket noi tu server
    while (1) {
        printf("Waiting for new client connect......\n");
        int client = accept(listener, NULL, NULL);
        printf("New client connected: %d\n", client);
        queue[num] = client;

        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &queue[num]);
        pthread_detach(tid);
        
        num++;
        if (num == 2) {
            //du mot cap client de chat
            client_sockets[queue[0]] = queue[1];
            client_sockets[queue[1]] = queue[0];
            num = 0;
        }
    }

    return 0;
}

void *client_proc(void *arg) {
    int client = *(int *)arg;
    char buf[256];
    //nhan du lieu tu client gui cho client cap tuong ung
    if (client_sockets[client] == 0) {
        char *msg = "Waiting for another....\n";
        send(client, msg, strlen(msg), 0);
    }


    while (1) {
        if (client_sockets[client] != 0) {
            char *msg = "Connecting success!\n";
            send(client, msg, strlen(msg), 0);
            break;
        }
    }
    int recv_client = client_sockets[client];
    //gui nhan du lieu tu client va chuyen cho client kia 
    while (1) {
        
            int ret = recv(client, buf, sizeof(buf), 0);
            
            if (ret <= 0) {
                //dong ket noi 
                close(client_sockets[client]);
                break;
            }

            buf[ret] = 0;
            char msg[1024];
            sprintf(msg, "Client %d: %s", client, buf);
            send(recv_client, msg, strlen(msg), 0);
        
    }
    close(client);
}