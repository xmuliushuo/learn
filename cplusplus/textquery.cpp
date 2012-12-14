/*
 * textquery.cpp
 *
 *  Created on: 2012-12-14
 *      Author: liushuo
 */

#include "textquery.h"
#include <istream>

using std::ifstream;
using std::string;
using std::istringstream;
using std::vector;

void TextQuery::store_file(ifstream& in)
{
	string line;
	while(getline(in, line)) {
		lines_of_text.push_back(line);
	}
}

void TextQuery::build_map()
{
	istringstream iss;
	string temp;
	line_no count = 1;
	for (vector<string>::iterator iter = lines_of_text.begin();
			iter != lines_of_text.end(); iter++, count++) {
		iss.str(*iter);
		while (iss >> temp) {
			word_map[temp].insert(count);
		}
		iss.clear();
	}
}
