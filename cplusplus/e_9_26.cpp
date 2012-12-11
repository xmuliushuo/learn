/*
 * e_9_26.cpp
 *
 *  Created on: 2012-12-11
 *      Author: liushuo
 */
#include <vector>
#include <list>

using namespace std;

void e_9_26()
{
	int ia[] = { 0, 1, 1, 2, 3, 5, 8, 13, 21, 55, 89 };
	vector<int> ivec(ia, ia + 11);
	list<int> ilist(ia, ia + 11);

	for (vector<int>::iterator iter = ivec.begin(); iter != ivec.end(); iter++) {
		if (((*iter) % 2) != 0) {
			ivec.erase(iter);
			iter--;
		}
	}

	for (list<int>::iterator iter = ilist.begin(); iter != ilist.end(); iter++) {
		if (((*iter) % 2) != 0) {
			ilist.erase(iter);
			iter--;
		}
	}
}

