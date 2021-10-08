#include "syscall.h"

void PrintArray(int a[], int size) {
    int i = 0;
    for (; i < size; ++i) {
        PrintNum(a[i]);
        PrintChar(' ');
    }
    PrintChar('\n');
}

int Less(int a, int b) {
    return a < b;
}

int Greater(int a, int b) {
    return a > b;
}

int Swap(int *a, int *b) {
    int t;
    t = *a;
    *a = *b;
    *b = t;
}

int Sort(int a[], int size, int (*sort_fn)(int, int)) {
    int i, j;
    for (j = size; j > 0; --j) {
        for (i = 0; i < j - 1; ++i) {
            if (sort_fn(a[i], a[i + 1]) == 0) {
                Swap(&a[i], &a[i + 1]);
            }
        }
    }
}

int main() {
    const int MAX_SIZE = 20;
    const char* MAX_SIZE_STRING = "20";

    int arraySize;
    int a[MAX_SIZE];
    int i;
    
    char sortDirection;
    int (*sortFunction)(int, int);
    char buffer[2];

    // Get array size
    PrintString("Enter array size: ");
    arraySize = ReadNum();
    if (arraySize > MAX_SIZE) {
        PrintString("Array size cannot be greater than ");
        PrintString(MAX_SIZE_STRING);
        PrintString("\n. Aborting.\n");
        Halt();
    }

    // Get sort direction
    PrintString("Sort by (ascending: <, descending: >): ");
    ReadString(buffer, 2);
    sortDirection = buffer[0];
    if (sortDirection == '<') {
        sortFunction = Less;
    } else if (sortDirection == '>') {
        sortFunction = Greater;
    } else {
        PrintString("Invalid sort direction. Aborting.\n");
        Halt();
    }

    // Get array elements
    for (i = 0; i < arraySize; ++i) {
        PrintString("Enter element: ");
        a[i] = ReadNum();
    }

    // Sort and display result
    PrintString("You've entered: ");
    PrintArray(a, arraySize);
    Sort(a, arraySize, sortFunction);
    PrintString("Sorted: ");
    PrintArray(a, arraySize);
}
