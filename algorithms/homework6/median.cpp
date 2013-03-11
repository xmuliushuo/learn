#include <iostream>
#include <vector>
#include <algorithm>


using namespace std;

const int Num = 10000;
vector<int> minHeap;
int minHeapLen = 0;
vector<int> maxHeap;
int maxHeapLen = 0;

bool compare(int a, int b)
{
	if (a > b) return true;
	else return false;
}

int main()
{
	int temp;
	int median;
	int result = 0;
	cin >> median;
	result += median;
	for (int i = 1; i < Num; i++) {
		cin >> temp;
		if (temp < median) {
			maxHeap.push_back(temp);
			push_heap(maxHeap.begin(), maxHeap.end());
		}
		else {
			minHeap.push_back(temp);
			push_heap(minHeap.begin(), minHeap.end(), compare);
		}
		if (maxHeap.size() > minHeap.size()) {
			minHeap.push_back(median);
			push_heap(minHeap.begin(), minHeap.end(), compare);
			pop_heap(maxHeap.begin(), maxHeap.end());
			median = *(maxHeap.end() - 1);
			maxHeap.pop_back();
		}
		else if (minHeap.size() > (maxHeap.size() + 1)) {
			maxHeap.push_back(median);
			push_heap(maxHeap.begin(), maxHeap.end());
			pop_heap(minHeap.begin(), minHeap.end(), compare);
			median = *(minHeap.end() - 1);
			minHeap.pop_back();
		}
		cout << median << endl;
		result += median;
		result %= 10000;
	}
	cout << result << endl;
	return 0;
}