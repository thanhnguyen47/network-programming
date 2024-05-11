#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> // chứa các hàm chuyển đổi cấu trúc địa chỉ
#include <unistd.h>
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
    addr.sin_addr.s_addr = htonl(INADDR_ANY); // lưu địa chỉ theo kiểu big-endian, 32bits nên cần phải được chuyển đổi: host to network long
    addr.sin_port = htons(9000); // lưu giá trị cổng theo kiểu big-endian, 16 bits nên cần phải được chuyển đổi: host to network short.

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

    printf("Dang cho ket noi...\n");
    
    struct sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    int client = accept(listener, (struct sockaddr *)&clientAddr, &clientAddrLen); // lấy thông tin địa chỉ chi tiết của client, chỉ trả về socket mới cho việc giao tiếp với client
    // maybe: int client = accept(listener, NULL, NULL) sẽ không lấy thông tin chi tiết địa chỉ của client 
    printf("Client moi ket noi: %d - IP: %s - Port: %d\n", 
        client, inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

    // char msg[] = "Hello client\n";
    // int ret = send(client, msg, strlen(msg), 0);
    // printf("%d bytes da duoc gui\n", ret);

    // char buf[256];
    // for (int i = 0; i < 10; i++)
    //     buf[i] = i + 97;
    // ret = send(client, buf, 10, 0);
    // printf("%d bytes da duoc gui\n", ret);

    // float f = 3.14;
    // ret = send(client, &f, sizeof(f), 0);
    // printf("%d bytes da duoc gui\n", ret);


    char buf[16];
    memset(buf, 97, sizeof(buf));
    // server dùng để nhận dữ liệu từ client
    while (1) {
        printf("Dang cho du lieu...\n");
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0) {
            printf("Ket noi bi huy");
            return 1;
        }

        if (ret < sizeof(buf))
            buf[ret] = 0;

        printf("So bytes nhan duoc: %d\n", ret);
        printf("Du lieu nhan duoc: %s\n", buf);
    }
    
    close(listener);
    return 0;
}