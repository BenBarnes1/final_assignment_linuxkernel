#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

volatile sig_atomic_t graceful_flag = 0; // Cờ ngắt khi nhấn Ctrl+C

// Hàm xử lý tín hiệu Ctrl+C
void handle_signal(int sig) {
    if (sig == SIGINT) graceful_flag = 1;
}

// Kiểm tra chuỗi có phải số không (Xác thực PID)
int is_numeric_string(const char *str) {
    if (*str == '\0') return 0;
    while (*str) {
        if (!isdigit(*str)) return 0;
        str++;
    }
    return 1;
}

// Chức năng 1: Giám sát danh sách tiến trình nâng cao từ /proc
void displayRunningProcesses() {
    DIR *proc_dir = opendir("/proc");
    if (proc_dir == NULL) {
        perror("[ERROR] Không thể mở /proc");
        return;
    }

    struct dirent *entry;
    char path[512], line[256];
    printf("\n%-10s %-25s %-15s %-15s\n", "PID", "TÊN TIẾN TRÌNH", "TRẠNG THÁI", "BỘ NHỚ ẢO");
    printf("----------------------------------------------------------------------------\n");

    while ((entry = readdir(proc_dir)) != NULL) {
        // Lọc thư mục có tên là số (PID)
        if (entry->d_type == DT_DIR && is_numeric_string(entry->d_name)) {
            char proc_name[256] = "N/A", proc_state[256] = "N/A", proc_vmsize[256] = "N/A";

            snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);
            FILE *file = fopen(path, "r");
            if (file != NULL) {
                while (fgets(line, sizeof(line), file)) {
                    // Đọc bóc tách thông tin Tên, Trạng thái, Bộ nhớ ảo
                    if (strncmp(line, "Name:", 5) == 0) {
                        char *start = line + 5; while (*start == ' ' || *start == '\t') start++;
                        char *end = strchr(start, '\n'); if (end) *end = '\0';
                        strncpy(proc_name, start, sizeof(proc_name) - 1);
                    }
                    else if (strncmp(line, "State:", 6) == 0) {
                        char *start = line + 6; while (*start == ' ' || *start == '\t') start++;
                        char *end = strchr(start, '\n'); if (end) *end = '\0';
                        strncpy(proc_state, start, sizeof(proc_state) - 1);
                    }
                    else if (strncmp(line, "VmSize:", 7) == 0) {
                        char *start = line + 7; while (*start == ' ' || *start == '\t') start++;
                        char *end = strchr(start, '\n'); if (end) *end = '\0';
                        strncpy(proc_vmsize, start, sizeof(proc_vmsize) - 1);
                    }
                }
                fclose(file);
                printf("%-10s %-25s %-15s %-15s\n", entry->d_name, proc_name, proc_state, proc_vmsize);
            }
        }
    }
    closedir(proc_dir);
}

// Chức năng 2: Gửi tín hiệu điều khiển tùy biến tới PID
void controlProcess() {
    pid_t pid;
    int sig_choice, target_signal = SIGTERM;

    printf("\nNhập PID mục tiêu: ");
    if (scanf("%d", &pid) != 1 || pid <= 0) { printf("PID lỗi.\n"); getchar(); return; }

    printf("1. SIGTERM | 2. SIGKILL | 3. SIGSTOP | 4. SIGCONT\nChọn tín hiệu: ");
    if (scanf("%d", &sig_choice) != 1) { printf("Lựa chọn lỗi.\n"); getchar(); return; }
    getchar(); // Clear buffer

    if (sig_choice == 2) target_signal = SIGKILL;
    else if (sig_choice == 3) target_signal = SIGSTOP;
    else if (sig_choice == 4) target_signal = SIGCONT;

    if (kill(pid, target_signal) == 0) printf("[SUCCESS] Đã gửi tín hiệu.\n");
    else perror("[ERROR] Thất bại");
}

// Chức năng 3: Sinh tiến trình con chạy lệnh hệ thống độc lập (Fork + Execvp)
void spawnAndExecute() {
    pid_t child_pid;
    char cmd_input[256];

    printf("\nNhập lệnh (VD: 'ls -la' hoặc 'ping -c 3 google.com'): ");
    if (fgets(cmd_input, sizeof(cmd_input), stdin) == NULL) return; // Kiểm tra lỗi fgets
    
    char *newline = strchr(cmd_input, '\n'); if (newline) *newline = '\0';
    if (strlen(cmd_input) == 0) return;

    // Tách chuỗi nhập vào thành các đối số cho mảng args
    char *args[64]; int i = 0;
    char *token = strtok(cmd_input, " ");
    while (token != NULL && i < 63) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    graceful_flag = 0; // Reset cờ ngắt
    child_pid = fork();

    if (child_pid < 0) { perror("Fork lỗi"); return; }
    else if (child_pid == 0) {
        // Tiến trình con nạp đè chương trình mới
        if (execvp(args[0], args) == -1) {
            perror("Execvp lỗi"); exit(EXIT_FAILURE);
        }
    }
    else {
        // Tiến trình cha theo dõi con phi nghẽn (WNOHANG)
        printf("[PARENT] Đã tạo tiến trình con PID = %d. Nhấn Ctrl+C để hủy.\n", child_pid);
        int status; pid_t wait_result;

        while (!graceful_flag) {
            wait_result = waitpid(child_pid, &status, WNOHANG);
            if (wait_result == child_pid) { // Con tự hoàn thành nhiệm vụ
                if (WIFEXITED(status)) printf("Con thoát bình thường, mã: %d\n", WEXITSTATUS(status));
                return;
            }
            usleep(100000); // Ngủ 100ms tránh nghẽn CPU (busy-waiting)
        }

        // Cưỡng chế hủy tiến trình con nếu cha nhận Ctrl+C
        printf("\nBị ngắt. Tiến hành dọn dẹp tiến trình con...\n");
        kill(child_pid, SIGKILL); waitpid(child_pid, NULL, 0);
    }
}

int main() {
    int choice;
    if (signal(SIGINT, handle_signal) == SIG_ERR) exit(1); // Đăng ký Ctrl+C

    do {
        printf("\n=========================================\n");
        printf("   HỆ THỐNG QUẢN LÝ TIẾN TRÌNH NÂNG CAO   \n");
        printf("=========================================\n");
        printf("1. Giám sát hệ thống tiến trình (/proc)\n");
        printf("2. Gửi tín hiệu điều khiển (Kill/Stop/Cont)\n");
        printf("3. Khởi tạo tiến trình con chạy lệnh (Execvp)\n");
        printf("4. Thoát\n");
        printf("Nhập lựa chọn (1-4): ");
        
        if (scanf("%d", &choice) != 1) { getchar(); choice = 0; continue; }
        getchar(); // Đọc ký tự newline

        switch (choice) {
            case 1: displayRunningProcesses(); break;
            case 2: controlProcess(); break;
            case 3: spawnAndExecute(); break;
            case 4: printf("Đang đóng ứng dụng.\n"); break;
            default: printf("Lựa chọn không hợp lệ.\n"); break;
        }
    } while (choice != 4);
    return 0;
}
