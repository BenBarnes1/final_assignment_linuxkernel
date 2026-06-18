#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "B2.h"

#define LOG_INFO(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOG_ERR(msg, ...) fprintf(stderr, "[ERROR] " msg "\n", ##__VA_ARGS__)

int server_fd;

void *receive_routine(void *arg) {
    (void)arg; 
    char buff[BUFF_SIZE];
    int numRead;
    
    while ((numRead = recv(server_fd, buff, BUFF_SIZE - 1, 0)) > 0) {
        buff[numRead] = '\0';
        printf("%s", buff);
        fflush(stdout);
    }
    
    LOG_INFO("\nServer da ngat ket noi.");
    exit(EXIT_SUCCESS);
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t tid;
    char userName[NAME_SIZE];
    char buff[BUFF_SIZE];
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERR("Khong the tao socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NUM);
    
    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        LOG_ERR("Dia chi IP server khong hop le.");
        exit(EXIT_FAILURE);
    }
    
    printf("Nhap ten hien thi cua ban: ");
    if (fgets(userName, NAME_SIZE, stdin) != NULL) {
        userName[strcspn(userName, "\n")] = '\0';
    }
    
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        LOG_ERR("Khong the ket noi den server %s:%d: %s", SERVER_ADDR, PORT_NUM, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (send(server_fd, userName, strlen(userName), 0) == -1) {
        LOG_ERR("Loi khi gui ten nguoi dung den server.");
        exit(EXIT_FAILURE);
    }
    LOG_INFO("Da ket noi thanh cong den Chat Server!");
    
    if (pthread_create(&tid, NULL, receive_routine, NULL) != 0) {
        LOG_ERR("Khong the tao luong nhan du lieu.");
        exit(EXIT_FAILURE);
    }
    
    while (fgets(buff, BUFF_SIZE, stdin) != NULL) {
        if (send(server_fd, buff, strlen(buff), 0) == -1) {
            LOG_ERR("Loi khi gui tin nhan.");
            break;
        }
    }
    
    close(server_fd);
    return EXIT_SUCCESS;
}
