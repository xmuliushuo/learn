/*
 * e_1_19.cpp
 *
 *  Created on: 2012-10-29
 *      Author: liushuo
 */

#include <iostream>

void e_1_19()
{
	int low, high;
	std::cout << "input two numbers" << std::endl;
	std::cin >> low >> high;
	int count = 1;
	for (int i = low; i <= high; i++) {
		std::cout << i << " ";
		if ((count % 10) == 0)
		{
			std::cout << std::endl;
		}
		count++;
	}
	std::cout << std::endl;
}


