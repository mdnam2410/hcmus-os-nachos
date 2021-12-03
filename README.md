# hcmus-os-nachos

## Đồ án 1

### Chia việc các Syscall và Exception

1. Đặng Thái Duy: 3 Syscall ReadNum, PrintNum, RandomNum
2. Võ Văn Toàn: 2 Syscall ReadChar, PrintChar và Exception OverflowNumber
3. Mai Duy Nam: 2 Syscall ReadString, PrintString ; chương trình Help và Exception nhập ký tự thay vì số nguyên

## Đồ án 2

### Việc cần làm

// Duy

- System call nhập/xuất file
  - CreateFile
  - Open
  - Close
  - Read
  - Write
- Đa chương, lập lịch và đồng bộ hóa
  - Chỉnh sửa addrspace.h và addrspace.cc: Nam
  - Xây dựng lớp ptable (Nam), stable (Toàn), ...
    // Nam
  - Exec
  - Join
  - Exit
    // Toàn
  - CreateSemaphore
  - Wait
  - Signal
- Viết chương trình minh họa
  - Chương trình main
  - Chương trình sinh viên
  - Chương trình vòi nước

### Những điều cần tìm hiểu

- Nhập/xuất file:
  - Cài đặt bảng mô tả file cho mỗi tiến trình
  - Lưu ý trong bảng mô tả file, hai phần tử đầu dành cho console input và console output
  - Lưu ý khi Read và Write đối với console input và console output
- Đa chương, lập lịch và đồng bộ hóa
  - Chỉnh sửa addrspace.h và addrspace.cc như thế nào
  - Cài đặt semaphore

### Thắc mắc

- Trong syscall.h đã định nghĩa sẵn syscall Create. Liệu Create và CreateFile có phải là một?
- File progtest.cc nằm ở đâu?
- Phần viết chương trình minh họa:
  - Mỗi sinh viên viết vào file và mỗi vòi nước cũng viết vào file, vậy định dạng output như vậy là sao?
- Vẽ hình trong báo cáo? WTF?
