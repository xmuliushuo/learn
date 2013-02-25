
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <sstream>

using namespace std;

//#define DEBUG

int Randomi(int i, int j);
int RandomMincut();
void display();

int g[201][201]; // 图的邻接矩阵
int g2[201][201];
int m; // 图的边数
int v[201]; // 用于标记顶点是否被删除
int edgeCount = 0;
const int n = 201;

int main()
{
    int i, j;
    int v1, v2;
    srand(time(NULL));
    // 输入
    // TODO  input
    string line;
    int start, temp;
    for (i = 1; i < n; i++) {
		getline(cin, line);
		istringstream stream(line);
		stream >> start;
		while (stream >> temp) {
			g[start][temp] = 1;
		}
	}

    for (i = 1; i < n; i++) {
        g[i][i] = 0;
    }
    for (i = 1; i < n; i++) {
        for (j = i + 1; j < n; j++) {
            edgeCount += g[i][j];
        }
    }
    int min = RandomMincut();
    for (i = 0; i < 2 * n; i++) {
        int temp = RandomMincut();
        if (temp < min) min = temp;
    }
    printf("%d", min);
    return 0;
}

int Randomi(int i, int j)
{
    return i + rand() % (j - i + 1);
}

int RandomMincut()
{
    #ifdef DEBUG
    printf("RandomMincut()\n");
    #endif
    int i, j, t;
    int edgenum = edgeCount;
    int e;
    int v1, v2;
    int temp;
    for (i = 1; i < n; i++) {
        v[i] = 1;
    }

    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            g2[i][j] = g[i][j];
        }
    }
    for (t = 1; t < n - 2; t++) {
        #ifdef DEBUG
        display();
        printf("-----------------\n");
        #endif
        e = Randomi(1, edgenum);
        temp = 0;
        v1 = -1;
        for (i = 1; i < n; i++) {
            for (j = 1; j < i; j++) {
                temp += g2[i][j];
                if (temp >= e) {
                    v1 = i;
                    v2 = j;
                    break;
                }
            }
            if (v1 >= 0) {
                break;
            }
        }
        edgenum -= g2[v1][v2];
        v[v1] = 0;
        for (i = 1; i < n; i++) {
            g2[v2][i] += g2[v1][i];
            g2[i][v2] = g2[v2][i];
            g2[v1][i] = 0;
            g2[i][v1] = 0;
        }
        g2[v2][v2] = 0;
    }
    #ifdef DEBUG
    display();
    #endif
    for (i = 1; i < n; i++) {
        if (v[i] == 1) {
            v1 = i;
            break;
        }
    }
    for (i = n - 1; i > 0; i--) {
        if (v[i] == 1) {
            v2 = i;
            break;
        }
    }
    #ifdef DEBUG
    printf("%d %d\n", v1, v2);
    #endif
    return g2[v1][v2];
}

void display()
{
    int i, j;
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            printf("%d ", g2[i][j]);
        }
        printf("\n");
    }
}
