/*
 * e_1_17.cpp
 *
 *  Created on: 2012-10-23
 *      Author: liushuo
 */

#include <iostream>

void e_1_17()
{
	int num;
	int total = 0;
	while (std::cin >> num) {
		if (num < 0) {
			total++;
		}
	}
	std::cout << "the number of negative number is " << total << std::endl;
}


