/*
 * e_1_18.cpp
 *
 *  Created on: 2012-10-24
 *      Author: liushuo
 */

#include <iostream>

void e_1_18()
{
	int low, high;
	std::cout << "input two numbers" << std::endl;
	std::cin >> low >> high;
	for (int i = low; i <= high; i++) {
		std::cout << i << " ";
	}
	std::cout << std::endl;
}
