#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

int main()
{
    // Download file http://storage.googleapis.com/it4060_samples/ecard.mp4

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // Phan giai ten mien
    struct addrinfo *info;
    if (getaddrinfo("storage.googleapis.com", "http", NULL, &info))
    {
        perror("getaddrinfo() failed.");
        return 1;
    }

    int ret = connect(client, info->ai_addr, info->ai_addrlen);
    if (ret == -1)
    {
        perror("connect() failed");
        return 1;
    }

    char req[] = "GET /it4060_samples/ecard.mp4 HTTP/1.1\r\nHost: storage.googleapis.com\r\nConnection: close\r\n\r\n";
    send(client, req, strlen(req), 0);

    char buf[2048];
    // nhan noi dung phan header
    char *header = NULL;
    int len = 0;

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            close(client);
            printf("disconnected\n");
            return 1;
        }

        // dùng realloc để cấp phát lại bộ nhớ cho header với độ lớn phù hợp.
        header = (char *)realloc(header, len + ret);
        memcpy(header + len, buf, ret);
        len += ret;

        // memmem tìm kiếm sự xuất hiện đầu tiên của một chuỗi con (trong vùng nhớ) trong một chuỗi lớn hơn (cũng trong vùng nhớ).
        if (memmem(header, len, "\r\n\r\n", 4) != NULL)
        {
            break;
        }
        /*
        Lưu ý: Hàm memmem không phải là một phần của chuẩn ANSI C, nhưng nó có sẵn trong GNU C Library (glibc).
        Nếu bạn làm việc trên một hệ thống không có memmem, bạn có thể cần viết lại hoặc sử dụng các hàm tương tự từ thư viện khác.
        */
    }

    // In header ra
    puts(header);

    char *pos = (char *)memmem(header, len, "\r\n\r\n", 4);
    FILE *f = fopen("download.mp4", "wb");

    // Nhận nội dung file
    int header_len = pos - header;
    if (header_len + 4 < len)
        fwrite(pos + 4, 1, len - header_len - 4, f);

    while (1)
    {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }

    free(header);
    fclose(f);
    close(client);

    printf("Done.");
}