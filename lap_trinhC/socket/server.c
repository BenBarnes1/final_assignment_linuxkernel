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

#define BACK_LOGS 10
#define LOG_INFO(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOG_ERR(msg, ...) fprintf(stderr, "[ERROR] " msg "\n", ##__VA_ARGS__)

typedef struct ClientNode {
    int client_fd;
    char userName[NAME_SIZE];
    struct ClientNode *next;
} client_node;

client_node *head = NULL;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(int c_fd, const char *userName) {
    client_node *new_node = (client_node *)malloc(sizeof(client_node));
    if (!new_node) {
        LOG_ERR("Loi cap phat bo nho cho client moi.");
        return;
    }
    new_node->client_fd = c_fd;
    strncpy(new_node->userName, userName, NAME_SIZE - 1);
    new_node->userName[NAME_SIZE - 1] = '\0';
    
    pthread_mutex_lock(&clients_mutex);
    new_node->next = head;
    head = new_node;
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int c_fd) {
    pthread_mutex_lock(&clients_mutex);
    client_node *current = head;
    client_node *prev = NULL;
    
    while (current != NULL) {
        if (current->client_fd == c_fd) {
            if (prev == NULL) {
                head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast_message(const char *message, int sender_fd) {
    pthread_mutex_lock(&clients_mutex);
    client_node *current = head;
    while (current != NULL) {
        if (current->client_fd != sender_fd) {
            if (send(current->client_fd, message, strlen(message), 0) == -1) {
                LOG_ERR("Loi khi gui tin nhan toi fd %d: %s", current->client_fd, strerror(errno));
            }
        }
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *client_routine(void *arg) {
    int c_fd = *(int *)arg;
    free(arg); 
    
    char userName[NAME_SIZE];
    char buff[BUFF_SIZE];
    char broadcast_buff[BUFF_SIZE + NAME_SIZE + 5];
    
    memset(userName, 0, NAME_SIZE);
    if (recv(c_fd, userName, NAME_SIZE - 1, 0) <= 0) {
        LOG_ERR("Khong the nhan ten nguoi dung hoac ngat ket noi. FD: %d", c_fd);
        close(c_fd);
        pthread_exit(NULL);
    }
    
    add_client(c_fd, userName);
    LOG_INFO("Client connected: %s (FD: %d)", userName, c_fd);
    
    snprintf(broadcast_buff, sizeof(broadcast_buff), "--- %s da tham gia ---", userName);
    broadcast_message(broadcast_buff, c_fd);
    
    int numRead;
    while ((numRead = recv(c_fd, buff, BUFF_SIZE - 1, 0)) > 0) {
        buff[numRead] = '\0';
        snprintf(broadcast_buff, sizeof(broadcast_buff), "%s: %s", userName, buff);
        broadcast_message(broadcast_buff, c_fd);
    }
    
    LOG_INFO("Client disconnected: %s (FD: %d)", userName, c_fd);
    snprintf(broadcast_buff, sizeof(broadcast_buff), "--- %s da roi di ---", userName);
    broadcast_message(broadcast_buff, c_fd);
    
    remove_client(c_fd);
    close(c_fd);
    pthread_exit(NULL);
}

int main() {
    int server_fd;
    struct sockaddr_in sock_addr;
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        LOG_ERR("Khong the tao socket: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOG_ERR("Khong the thiet lap SO_REUSEADDR: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(PORT_NUM);
    
    if (inet_pton(AF_INET, SERVER_ADDR, &sock_addr.sin_addr) <= 0) {
        LOG_ERR("Dia chi IP khong hop le hoac khong duoc ho tro.");
        exit(EXIT_FAILURE);
    }
    
    if (bind(server_fd, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == -1) {
        LOG_ERR("Khong the bind socket toi cong %d: %s", PORT_NUM, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, BACK_LOGS) == -1) {
        LOG_ERR("Loi khi thuc thi listen: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    LOG_INFO("Server dang lang nghe tren %s:%d...", SERVER_ADDR, PORT_NUM);
    
    while (1) {
        int *client_fd = malloc(sizeof(int));
        if (!client_fd) {
            LOG_ERR("Loi cap phat bo nho cho FD moi.");
            continue;
        }
        
        *client_fd = accept(server_fd, NULL, NULL);
        if (*client_fd == -1) {
            LOG_ERR("Loi khi accept ket noi: %s", strerror(errno));
            free(client_fd);
            continue;
        }
        
        pthread_t tid;
        if (pthread_create(&tid, NULL, client_routine, (void *)client_fd) != 0) {
            LOG_ERR("Khong the tao thread cho client: %s", strerror(errno));
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(tid);
        }
    }
    
    close(server_fd);
    return EXIT_SUCCESS;
}
