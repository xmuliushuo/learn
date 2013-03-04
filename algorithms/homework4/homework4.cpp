#include <iostream>
#include <fstream>
#include <stack>
#include <stdlib.h>
using namespace std;

const int MaxVertex = 875714;
const int MaxArc = 5105043;

int visited[MaxVertex + 1];
int finished[MaxVertex + 1];

int count = 0;
int temp = 0;

stack<int> s;

typedef struct arc {
	int tailvex, headvex;
	struct arc *hlink, *tlink;
}Arc;

typedef struct node {
	Arc *firstin, *firstout;
}Node;

typedef struct {
	Node vlist[MaxVertex + 1];
	int vexnum, arcnum;
}Graph;

void initGraph(Graph &G)
{
	ifstream infile("SCC.txt");
	for (int i = 1; i <= G.vexnum; ++i) {
		G.vlist[i].firstin = NULL;
		G.vlist[i].firstout = NULL;
	}
	int v1, v2;
	Arc *p;
	for (int i = 0; i < G.arcnum; ++i) {
		//cin >> v1 >> v2;
		infile >> v1 >> v2;
		//cout << v1 << " " << v2 << endl;
		p = (Arc *)malloc(sizeof(Arc));
		if (p == NULL) {
			cout << "memory" << endl;
			exit(-1);
		}
		p->tailvex = v2;
		p->headvex = v1;
		p->hlink = G.vlist[v1].firstout;
		p->tlink = G.vlist[v2].firstin;
		G.vlist[v1].firstout = p;
		G.vlist[v2].firstin = p;
	}
}

void DFS(const Graph &G, int v)
{
	//cout << "DFS " << v << endl;
	visited[v] = 1;
	Arc *p = G.vlist[v].firstout;
	while (p != NULL) {
		if (!visited[p->tailvex]) {
			DFS(G, p->tailvex);
		}
		p = p->hlink;
	}
	finished[++count] = v;
}
// void DFS(const Graph &G, int v)
// {
// 	s.push(v);
// 	while (!s.empty()) {
// 		visited[s.top()] = 1;
// 		Arc *p = G.vlist[s.top()].firstout;
// 		while (p != NULL) {
// 			if (!visited[p->tailvex]) {
// 				visited[p->tailvex] = 1;
// 				s.push
// 			}
// 		}
// 	}
// }

void TDFS(const Graph &G, int v)
{
	visited[v] = 1;
	temp++;
	Arc *p = G.vlist[v].firstin;
	while (p != NULL) {
		if (!visited[p->headvex]) {
			TDFS(G, p->headvex);
		}
		p = p->tlink;
	}
}

int main()
{
	Graph G;
	G.vexnum = MaxVertex;
	G.arcnum = MaxArc;
	initGraph(G);
	
	for (int i = 1; i <= MaxVertex; ++i) {
		visited[i] = 0;
	}

	for (int i = 1; i <= MaxVertex; i++) {
		if (!visited[i]) DFS(G, i);
	}
	cout << "DFS finished" << endl;
	for (int i = 1; i <= MaxVertex; ++i) {
		visited[i] = 0;
	}

	for (int i = MaxVertex; i >= 1; i--) {
		if (!visited[finished[i]]){
			temp = 0;
			TDFS(G, finished[i]);
			if (temp > 200) {
				cout << temp << endl;
			}
		}
	}

	// for (int i = 1; i <= count; i++) {
	// 	cout << finished[i] << endl;
	// }

	return 0;
}