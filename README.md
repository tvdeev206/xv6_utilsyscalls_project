# Đồ Án 1: Xv6 Utilities & System Calls

Đồ án môn **Hệ điều hành** — Tìm hiểu, xây dựng các tiện ích người dùng (User Utilities), cải tiến Shell và hiện thực các lời gọi hệ thống (System Calls) trên hệ điều hành **xv6-riscv**.

---
## Quy định làm bài

### File readme trong từng branch
- Tự viết hướng giải cụ thể để có thể dễ dàng đối chiếu lại.
- Nêu các thay đổi ở từng file (nếu có thay đổi).
- Thêm các hình minh họa (nếu có thì dùng mermaid).
- Thêm cách chạy lại chương trình.
- Tổng hợp lại kết quả kiểm thử và hạn chế thành 1 bảng:
  - Lệnh kiểm thử
  - Kết quả
  - Lỗi (nếu gặp)
- **Lưu ý**: nên viết từng phần theo mục ứng với phần mình làm trong file report trong prism để lúc sau thêm vào dễ hơn.

### File report chung
- Gồm đường dẫn đến Prism project.
- Report sẽ làm chung trong Prism, khi nào xong thì sẽ check lại và viết lại vào prism.
- Nếu có hình minh họa thì sẽ để vào trong file image (Prism).

### Chia việc
- **Phần 1**: gồm memdunp, find + exec và cải tiến shell (Tổng điểm 5.5)
- **Phần 2**: gồm sandbox a command, attack xv6 và bonus (Tổng điểm 6.5)
---

## Cài đặt & Khởi chạy

### 1. Tải mã nguồn xv6
```bash
git clone git://g.csail.mit.edu/xv6-labs-2026
cd xv6-labs-2026
```

### 2. Biên dịch và chạy xv6 trên QEMU
```bash
make qemu
```
- **Các phím tắt hữu ích trong QEMU:**
  - `Ctrl-p`: In thông tin về các tiến trình (processes) đang hoạt động.
  - `Ctrl-a x`: Thoát khỏi trình giả lập QEMU.
  - `make qemu-fs`: Khởi động lại QEMU giữ nguyên hệ thống file (`fs.img`) từ lần chạy trước.

---

## Nội dung thực hiện

Đồ án được chia thành 2 phần chính làm việc trên 2 nhánh (`branch`) riêng biệt của kho mã nguồn.

### Phần 4.1: Lab Utilities (Nhánh `util`)

Chuyển sang nhánh `util` trước khi thực hiện:
```bash
git checkout util
make clean
```

#### 1. `memdump` (`user/memdump.c`)
Hiện thực hàm `memdump(char *fmt, char *data, int len)` để in nội dung của `len` byte hợp lệ trong vùng nhớ `data` theo chuỗi định dạng `fmt`:
- `i`: In 4 byte tiếp theo dưới dạng số nguyên 32-bit (hệ thập phân).
- `p`: In 8 byte tiếp theo dưới dạng số nguyên 64-bit (hệ thập lục phân - hex).
- `h`: In 2 byte tiếp theo dưới dạng số nguyên 16-bit (hệ thập phân).
- `c`: In 1 byte tiếp theo dưới dạng ký tự ASCII 8-bit.
- `s`: 8 byte tiếp theo chứa con trỏ 64-bit trỏ tới một chuỗi C; in chuỗi đó.
- `S`: Phần còn lại của dữ liệu chứa chuỗi C kết thúc bằng null (hoặc đến hết `len` byte hợp lệ); in chuỗi đó.
- **Kiểm tra giới hạn (Bounds check):** Không đọc vượt quá `len` byte. Nếu thiếu dữ liệu cho ký tự định dạng `X`, in thông báo `memdump: not enough data for 'X'` và dừng lại.

**Kiểm tra:**
```bash
# Trong xv6 shell:
$ memdump
$ echo deadc0de | memdump hhcccc
$ echo deadc0de | memdump p
$ echo a | memdump i

# Từ thư mục gốc bên ngoài QEMU:
$ ./grade-lab-util memdump
```

#### 2. `find` + `-exec` (`user/find.c`)
- **Part A – `find`:** Viết tiện ích tìm kiếm đệ quy tất cả các file và thư mục khớp với tên cho trước trong một cây thư mục (sử dụng `open`, `read`, `fstat`; không đệ quy vào `.` và `..`).
- **Part B – `-exec`:** Bổ sung tùy chọn `-exec cmd` để thực thi lệnh `cmd <file>` đối với mỗi kết quả tìm được bằng cách kết hợp các system call `fork`, `exec`, và `wait` (hỗ trợ tối đa `MAXARG` tham số từ `kernel/param.h`).

**Kiểm tra:**
```bash
# Trong xv6 shell:
$ find . b
$ find . wc -exec echo hi
$ sh < findtest.sh

# Từ thư mục gốc bên ngoài QEMU:
$ ./grade-lab-util find
$ ./grade-lab-util exec
```

#### 3. Cải tiến Shell (`user/sh.c`)
Bổ sung 3 tính năng nâng cao cho chương trình shell mặc định của xv6:
- **3a. Ẩn dấu nhắc `$` khi xử lý lệnh từ file:** Chỉ in prompt `$ ` khi đầu vào chuẩn (`stdin`) là thiết bị console, giúp đầu ra khi chạy `sh < findtest.sh` không bị lặp ký tự `$`.
- **3b. Tự động hoàn thành bằng phím Tab (Tab Completion):**
  - Từ đầu tiên của dòng lệnh: đối chiếu với tên các chương trình trong thư mục gốc `/`.
  - Các từ tiếp theo: đối chiếu với tên file trong thư mục hiện tại.
  - Tự động điền nếu có duy nhất 1 tên khớp; liệt kê danh sách các tên khớp trên dòng mới nếu có nhiều kết quả.
- **3c. Lịch sử câu lệnh (Command History):**
  - Lưu trữ tối thiểu 10 câu lệnh gần nhất.
  - Hỗ trợ lệnh nội tại (built-in) `history` để in danh sách lệnh kèm số thứ tự (bắt đầu từ 1).
  - Hỗ trợ `!!` để chạy lại lệnh gần nhất và `!n` để chạy lại lệnh thứ `n` (in lệnh ra màn hình trước khi thực thi).
  - Cho phép điều hướng qua lịch sử lệnh bằng phím mũi tên **Lên / Xuống**.

---

### Phần 4.2: Lab System Calls (Nhánh `syscall`)

Chuyển sang nhánh `syscall` trước khi thực hiện:
```bash
git fetch
git checkout syscall
make clean
```

#### 1. Sandbox một câu lệnh (`interpose` System Call)
- **Part A – Giới hạn System Call bằng Bitmask:**
  - Khai báo và hiện thực system call `interpose(int mask, char *path)` (cập nhật `user/user.h`, `user/usys.pl`, `kernel/syscall.h`, `kernel/syscall.c`, `kernel/sysproc.c`, `kernel/proc.h`, `kernel/proc.c`).
  - Lưu cấu hình `mask` vào cấu trúc tiến trình `struct proc` để chặn các system call tương ứng với các bit được bật trong `mask`.
  - Đảm bảo `mask` được sao chép từ tiến trình cha sang tiến trình con trong `kfork()`.
- **Part B – Sandbox với đường dẫn cho phép (Allowed Pathnames):**
  - Mở rộng `sys_interpose()` để lưu lại đường dẫn được phép (`pathname`, kích thước `MAXPATH`) thông qua `argstr`.
  - Nếu `open` hoặc `exec` nằm trong `mask` bị chặn nhưng tham số đường dẫn trùng khớp với `pathname` cho phép, kernel vẫn cho phép thực thi lời gọi đó.
  - Kế thừa cả `mask` và `pathname` cho phép khi gọi `fork`.

**Kiểm tra:**
```bash
# Trong xv6 shell:
$ sandbox 32768 - cat README
$ sandbox 32768 README grep xv6 README
$ sandbox 32768 README grep xv6 x

# Từ thư mục gốc bên ngoài QEMU:
$ ./grade-lab-syscall sandbox_mask
$ ./grade-lab-syscall sandbox
```

#### 2. Kiểm thử cô lập bộ nhớ (`user/attack.c`)
- **Bối cảnh:** Trong nhánh lab này, lệnh xóa trắng trang nhớ `memset(mem, 0, sz)` trong `uvmalloc()` (`kernel/vm.c`) và các lệnh xóa trang giải phóng trong `kernel/kalloc.c` đã được lược bỏ (bởi cờ biên dịch `#ifndef LAB_SYSCALL`). Do đó, các trang bộ nhớ mới cấp phát qua `sbrk()` vẫn lưu giữ dữ liệu từ tiến trình đã giải phóng trước đó.
- **Yêu cầu:** Viết chương trình người dùng `user/attack.c` cấp phát bộ nhớ và tìm chuỗi dữ liệu bí mật đã được chương trình `user/secret.c` ghi vào bộ nhớ ở lần chạy trước, sau đó in chuỗi đó ra một dòng riêng biệt (không chỉnh sửa mã nguồn kernel hay `user/secret.c` khi nộp bài).

**Kiểm tra:**
```bash
# Trong xv6 shell:
$ secret xyzzy
$ attack

# Từ thư mục gốc bên ngoài QEMU:
$ ./grade-lab-syscall attack
```

#### 3. Phần mở rộng (Bonus) – Phân tích và vá lỗi trong xv6
- Tìm kiếm và phân tích một lỗi tồn tại trong mã nguồn gốc của xv6 ảnh hưởng đến tính cô lập giữa các tiến trình (process isolation) hoặc gây dừng kernel (panic/hang).
- Trình bày trong báo cáo (`Report.pdf`) và minh họa trực tiếp khi vấn đáp:
  1. Vị trí lỗi (file/hàm) và nguyên nhân gốc rễ.
  2. Chương trình kiểm chứng ở tầng user-space (ví dụ `user/exploit.c`) sử dụng system call.
  3. Hệ quả đối với hệ thống.
  4. Giải pháp khắc phục (bản vá lỗi) và kiểm tra lại với `make grade`.

---

## Thang điểm & Đánh giá

**Công thức tính điểm mỗi bài:**
$$\text{Điểm bài} = 60\% \times \text{Điểm chấm bài (Code, Test, Report)} + 40\% \times \text{Điểm vấn đáp \& Demo}$$

| STT | Nội dung | Điểm tối đa | Tự chấm |
| :-: | :--- | :---: | :---: |
| 1 | `memdump` | 1.5 | |
| 2 | `find` + `exec` | 2.5 | |
| 3 | Cải tiến Shell (`sh`) | 1.5 | |
| 4 | Sandbox a command (`interpose`) | 2.5 | |
| 5 | Kiểm thử cô lập bộ nhớ (`attack xv6`) | 2.0 | |
| 6 | Bonus (Phân tích & vá lỗi xv6) | 2.0 | |
| **Tổng** | | **12.0** | |

Để chạy toàn bộ bộ kiểm thử tự động trên từng nhánh:
```bash
make grade
```

---

## Quy cách đóng gói & Nộp bài

Nộp bài trực tiếp trên **Moodle** dưới định dạng file nén `.zip` đặt tên theo mã số sinh viên của các thành viên: `MSSV1_MSSV2_MSSV3.zip`.

### Cách tạo file `.patch` cho từng nhánh
```bash
# Trên branch util (Phần 4.1):
git checkout util
git diff origin/util > util.patch

# Trên branch syscall (Phần 4.2):
git checkout syscall
git diff origin/syscall > syscall.patch
```

### Cấu trúc thư mục nộp bài
```text
MSSV1_MSSV2_MSSV3.zip
├── Report.pdf
└── Release/
    ├── util.patch        # Kết quả từ: git diff origin/util
    └── syscall.patch     # Kết quả từ: git diff origin/syscall
```

> **Yêu cầu đối với `Report.pdf`:**
> Trình bày ngắn gọn, không sao chép toàn bộ mã nguồn. Với mỗi bài cần nêu rõ:
> 1. **Người thực hiện:** Thành viên phụ trách và tỉ lệ đóng góp.
> 2. **Hướng giải:** Tóm tắt ý tưởng chính.
> 3. **Thay đổi mấu chốt:** Các file/hàm đã chỉnh sửa hoặc thêm mới và lý do.
> 4. **Luồng hoạt động:** Sơ đồ khối (flowchart) hoặc minh họa luồng thực thi (User Program $\rightarrow$ Syscall $\rightarrow$ Kernel $\rightarrow$ Kết quả).
> 5. **Kiểm thử & Hạn chế:** Cách kiểm thử, kết quả đạt được và các phần chưa hoàn thiện (nếu có).
> 6. **Bảng tự chấm điểm** ở cuối báo cáo.

---

## Tài liệu tham khảo

- **MIT 6.828 / 6.1810 Lab Utilities:** [https://pdos.csail.mit.edu/6.828/2026/labs/util.html](https://pdos.csail.mit.edu/6.828/2026/labs/util.html)
- **MIT 6.828 / 6.1810 Lab System Calls:** [https://pdos.csail.mit.edu/6.828/2026/labs/syscall.html](https://pdos.csail.mit.edu/6.828/2026/labs/syscall.html)
- **Sách xv6 RISC-V (xv6 book):** [https://mit-pdos.github.io/xv6-riscv-book/](https://mit-pdos.github.io/xv6-riscv-book/)
