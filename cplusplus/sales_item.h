/*
 * sales_item.h
 *
 *  Created on: 2012-12-10
 *      Author: liushuo
 */

#ifndef SALES_ITEM_H_
#define SALES_ITEM_H_

#include <iostream>
#include <string>

class Sales_item {
public:
	std::ostream& output(std::ostream& out) const;
	std::istream& input(std::istream& in);
	Sales_item():units_sold(0), revenue(0.0)
	{ }
private:
	std::string isbn;
	unsigned units_sold;
	double revenue;

};


#endif /* SALES_ITEM_H_ */
