#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int signo)
{
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

int main()
{
    // Tạo socket chờ kết nối
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

    signal(SIGCHLD, signalHandler); // đăng ký xử lý sự kiện SIGCHLD tránh tiến trình con ở trạng thái zombie

    while (1)
    {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        if (fork() == 0)
        {
            // tien trinh con xu ly yeu cau tu client
            // dong socket listener được sao chép từ tiến trình cha (1 listener là đủ)
            close(listener);

            char buf[256];
            while (1)
            {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;

                buf[ret] = 0;
                printf("Received: %s\n", buf);
            }

            // Kết thúc tiến trình con
            exit(0);
        }

        // Đóng socket client ở tiến trình cha (chỉ cần ở tiến trình con là đủ)
        close(client);
    }

    return 0;
}