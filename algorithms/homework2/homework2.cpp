
#include <iostream>

using namespace std;

const int Num = 10;
int A[Num];
int count = 0;
int partition(int start, int end);
int partition2(int start, int end);
int partition3(int start, int end);
void qsort(int, int);
int main()
{
    for (int i = 0; i < Num; i++) {
        cin >> A[i];
    }
    qsort(0, Num - 1);
    cout << count << endl;
    return 0;
}

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void qsort(int start, int end)
{
    if (start == end) {
        return;
    }
    count += end - start;
    int index = partition2(start, end);
    if ((index - 1) > start)
        qsort(start, index - 1);
    if ((index + 1) < end)
        qsort(index + 1, end);
}

int partition(int start, int end)
{
    if (start >= end) {
        return start;
    }
    int p = A[start];
    int i = start + 1;
    for (int j = start + 1; j <= end; j++) {
        if (A[j] < p) {
            swap(A + j, A + i);
            i++;
        }
    }
    swap (A + start, A + i - 1);
    return i - 1;
}

int partition2(int start, int end)
{
    if (start >= end) {
        return start;
    }
    int p = A[end];
    swap(A + start, A + end);
    int i = start + 1;
    for (int j = start + 1; j <= end; j++) {
        if (A[j] < p) {
            swap(A + j, A + i);
            i++;
        }
    }
    swap (A + start, A + i - 1);
    return i - 1;
}

int partition3(int start, int end)
{
    if (start >= end) {
        return start;
    }
    int a = A[start];
    int b = A[(start + end) / 2];
    int c = A[end];
    int p;
    if ((a > b) && (a > c)) {
        if (b > c) {
            p = A[(start + end) / 2];
            swap(A + start, A + (start + end) / 2);
        }
        else {
            p = A[end];
            swap(A + start, A + end);
        }
    }
    else if ((b > a) && (b > c)) {
        if (a > c) {
            p = A[start];
        }
        else {
            p = A[end];
            swap(A + start, A + end);
        }
    }
    else if ((c > a) && (c > b)) {
        if (b > a) {
            p = A[(start + end) / 2];
            swap(A + start, A + (start + end) / 2);
        }
        else {
            p = A[start];
        }
    }
    int i = start + 1;
    for (int j = start + 1; j <= end; j++) {
        if (A[j] < p) {
            swap(A + j, A + i);
            i++;
        }
    }
    swap (A + start, A + i - 1);
    return i - 1;
}
