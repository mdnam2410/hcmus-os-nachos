#include "syscall.h"
#include "copyright.h"
#define SO_VOI_NUOC 2

void main()
{
    // Khai bao
    int f_Success;             // Bien co dung de kiem tra thanh cong
    int si_voinuoc, si_result; // Bien id cho file
    char c_readFile;           // Bien ki tu luu ki tu doc tu file
    int vn[SO_VOI_NUOC];       // n voi nuoc
    int v;                     // Dung tich nuoc cua sinh vien
    int flag_done_result;      // Bien co luu dau hieu doc xong file result
    int index_min;
    int i;

    //-----------------------------------------------------------

    
    for(i = 0; i< SO_VOI_NUOC; ++i){
        vn[i]=0;
    }
    // Xu ly voi nuoc
    // WHILE(11111111111111111111111111111111111111)
    while (1)
    {
        Wait("m_vn");

        // Mo file result.txt de ghi voi nao su dung
        si_result = Open("result.txt", 0);
        if (si_result == -1)
        { //?
            Signal("sinhvien");
            return;
        }
        // WHILE(2222222222222222222222222222222222222222)
        while (1)
        {
            Wait("voinuoc");
            c_readFile = 0;
            // Mo file voi nuoc .txt de doc dung tich
            si_voinuoc = Open("voinuoc.txt", 1);
            if (si_voinuoc == -1)
            {
                //?
                Close(si_result);
                Signal("sinhvien");

                return;
            }

            v = 0;
            flag_done_result = 0;
            // WHILE(3333333333333333333333333333333333333333333333)
            while (1)
            {
                if (Read(&c_readFile, 1, si_voinuoc) == -2) // EOF in file
                {
                    Close(si_voinuoc);
                    break;
                }
                if (c_readFile != '*') // EOF user define
                {
                    v = v * 10 + (c_readFile - 48); 
                }
                else
                {
                    flag_done_result = 1;
                    Close(si_voinuoc);
                    break;
                }
            }
            // WHILE(3333333333333333333333333333333333333333333333)
            if (v != 0)
            {
                // find min in vector vn and add it
                index_min = 0;
                for(i = 1; i< SO_VOI_NUOC; ++i){
                    if(vn[index_min] > vn[i]){
                        index_min = i;
                    }
                }
                vn[index_min]+=v;
                // Tranfs index min +1 to char
                // Mượn c_readFile để lưu char sau khi convert
                c_readFile = index_min+48+1;
                Write(&c_readFile, 1, si_result);
            }

            if (flag_done_result == 1)
            {
                // make vector v to zero
                for(i = 0; i< SO_VOI_NUOC; ++i){
                    vn[i]=0;
                }
                Close(si_result);
                Signal("sinhvien");
                break;
            }

            Signal("sinhvien");
        }
        // WHILE(2222222222222222222222222222222222222222)
    }
    // WHILE(11111111111111111111111111111111111111)
}
