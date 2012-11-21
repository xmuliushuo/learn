#include <iostream>

void e_2_11()
{
	int base;
	int exponent;
	std::cout << "input the base" << std::endl;
	std::cin >> base;
	std::cout << "input the exponent" << std::endl;
	std::cin >> exponent;
	int result = 1;
	for (int i = 0; i < exponent; i++) {
		result *= base;
	}
	std::cout << "the result is " << result << std::endl;
	return;
}
