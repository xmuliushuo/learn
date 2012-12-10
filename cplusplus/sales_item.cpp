/*
 * sales_item.cpp
 *
 *  Created on: 2012-12-10
 *      Author: liushuo
 */

#include "sales_item.h"

std::istream& Sales_item::input(std::istream& in)
{
	double price;
	in >> isbn >> units_sold >> price;
	if (in) {
		revenue = units_sold * price;
	}
	else {
		units_sold = 0;
	}
	return in;
}


std::ostream& Sales_item::output(std::ostream& out) const
{
	out << "isbn: " << isbn << std::endl
		<< "revenue: " << revenue << std::endl;
	return out;
}
