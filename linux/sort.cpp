#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

std::vector<int> v;

int main()
{
	int a, b;
	string temp;
	int time;
	int n;
	cin >> n;
	int i;
	for (i = 0; i < n; ++i) {
		cin >> a >> b >> temp >> time;
		v.push_back(time);
	}
	sort(v.begin(), v.end());
	for (i = 0; i < n; ++i) {
		cout << v[i] << endl;
	}
	return 0;
}