#include <stdio.h>
#include <pthread.h>
#define NLOOP 1000
int counter;
int counter_mutex = 0; // dùng để quản lý tài nguyên dùng chung
// pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER; // dùng để quản lý tài nguyên dùng chung

void *doit(void *);
int main()
{
    pthread_t t1, t2;
    pthread_create(&t1, NULL, doit, NULL); // tạo luồng mới thực thi hàm doit
    pthread_create(&t2, NULL, doit, NULL); // tạo luồng mới thực thi hàm doit

    pthread_join(t1, NULL); // đợi luồng t1 kết thúc
    pthread_join(t2, NULL); // đợi luồng t2 kết thúc
    printf("s");
}

void *doit(void *arg)
{
    for (int i = 0; i < NLOOP; ++i)
    {
        pthread_mutex_lock(&counter_mutex); // khóa biến mutex lại cho tới khi một luồng khác mở.
        int val = counter;
        printf("%ld: %d\n", pthread_self(), val + 1);
        counter = val + 1;
        pthread_mutex_unlock(&counter_mutex);
    }
}