#include <stdio.h>
#include <arpa/inet.h>

void main() {
    printf("Hello\n");
    short int N = 8000;
    printf("%d", htons(N));
}