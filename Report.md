# Báo Cáo Đồ Án 1: Xv6 Utilities & System Calls

## Thông tin nhóm

| STT | Họ và Tên | MSSV | Tỉ lệ đóng góp (%) |
| :-: | :--- | :---: | :---: |
| 1 | *(Điền tên thành viên 1)* | `MSSV1` | 100% |
| 2 | *(Điền tên thành viên 2)* | `MSSV2` | 100% |
| 3 | *(Điền tên thành viên 3)* | `MSSV3` | 100% |

---

## Phần 4.1: Lab Utilities (Branch `util`)

### Bài 1: `memdump`

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Hướng giải:**
  - Duyệt qua từng ký tự trong chuỗi định dạng `fmt`, đồng thời theo dõi con trỏ vị trí hiện tại và số byte còn lại (`len`).
  - Kiểm tra điều kiện biên trước khi đọc từng kiểu dữ liệu (`i`: 4 byte, `p`: 8 byte, `h`: 2 byte, `c`: 1 byte, `s`: 8 byte, `S`: phần còn lại cho tới ký tự `\0` hoặc hết `len` byte).
- **Thay đổi mấu chốt:**
  - `user/memdump.c`: Hiện thực hàm `memdump(char *fmt, char *data, int len)`.
- **Luồng hoạt động:**
  ```mermaid
  flowchart LR
    A["Input: fmt, data, len"] --> B{"Còn ký tự trong fmt?"}
    B -- Không --> C["Kết thúc"]
    B -- Có --> D{"Đủ số byte trong len?"}
    D -- Không --> E["In lỗi: not enough data for 'X'"] --> C
    D -- Có --> F["Ép kiểu & in theo định dạng (i/p/h/c/s/S)"]
    F --> G["Cập nhật con trỏ data & giảm len"] --> B
  ```
- **Kiểm thử & Hạn chế:**
  - Chạy `memdump`, `echo deadc0de | memdump hhcccc`, `echo a | memdump i` trong xv6 và `./grade-lab-util memdump`.
  - Kết quả: *(Điền kết quả kiểm thử)*

---

### Bài 2: `find` + `-exec`

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Hướng giải:**
  - **Part A (`find`):** Sử dụng `open`, `fstat` để kiểm tra loại file (`T_FILE` hoặc `T_DIR`). Với thư mục, đọc các mục `struct dirent` và gọi đệ quy cho các mục khác `.` và `..`.
  - **Part B (`-exec`):** Khi phát hiện tham số `-exec`, lưu danh sách đối số của lệnh cần chạy. Mỗi khi tìm thấy file khớp tên, dùng `fork()` tạo tiến trình con để gọi `exec()`, và tiến trình cha gọi `wait(0)`.
- **Thay đổi mấu chốt:**
  - `user/find.c`: Thêm mới chương trình `find` hỗ trợ đệ quy và tùy chọn `-exec`.
  - `Makefile`: Thêm `$U/_find` vào danh sách `UPROGS`.
- **Luồng hoạt động:**
  ```mermaid
  flowchart TD
    A["find(path, target, exec_argv)"] --> B["open(path) & fstat(fd, &st)"]
    B --> C{"st.type == T_DIR?"}
    C -- Không --> D{"Tên khớp target?"}
    C -- Có --> E["Đọc từng dirent (bỏ qua '.' và '..')"]
    E --> F["Gọi đệ quy find(subpath, target, exec_argv)"]
    D -- Có --> G{"Có cờ -exec?"}
    G -- Không --> H["printf('%s\\n', path)"]
    G -- Có --> I["fork() -> child: exec(cmd, argv) / parent: wait(0)"]
  ```
- **Kiểm thử & Hạn chế:**
  - Chạy `sh < findtest.sh`, `./grade-lab-util find`, `./grade-lab-util exec`.
  - Kết quả: *(Điền kết quả kiểm thử)*

---

### Bài 3: Cải tiến Shell (`sh`)

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Hướng giải:**
  - **3a (Ẩn `$` khi đọc từ file):** Kiểm tra loại file descriptor `0` bằng `fstat(0, &st)` (chỉ in `$ ` nếu `st.type == T_DEVICE`).
  - **3b (Tab completion):** Xử lý ký tự `\t` để tìm các mục khớp tiền tố trong `/` (từ đầu tiên) hoặc `.` (các từ tiếp theo).
  - **3c (Lịch sử lệnh):** Lưu mảng vòng các câu lệnh đã nhập, xử lý lệnh nội tại `history`, `!!`, `!n` và mã thoát ANSI cho phím mũi tên Lên/Xuống.
- **Thay đổi mấu chốt:**
  - `user/sh.c` (và `kernel/console.c` nếu có chỉnh sửa chế độ đọc phím): Bổ sung kiểm tra thiết bị console, hoàn thành từ bằng Tab và quản lý lịch sử lệnh.
- **Luồng hoạt động:**
  - *(Mô tả sơ đồ xử lý chuỗi lệnh và phím bấm)*
- **Kiểm thử & Hạn chế:**
  - Kiểm thử trực tiếp trên QEMU với `sh < findtest.sh`, phím `Tab`, lệnh `history`, `!!`, `!n` và phím mũi tên.
  - Kết quả: *(Điền kết quả kiểm thử)*

---

## Phần 4.2: Lab System Calls (Branch `syscall`)

### Bài 1: Sandbox a command (`interpose`)

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Hướng giải:**
  - **Part A:** Thêm trường `sandbox_mask` vào `struct proc`. Khi `sys_interpose()` được gọi, lưu `mask` vào tiến trình hiện tại và sao chép sang tiến trình con trong `kfork()`. Tại `syscall()`, kiểm tra bit tương ứng với số hiệu syscall.
  - **Part B:** Thêm mảng `sandbox_path[MAXPATH]` vào `struct proc`. Nếu syscall là `SYS_open` hoặc `SYS_exec` và bị đánh dấu trong `mask`, đọc chuỗi đường dẫn đối số đầu tiên để so sánh với `sandbox_path`.
- **Thay đổi mấu chốt:**
  - `Makefile`: Thêm `$U/_sandbox` và `$U/_sbtest` vào `UPROGS`.
  - `user/user.h`, `user/usys.pl`, `kernel/syscall.h`: Khai báo stub và mã số `SYS_interpose`.
  - `kernel/proc.h`, `kernel/proc.c`: Lưu và kế thừa `mask` cùng đường dẫn được phép qua `kfork()`.
  - `kernel/sysproc.c`, `kernel/syscall.c`: Hiện thực `sys_interpose()` và kiểm tra quyền thực thi trong `syscall()`.
- **Luồng hoạt động:**
  ```mermaid
  sequenceDiagram
    participant U as User Process (sandbox)
    participant S as syscall() Dispatcher
    participant K as Kernel Syscall Handler
    U->>S: ecall (SYS_interpose, mask, path)
    S->>K: sys_interpose() lưu mask & path vào struct proc
    U->>S: ecall (SYS_open / SYS_exec / ...)
    alt Syscall không bị mask hoặc khớp allowed path
      S->>K: Thực thi hàm sys_* tương ứng
      K-->>U: Trả kết quả
    else Syscall bị chặn bởi mask
      S-->>U: Trả về -1 (Từ chối thực thi)
    end
  ```
- **Kiểm thử & Hạn chế:**
  - Chạy `./grade-lab-syscall sandbox_mask` và `./grade-lab-syscall sandbox`.
  - Kết quả: *(Điền kết quả kiểm thử)*

---

### Bài 2: Kiểm thử cô lập bộ nhớ (`attack xv6`)

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Hướng giải:**
  - Do `uvmalloc()` và `kalloc()` không xóa dữ liệu cũ trên trang vật lý khi biên dịch với `LAB_SYSCALL`, chương trình `user/attack.c` gọi `sbrk()` để cấp phát các trang nhớ vừa được `user/secret.c` giải phóng và quét tìm chuỗi dữ liệu tại vị trí offset tương ứng.
- **Thay đổi mấu chốt:**
  - `user/attack.c`: Cấp phát bộ nhớ động qua `sbrk()` và đọc chuỗi dữ liệu còn lưu lại trên trang nhớ.
- **Luồng hoạt động:**
  ```mermaid
  flowchart LR
    A["secret.c cấp phát trang & ghi chuỗi"] --> B["secret.c kết thúc -> kfree() trả trang vào freelist"]
    B --> C["attack.c gọi sbrk() nhận lại trang nhớ cũ"]
    C --> D["Đọc và in chuỗi dữ liệu từ trang nhớ"]
  ```
- **Kiểm thử & Hạn chế:**
  - Chạy `secret xyzzy` tiếp theo là `attack` trong xv6 shell; chạy `./grade-lab-syscall attack`.
  - Kết quả: *(Điền kết quả kiểm thử)*

---

### Bài 3: Bonus – Phân tích & khắc phục lỗi trong xv6

- **Người thực hiện:** *(Tên thành viên & tỉ lệ đóng góp)*
- **Mô tả lỗi:**
  - File / Hàm: *(Điền tên file và hàm)*
  - Nguyên nhân: *(Mô tả nguyên nhân)*
- **Chương trình kiểm chứng (`user/exploit.c`):**
  - *(Mô tả cách chương trình gọi system call để tái hiện lỗi)*
- **Hệ quả:**
  - *(Mô tả ảnh hưởng đến tiến trình hoặc kernel)*
- **Cách khắc phục:**
  - *(Mô tả bản vá lỗi và kết quả kiểm tra lại với `make grade`)*

---

## Bảng tự chấm điểm

| STT | Bài | Điểm tối đa | Tự chấm | Ghi chú |
| :-: | :--- | :---: | :---: | :--- |
| 1 | `memdump` | 1.5 | | |
| 2 | `find` + `exec` | 2.5 | | |
| 3 | Cải tiến Shell | 1.5 | | |
| 4 | Sandbox a command | 2.5 | | |
| 5 | Kiểm thử cô lập bộ nhớ (`attack xv6`) | 2.0 | | |
| 6 | Bonus | 2.0 | | |
| **Tổng** | | **12.0** | | |
