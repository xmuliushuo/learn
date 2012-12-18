/*
 * textquery.cpp
 *
 *  Created on: 2012-12-14
 *      Author: liushuo
 */

#include "textquery.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>

using namespace std;

void TextQuery::store_file(ifstream &is)
{
	string line;
	while(getline(is, line)) {
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

set<TextQuery::line_no> TextQuery::run_query(const std::string &query_word) const
{
	map<string, set<line_no> >::const_iterator loc = word_map.find(query_word);
	if (loc == word_map.end())
		return set<line_no>();
	else
		return loc->second;
}

string TextQuery::text_line(TextQuery::line_no line) const
{
	if (line < lines_of_text.size())
		return lines_of_text[line];
	throw out_of_range("line number out of range");
}
