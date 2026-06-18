#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

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

int file_exists(const char* filename) {
    if (access(filename, F_OK) == 0) {
        return 1;
    }
    return 0;
}

void create_file(const char* filename) {
    if (file_exists(filename)) {
        LOG_ERR("File '%s' da ton tai.", filename);
        return;
    }
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        LOG_ERR("Khong the tao file '%s'. Chi tiet: %s", filename, strerror(errno));
    } else {
        LOG_INFO("File '%s' da duoc tao thanh cong.", filename);
        fclose(file);
    }
}

void display_file(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        LOG_ERR("Khong the mo file '%s' de doc. Chi tiet: %s", filename, strerror(errno));
        return;
    }
    
    char buffer[1024];
    LOG_INFO("Noi dung file '%s':", filename);
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }
    printf("\n");
    fclose(file);
}

void edit_file(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "nano %s", filename);
    
    int ret = system(cmd);
    if (ret == -1) {
        LOG_ERR("Khong the khoi tao tien trinh chinh sua file.");
    } else {
        LOG_INFO("Da hoan tat chinh sua file '%s'.", filename);
    }
}

void check_permissions(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    struct stat st;
    if (stat(filename, &st) == 0) {
        LOG_INFO("Quyen cua file '%s':", filename);
        printf("  Chu so huu - Doc: %s, Ghi: %s, Thuc thi: %s\n",
               (st.st_mode & S_IRUSR) ? "Co" : "Khong",
               (st.st_mode & S_IWUSR) ? "Co" : "Khong",
               (st.st_mode & S_IXUSR) ? "Co" : "Khong");
        printf("  Nhom       - Doc: %s, Ghi: %s, Thuc thi: %s\n",
               (st.st_mode & S_IRGRP) ? "Co" : "Khong",
               (st.st_mode & S_IWGRP) ? "Co" : "Khong",
               (st.st_mode & S_IXGRP) ? "Co" : "Khong");
        printf("  Khac       - Doc: %s, Ghi: %s, Thuc thi: %s\n",
               (st.st_mode & S_IROTH) ? "Co" : "Khong",
               (st.st_mode & S_IWOTH) ? "Co" : "Khong",
               (st.st_mode & S_IXOTH) ? "Co" : "Khong");
    } else {
        LOG_ERR("Khong the kiem tra quyen cua file '%s'. Chi tiet: %s", filename, strerror(errno));
    }
}

void delete_file(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    if (remove(filename) == 0) {
        LOG_INFO("File '%s' da duoc xoa thanh cong.", filename);
    } else {
        LOG_ERR("Khong the xoa file '%s'. Chi tiet: %s", filename, strerror(errno));
    }
}

void compress_file(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gzip -f %s", filename);
    int ret = system(cmd);
    if (ret == -1 || WEXITSTATUS(ret) != 0) {
        LOG_ERR("Qua trinh nen file '%s' that bai.", filename);
    } else {
        LOG_INFO("Da nen file '%s' thanh cong (tao file .gz).", filename);
    }
}

void decompress_file(const char* filename) {
    if (!file_exists(filename)) {
        LOG_ERR("File '%s' khong ton tai.", filename);
        return;
    }
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "gzip -d -f %s", filename);
    int ret = system(cmd);
    if (ret == -1 || WEXITSTATUS(ret) != 0) {
        LOG_ERR("Qua trinh giai nen file '%s' that bai.", filename);
    } else {
        LOG_INFO("Da giai nen file '%s' thanh cong.", filename);
    }
}

int main() {
    char filename[256];
    char choice_str[10];
    int ch;
    
    while(1) {
        printf("\n");
        LOG_INFO("CHUONG TRINH QUAN LY FILE UBUNTU");
        printf("1. Tao file moi\n");
        printf("2. Hien thi noi dung file\n");
        printf("3. Chinh sua file (nano)\n");
        printf("4. Kiem tra quyen cua file\n");
        printf("5. Xoa file\n");
        printf("6. Nen file (gzip)\n");
        printf("7. Giai nen file (gzip)\n");
        printf("8. Thoat\n");
        
        get_input("Lua chon cua ban (1-8): ", choice_str, sizeof(choice_str));
        ch = atoi(choice_str);
        
        if (ch >= 1 && ch <= 7) {
            get_input("Nhap ten file thao tac: ", filename, sizeof(filename));
            if (strlen(filename) == 0) {
                LOG_ERR("Ten file khong duoc de trong.");
                continue;
            }
        }
        
        switch(ch) {
            case 1: create_file(filename); break;
            case 2: display_file(filename); break;
            case 3: edit_file(filename); break;
            case 4: check_permissions(filename); break;
            case 5: delete_file(filename); break;
            case 6: compress_file(filename); break;
            case 7: decompress_file(filename); break;
            case 8: 
                LOG_INFO("Thoat chuong trinh."); 
                return EXIT_SUCCESS;
            default:
                LOG_ERR("Lua chon khong hop le. Vui long nhap tu 1 den 8.");
        }
    }
    return EXIT_SUCCESS;
}
