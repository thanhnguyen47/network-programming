#include <stdio.h>
#include <string.h>

int main() {
    char str[] = "Hello, world!";
    size_t size = sizeof(str); // Kích thước của mảng str
    size_t length = strlen(str); // Độ dài của xâu str

    printf("Kich thuoc: %zu byte\n", size);
    printf("Do dai: %zu ky tu\n", length);

    return 0;
}
