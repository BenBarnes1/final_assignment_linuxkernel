#!/bin/bash

# Hàm hiển thị thanh phân cách cho đẹp giao diện
print_sep() {
    echo "=========================================================="
}

# Hàm chờ người dùng nhấn Enter để tiếp tục
pause() {
    read -p "Nhấn [Enter] để tiếp tục..." temp
}

while true; do
    clear
    print_sep
    echo "    BASH QUẢN LÝ FILE & THƯ MỤC NÂNG CAO - shell"
    print_sep
    echo "Thư mục hiện tại: $(pwd)"
    print_sep
    echo "1.  Tạo file mới                  11. Sao chép file/thư mục"
    echo "2.  Tạo thư mục mới               12. Di chuyển/Đổi tên file/thư mục"
    echo "3.  Hiển thị nội dung file        13. Nén file (gzip)"
    echo "4.  Chỉnh sửa file (nano)         14. Giải nén file (gunzip)"
    echo "5.  Xem danh sách file/thư mục    15. Xóa file"
    echo "6.  Điều hướng (Vào thư mục con)  16. Xóa thư mục"
    echo "7.  Quay lại thư mục cha (.. )    17. Đổi thuộc tính: Chỉ đọc"
    echo "8.  Kiểm tra quyền (ls -l)        18. Đổi thuộc tính: Ẩn file/thư mục"
    echo "9.  Thay đổi quyền (chmod)        19. [Thêm] Tìm kiếm file nhanh"
    echo "10. Xem dung lượng ổ đĩa (df -h)  20. THOÁT CHƯƠNG TRÌNH"
    print_sep
    
    read -p "Chọn chức năng (1-20): " choice
    echo ""

    case $choice in
        1)
            read -p "Nhập tên file mới cần tạo: " file
            if [ -z "$file" ]; then echo "Lỗi: Tên file không được để trống!";
            elif [ -e "$file" ]; then echo "Lỗi: File/Thư mục đã tồn tại!";
            else touch "$file" && echo "Tạo file '$file' thành công."; fi
            pause ;;
            
        2)
            read -p "Nhập tên thư mục mới cần tạo: " dir
            if [ -z "$dir" ]; then echo "Lỗi: Tên thư mục không được để trống!";
            elif [ -e "$dir" ]; then echo "Lỗi: Thư mục/File đã tồn tại!";
            else mkdir -p "$dir" && echo "Tạo thư mục '$dir' thành công."; fi
            pause ;;
            
        3)
            read -p "Nhập tên file cần xem: " file
            if [ -f "$file" ]; then cat "$file";
            else echo "Lỗi: File không tồn tại hoặc là thư mục!"; fi
            pause ;;
            
        4)
            read -p "Nhập tên file cần sửa: " file
            if [ -z "$file" ]; then echo "Lỗi: Chưa nhập tên file!";
            else nano "$file"; fi ;;
            
        5)
            echo "Danh sách file và thư mục trong '$(pwd)':"
            print_sep
            ls -F --color=auto
            print_sep
            pause ;;
            
        6)
            read -p "Nhập tên thư mục con muốn vào: " dir
            if [ -d "$dir" ]; then cd "$dir" && echo "Đã chuyển đến: $(pwd)";
            else echo "Lỗi: Thư mục không tồn tại!"; fi
            echo "Nhấn Enter để quay lại Menu..." && read temp ;;
            
        7)
            cd .. && echo "Đã quay lại thư mục cha: $(pwd)"
            echo "Nhấn Enter để quay lại Menu..." && read temp ;;
            
        8)
            read -p "Nhập tên file/thư mục cần kiểm tra quyền: " item
            if [ -e "$item" ]; then ls -l "$item";
            else echo "Lỗi: Đối tượng không tồn tại!"; fi
            pause ;;
            
        9)
            read -p "Nhập tên file/thư mục: " item
            if [ -e "$item" ]; then
                read -p "Nhập quyền mới (Ví dụ: 755, u+x, g-w): " perm
                if [ -z "$perm" ]; then echo "Lỗi: Quyền không được để trống!";
                else chmod "$perm" "$item" && echo "Thay đổi quyền thành công."; fi
            else echo "Lỗi: Đối tượng không tồn tại!"; fi
            pause ;;
            
        10)
            df -h .
            pause ;;
            
        11)
            read -p "Nhập nguồn (file/thư mục cần copy): " src
            read -p "Nhập đường dẫn đích (thư mục hoặc tên mới): " dest
            if [ ! -e "$src" ]; then echo "Lỗi: Nguồn không tồn tại!";
            elif [ -z "$dest" ]; then echo "Lỗi: Đích không được để trống!";
            else cp -r "$src" "$dest" && echo "Sao chép thành công."; fi
            pause ;;
            
        12)
            read -p "Nhập file/thư mục cần Di chuyển/Đổi tên: " src
            read -p "Nhập đường dẫn đích hoặc tên mới: " dest
            if [ ! -e "$src" ]; then echo "Lỗi: Đối tượng nguồn không tồn tại!";
            elif [ -z "$dest" ]; then echo "Lỗi: Đích không được để trống!";
            else mv "$src" "$dest" && echo "Di chuyển/Đổi tên thành công."; fi
            pause ;;
            
        13)
            read -p "Nhập tên file cần nén (Gzip): " file
            if [ -f "$file" ]; then gzip "$file" && echo "Nén file thành công (Tạo ra file .gz).";
            else echo "Lỗi: Không tìm thấy file hoặc đây là một thư mục!"; fi
            pause ;;
            
        14)
            read -p "Nhập tên file .gz cần giải nén: " file
            if [ -f "$file" ]; then 
                if [[ "$file" == *.gz ]]; then gunzip "$file" && echo "Giải nén thành công.";
                else echo "Lỗi: File không đúng định dạng .gz!"; fi
            else echo "Lỗi: Không tìm thấy file!"; fi
            pause ;;
            
        15)
            read -p "Nhập tên file cần XÓA: " file
            if [ -f "$file" ]; then
                read -p "Bạn có chắc chắn muốn xóa '$file'? (y/n): " confirm
                if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
                    rm "$file" && echo "Đã xóa file."
                else echo "Đã hủy thao tác xóa."; fi
            else echo "Lỗi: Không tìm thấy file!"; fi
            pause ;;
            
        16)
            read -p "Nhập tên thư mục cần XÓA: " dir
            if [ -d "$dir" ]; then
                read -p "Xóa thư mục sẽ mất hết dữ liệu bên trong! Chắc chắn? (y/n): " confirm
                if [ "$confirm" = "y" ] || [ "$confirm" = "Y" ]; then
                    rm -rf "$dir" && echo "Đã xóa thư mục."
                else echo "Đã hủy thao tác xóa."; fi
            else echo "Lỗi: Thư mục không tồn tại!"; fi
            pause ;;
            
        17)
            read -p "Nhập tên file cần đặt thuộc tính chỉ đọc: " file
            if [ -f "$file" ]; then
                chmod 444 "$file" && echo "Đã đổi file '$file' thành CHỈ ĐỌC (Mọi người dùng chỉ có quyền Read)."
            else echo "Lỗi: Không tìm thấy file!"; fi
            pause ;;
            
        18)
            read -p "Nhập tên file/thư mục cần Ẩn/Hiện: " item
            if [ -e "$item" ]; then
                # Kiểm tra xem tên có bắt đầu bằng dấu chấm chưa
                if [[ "$item" == .* ]]; then
                    # Nếu đang ẩn -> Hiện (bỏ dấu chấm)
                    new_name="${item#.}"
                    mv "$item" "$new_name" && echo "Đã HIỆN đối tượng thành '$new_name'."
                else
                    # Nếu đang hiện -> Ẩn (thêm dấu chấm)
                    mv "$item" ".$item" && echo "Đã ẨN đối tượng thành '.$item'."
                fi
            else echo "Lỗi: Đối tượng không tồn tại!"; fi
            pause ;;
            
        19)
            read -p "Nhập từ khóa tên file cần tìm: " keyword
            if [ -z "$keyword" ]; then echo "Lỗi: Từ khóa trống!";
            else
                echo "Kết quả tìm kiếm trong thư mục hiện tại và thư mục con:"
                print_sep
                find . -name "*$keyword*" 2>/dev/null
                print_sep
            fi
            pause ;;
            
        20)
            echo "Cảm ơn bạn đã sử dụng chương trình. Tạm biệt!"
            exit 0 ;;
            
        *)
            echo "Lỗi: Lựa chọn không hợp lệ (Phải từ 1-20)!"
            pause ;;
    esac
done
