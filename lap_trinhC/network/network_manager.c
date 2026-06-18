#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/wait.h>

#define LOG_INFO(msg, ...) printf("[INFO] " msg "\n", ##__VA_ARGS__)
#define LOG_ERR(msg, ...) fprintf(stderr, "[ERROR] " msg "\n", ##__VA_ARGS__)

void clear_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_input(const char* prompt, char* buffer, size_t size) {
    printf("%s", prompt);
    if (fgets(buffer, size, stdin) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        } else {
            clear_stdin();
        }
    }
}

int execute_system_command(const char* cmd) {
    int ret = system(cmd);
    if (ret == -1) {
        LOG_ERR("Khong the khoi tao shell de chay lenh: %s. Chi tiet: %s", cmd, strerror(errno));
        return -1;
    }
    if (WIFEXITED(ret) && WEXITSTATUS(ret) != 0) {
        LOG_ERR("Lenh '%s' that bai voi ma loi %d. Ban da chay voi quyen sudo chua?", cmd, WEXITSTATUS(ret));
        return -1;
    }
    return 0;
}

void list_network_interfaces() {
    struct ifaddrs *ifaddr;
    int family, s;
    char host[NI_MAXHOST];
    
    if (getifaddrs(&ifaddr) == -1) {
        LOG_ERR("Loi khi goi getifaddrs: %s", strerror(errno));
        return;
    }
    
    LOG_INFO("Danh sach thong tin giao dien mang:");
    
    for (struct ifaddrs *ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        
        family = ifa->ifa_addr->sa_family;
        
        printf("  %-8s %s (%d)\n",
               ifa->ifa_name,
               (family == AF_PACKET) ? "AF_PACKET" :
               (family == AF_INET) ? "AF_INET (IPv4)" :
               (family == AF_INET6) ? "AF_INET6 (IPv6)" : "Khac",
               family);
        
        if (family == AF_INET || family == AF_INET6) {
            s = getnameinfo(ifa->ifa_addr,
                            (family == AF_INET) ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
                            host, NI_MAXHOST,
                            NULL, 0, NI_NUMERICHOST);
            if (s != 0) {
                LOG_ERR("getnameinfo() that bai: %s", gai_strerror(s));
            } else {
                printf("           Dia chi IP: <%s>\n", host);
            }
        } else if (family == AF_PACKET && ifa->ifa_data != NULL) {
            struct ifreq ifr;
            strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = '\0';
            
            int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sockfd >= 0) {
                if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == 0) {
                    printf("           Trang thai: %s\n", (ifr.ifr_flags & IFF_UP) ? "UP" : "DOWN");
                }
                close(sockfd);
            }
        }
    }
    freeifaddrs(ifaddr);
}

void enable_interface(const char *interface_name) {
    char command[256];
    snprintf(command, sizeof(command), "ip link set dev %s up", interface_name);
    
    LOG_INFO("Dang bat giao dien '%s'...", interface_name);
    if (execute_system_command(command) == 0) {
        LOG_INFO("Giao dien '%s' da duoc bat thanh cong.", interface_name);
    }
}

void disable_interface(const char *interface_name) {
    char command[256];
    snprintf(command, sizeof(command), "ip link set dev %s down", interface_name);
    
    LOG_INFO("Dang tat giao dien '%s'...", interface_name);
    if (execute_system_command(command) == 0) {
        LOG_INFO("Giao dien '%s' da duoc tat thanh cong.", interface_name);
    }
}

void change_ip_add(const char *interface_name) {
    char ip_address[64];
    char command[256];
    
    get_input("Nhap dia chi IP moi (vi du: 192.168.1.100/24): ", ip_address, sizeof(ip_address));
    if (strlen(ip_address) == 0) {
        LOG_ERR("Dia chi IP khong duoc de trong.");
        return;
    }

    LOG_INFO("Dang thiet lap IP moi cho giao dien '%s'...", interface_name);
    
    snprintf(command, sizeof(command), "ip addr flush dev %s", interface_name);
    if (execute_system_command(command) != 0) return;
    
    snprintf(command, sizeof(command), "ip addr add %s dev %s", ip_address, interface_name);
    if (execute_system_command(command) != 0) return;
    
    snprintf(command, sizeof(command), "ip link set dev %s up", interface_name);
    if (execute_system_command(command) == 0) {
        LOG_INFO("Da thay doi dia chi IP thanh cong thanh %s.", ip_address);
    }
}

int main() {
    char choice_str[10];
    char interface_name[100];
    int choice;

    while(1) {
        printf("\n");
        LOG_INFO("CHUONG TRINH QUAN LY GIAO DIEN MANG (NETWORK INTERFACES)");
        printf("1. Liet ke cac giao dien mang\n");
        printf("2. Bat (Enable) mot giao dien\n");
        printf("3. Tat (Disable) mot giao dien\n");
        printf("4. Thay doi dia chi IP cua mot giao dien\n");
        printf("5. Thoat\n");
        
        get_input("Lua chon cua ban (1-5): ", choice_str, sizeof(choice_str));
        choice = atoi(choice_str);

        if (choice >= 2 && choice <= 4) {
            get_input("Nhap ten giao dien mang (vi du: eth0, lo, ens33): ", interface_name, sizeof(interface_name));
            if (strlen(interface_name) == 0) {
                LOG_ERR("Ten giao dien khong duoc de trong.");
                continue;
            }
        }

        switch(choice) {
            case 1: list_network_interfaces(); break;
            case 2: enable_interface(interface_name); break;
            case 3: disable_interface(interface_name); break;
            case 4: change_ip_add(interface_name); break;
            case 5: 
                LOG_INFO("Thoat chuong trinh."); 
                return EXIT_SUCCESS;
            default:
                LOG_ERR("Lua chon khong hop le. Vui long thu lai.");
        }
    }
    return EXIT_SUCCESS;
}
