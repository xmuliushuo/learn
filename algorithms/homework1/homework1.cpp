#include <iostream>
#include <stdlib.h>

using namespace std;

const int MaxNum = 100000;
int num[MaxNum];
int numTemp[MaxNum];
void display();

void assert(bool condition)
{
    if (!condition) {
        cout << "error" << endl;
        exit(1);
    }
}

long long merge(int *array, int start, int middle, int end)
{
    assert(start <= end);
    int i = start;
    int j = middle + 1;
    int k = 0;
    long long count = 0;
    while (i <= middle || j <= end) {
        if ((array[i] <= array[j] && i <= middle) || j > end) {
            numTemp[k++] = array[i++];
        }
        else {
            numTemp[k++] = array[j++];
            count += middle - i + 1;
        }
    }
    for (i = start; i <= end; i++) {
        array[i] = numTemp[i - start];
    }
    //display();
    return count;
}

long long count(int *array, int start, int end)
{
    assert(start <= end);
    if (start == end) {
        return 0;
    }
    else {
        int middle = (start + end) / 2;
        long long a = count(array, start, middle);
        long long b = count(array, middle + 1, end);
        long long c = merge(array, start, middle, end);
        return a + b + c;
    }
}

void display()
{
    for (int i = 0; i < MaxNum; i++) {
        cout << num[i] << " ";
    }
    cout << endl;
}

int main()
{
    int i = 0;
    for (i = 0; i < MaxNum; i++) {
        cin >> num[i];
    }
    long long result = count(num, 0, MaxNum - 1);
    //display();
    cout << "result : " << result << endl;
    cin >> i;
    return 0;
}

