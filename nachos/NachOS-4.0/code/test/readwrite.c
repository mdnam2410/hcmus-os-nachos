#include "syscall.h"
#define MAX_LENGTH 255

int main()
{

    int input;
    int output;
    char buffer[MAX_LENGTH];
    int len;
    // PrintString("\n\t\t\t-----ECHO TRONG NACHOS-----\n\n");
    // PrintString(" - input file: ");

    // Goi ham Open de mo file input
    input = Open("test.txt", 1);
    if (input != -1)
    {
        // Goi ham Read de doc noi dung nhap vao input
        // Bay gio len vua co the la do dai, vua co the la ket qua (thanh cong/that bai) cua ham Read()
        len = Read(buffer, MAX_LENGTH, input);

        if (len != -1 && len != -2) // Kiem tra co bi loi, hay co EOF hay khong
        {
            output = Open("testclone.txt", 0); // Goi ham Open voi type = 3 de su dung output
            if (output != -1)
            {
                // PrintString(" -> file output: ");
                Write(buffer, len, output); // Goi ham Write de ghi noi dung doc duoc vao output
                Close(output);              // Goi ham Close de dong output
            }
        }
        Close(input); // Goi ham Close de dong input
    }
    return 0;
}
