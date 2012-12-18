/*
 * person.h
 *
 *  Created on: 2012-12-18
 *      Author: liushuo
 */

#ifndef PERSON_H_
#define PERSON_H_

#include <string>

class Person {
public:
	Person(std::string &name, std::string &address):m_name(name), m_address(address)
	{

	}
	std::string get_name() const
	{
		return m_name;
	}
	std::string get_address() const
	{
		return m_address;
	}
private:
	std::string m_name;
	std::string m_address;
};


#endif /* PERSON_H_ */
