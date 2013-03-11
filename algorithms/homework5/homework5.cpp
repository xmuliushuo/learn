#include <iostream>
#include <sstream>
#include <string>

using namespace std;

const int n = 201;
int g[n][n]; // 图的邻接矩阵
int D[n];
bool final[n];

int main()
{
	int i, j;
	int vetex, arc, start;
	char temp;
	string line;
	
	// initial
	for (i = 1; i < n; i++) {
		for (j = 1; j < n; j++) {
			g[i][j] = 1000000;
			if (i == j) g[i][j] = 0;
		}
		final[i] = false;
	}
	//input
	for (i = 1; i < n; i++) {
		getline(cin, line);
		istringstream stream(line);
		stream >> start;
		while (stream >> vetex >> temp >> arc) {
			g[start][vetex] = arc;
		}
	}

	for (i = 1; i < n; i++) {
		D[i] = g[1][i];
	}
	D[1] = 0;
	final[1] = true;
	int v;
	for (i = 2; i < n; i++) {
		int min = 1000000;
		for (j = 1; j < n; j++) {
			if (!final[j])
				if (D[j] < min) {
					v = j;
					min = D[v];
				}
		}
		final[v] = true;
		for (j = 0; j < n ; j++) {
			if ((!final[j]) && (min + g[v][j] < D[j])) {
				D[j] = min + g[v][j];
			}
		}
	}
	// for (i = 1; i < n ; i++) {
	// 	cout << i << ": " << D[i] << endl;
	// }
	 // 7,37,59,82,99,115,133,165,188,197
	cout << 7 << ": " << D[7] << endl;
	cout << 37 << ": " << D[37] << endl;
	cout << 59 << ": " << D[59] << endl;
	cout << 82 << ": " << D[82] << endl;
	cout << 99 << ": " << D[99] << endl;
	cout << 115 << ": " << D[115] << endl;
	cout << 133 << ": " << D[133] << endl;
	cout << 165 << ": " << D[165] << endl;
	cout << 188 << ": " << D[188] << endl;
	cout << 197 << ": " << D[197] << endl;
	return 0;
}