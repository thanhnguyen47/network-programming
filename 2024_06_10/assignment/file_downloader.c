#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <malloc.h>

int https_download(char *url)
{
    SSL_library_init();                           // Khởi tạo thư viện OpenSSL
    const SSL_METHOD *meth = TLS_client_method(); // Khai báo phương thức mã hóa TLS
    SSL_CTX *ctx = SSL_CTX_new(meth);             // Tạo context mới
    SSL *ssl = SSL_new(ctx);                      // Tạo con trỏ ssl
    if (!ssl)
    {
        printf("Error creating SSL.\n");
        return -1;
    }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct addrinfo *info;

    // https://lebavui.github.io/videos/ecard.mp4

    char *pos1 = strstr(url, "://") + 3;
    char *pos2 = strchr(pos1, '/');

    char domain[32] = {0};
    char request[100] = {0};

    memcpy(domain, pos1, pos2 - pos1);
    memcpy(request, pos2, strlen(pos2));

    if (getaddrinfo(domain, "https", NULL, &info))
    {
        puts("getaddrinfo() failed");
        return -1;
    }

    if (connect(client, info->ai_addr, info->ai_addrlen))
    {
        perror("connect() failed");
        return -1;
    }

    SSL_set_fd(ssl, client);    // Gắn con trỏ ssl với socket
    int err = SSL_connect(ssl); // Tạo kết nối ssl
    if (err <= 0)
    {
        printf("Error creating SSL connection.  err=%x\n", err);
        return -1;
    }

    char req[1024];
    sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", request, domain);
    SSL_write(ssl, req, strlen(req));

    char buf[2048];
    // Nhan noi dung phan header
    char *header = NULL;
    int len = 0;

    while (1)
    {
        int ret = SSL_read(ssl, buf, sizeof(buf));
        if (ret <= 0)
        {
            close(client);
            printf("disconnected\n");
            return 1;
        }

        header = realloc(header, len + ret);
        memcpy(header + len, buf, ret);
        len += ret;

        if (memmem(header, len, "\r\n\r\n", 4) != NULL)
        {
            break;
        }
    }

    // In header
    puts(header);

    char *pos = memmem(header, len, "\r\n\r\n", 4);

    FILE *f = fopen("download.mp4", "wb");

    // Nhan noi dung file
    int header_length = pos - header;
    if (header_length + 4 < len)
        fwrite(pos + 4, 1, len - header_length - 4, f);

    while (1)
    {
        int ret = SSL_read(ssl, buf, sizeof(buf));
        if (ret <= 0)
            break;
        fwrite(buf, 1, ret, f);
    }

    free(header);

    fclose(f);
    close(client);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ctx);
}

int http_download(char *url)
{
    // Download file http://storage.googleapis.com/it4060_samples/ecard.mp4

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    char *pos1 = strstr(url, "://") + 3;
    char *pos2 = strchr(pos1, '/');

    char domain[32] = {0};
    char request[100] = {0};

    memcpy(domain, pos1, pos2 - pos1);
    memcpy(request, pos2, strlen(pos2));

    // Phan giai ten mien
    struct addrinfo *info;
    if (getaddrinfo(domain, "http", NULL, &info))
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

    char req[1024];
    sprintf(req, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", request, domain);
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

int main(int argc, char *argv[])
{
    if (strncmp(argv[1], "https", 5) == 0)
    {
        https_download(argv[1]);
    }
    else if (strncmp(argv[1], "http", 4) == 0)
    {
        http_download(argv[1]);
    }

    return 0;
}