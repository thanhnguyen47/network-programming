#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

void signalHandler(int signo) {
    int pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}

int main() {
    signal(SIGCHLD, signalHandler); // đăng ký việc xử lý khi sự kiện SIGCHILD xảy ra.
    // SIGCHILD gửi bởi hệ thống tới tiến trình cha khi tiến trình con kết thúc.

    // Nếu không được xử lý bởi cha, tiến trình con sẽ ở trạng thái zombie.
    
    if (fork() == 0) {
        printf("Child process started, pid = %d\n", getpid());
        sleep(3); // trong thư viện sys/wait.h tính bằng s
        printf("Child process done, pid = %d\n", getpid());
        exit(0);
    }

    getchar(); // dừng tiến trình cha lại đến khi nhấn 1 phím bất kỳ thì làm tiếp.
    // các tiến trình khác vẫn chạy bình thường.
    printf("Main process done.\n");
    return 0;
}