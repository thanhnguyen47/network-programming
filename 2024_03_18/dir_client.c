#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h> // to use getcwd
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char* argv[]) 
{
    char buf[2048];
    int pos = 0;

    char path[256];
    getcwd(path, sizeof(path)); // retrieve the current working directory of the calling program
    strcpy(buf, path);
    pos += strlen(path) + 1;
    
    DIR *d = opendir(path); // mở thư mục chỉ định bởi đường dẫn. con trỏ d là đại diện cho thư mục
    struct dirent *dir;
    struct stat st;

    if (d != NULL)
        while ((dir = readdir(d)) != NULL) { // đọc tất cả các file trong d, lưu vào cấu trúc dir
            if (dir->d_type == DT_REG) { // kiểm tra có phải là file thông thường hay không. regular file.
                stat(dir->d_name, &st); // lấy thông tin về tệp (kích thước, quyền truy cập, ...) và lưu vào cấu trúc st.
                printf("%s - %ld bytes\n", dir->d_name, st.st_size);

                strcpy(buf + pos, dir->d_name); // sao chép tên tệp vào buffer từ vị trí pos
                pos += strlen(dir->d_name) + 1; // cập nhật vị trí mới trong buffer
                memcpy(buf + pos, &st.st_size, sizeof(st.st_size)); // sao chép kích thước tệp vào buffer ----> cần sử dụng memset vì đây là giá trị số  nguyên.
                pos += sizeof(st.st_size); // cập nhật pos


                // ==> lưu hết một đống vào buf rồi gửi đi một lượt.
            }
        }

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    // Gui sang server
    send(client, buf, pos, 0);

    close(client);

    return 0;
}