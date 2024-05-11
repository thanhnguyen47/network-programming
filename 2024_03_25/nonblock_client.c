#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>

int main() {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr*)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    // chuyen socket sang che do bat dong bo
    unsigned long ul = 1;
    ioctl(client, FIONBIO, &ul);

    // chuyển stdin sang chế độ bất đồng bộ
    ioctl(STDIN_FILENO, FIONBIO, &ul);

    char buf[256];
    while(1) {
        // Đọc dữ liệu từ bàn phím, gửi sang server
        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            send(client, buf, strlen(buf), 0);
            if (strncmp(buf, "exit", 4) == 0) {
                break;
            }
        }

        // Nhận dữ liệu từ server, in ra màn hình
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret != -1) {
            if (ret == 0) break;
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
        else {
            if (errno == EWOULDBLOCK) {
                // loi do chua co du lieu
                // khong lam gi
            } else {
                // loi khac
                break;
            }
        }
    }

    close(client);

    return 0;
}