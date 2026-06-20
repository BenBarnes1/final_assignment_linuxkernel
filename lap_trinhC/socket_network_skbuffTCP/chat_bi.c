#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/tcp.h>

#define TARGET_PORT 5001

int connection_socket = -1;
char device_path[64];

void *network_receive_thread(void *arg)
{
    char buffer[1024];
    while (1)
    {
        int n = recv(connection_socket, buffer, sizeof(buffer), 0);
        if (n <= 0)
            exit(EXIT_FAILURE);
        for (int i = 0; i < n; i++)
        {
            if (buffer[i] == 'X')
            {
                send(connection_socket, "Y", 1, 0);
            }
        }
    }
    return NULL;
}

void *kernel_read_thread(void *arg)
{
    int fd = open(device_path, O_RDONLY);
    char buf[256], line_buffer[512];
    int line_idx = 0;

    printf("\n--- DA VAO KHOI CHAT BẢO MẬT ---\n\n> ");
    fflush(stdout);

    while (1)
    {
        int n = read(fd, buf, 255);
        if (n > 0)
        {
            for (int i = 0; i < n; i++)
            {
                if (buf[i] == '\n' || line_idx >= 510)
                {
                    line_buffer[line_idx] = '\0';
                    printf("\r\033[K[Doi phuong]: %s\n> ", line_buffer);
                    fflush(stdout);
                    line_idx = 0;
                }
                else
                {
                    line_buffer[line_idx++] = buf[i];
                }
            }
        }
    }
    return NULL;
}

void handle_chat()
{
    char msg[256];
    pthread_t net_tid, kern_tid;

    int flag = 1;
    setsockopt(connection_socket, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));

    pthread_create(&net_tid, NULL, network_receive_thread, NULL);
    pthread_create(&kern_tid, NULL, kernel_read_thread, NULL);

    int fd = open(device_path, O_WRONLY);
    if (fd < 0)
    {
        printf("Loi: Vui long chay bang Sudo.\n");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        if (fgets(msg, sizeof(msg), stdin) == NULL)
            continue;
        int len = strlen(msg);
        if (len <= 1)
        {
            printf("> ");
            fflush(stdout);
            continue;
        }

        for (int i = 0; i < len; i++)
        {
            write(fd, &msg[i], 1);
            send(connection_socket, "X", 1, 0);
            usleep(15000);
        }
    }
    close(fd);
}

void run_server()
{
    int server_fd, opt = 1;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(TARGET_PORT);
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);
    printf("[Server] Dang doi doi phuong...\n");
    connection_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    handle_chat();
    close(server_fd);
}

void run_client(char *server_ip)
{
    struct sockaddr_in serv_addr;
    connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(TARGET_PORT);
    inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);
    if (connect(connection_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        exit(EXIT_FAILURE);
    handle_chat();
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        return EXIT_FAILURE;
    if (strcmp(argv[1], "server") == 0)
    {
        strcpy(device_path, "/dev/stego_server");
        run_server();
    }
    else if (strcmp(argv[1], "client") == 0 && argc == 3)
    {
        strcpy(device_path, "/dev/stego_client");
        run_client(argv[2]);
    }
    return EXIT_SUCCESS;
}
