#include "Entry.hpp"

/* CONSTRUCTORS */
Entry::Entry()
	{}

Entry::Entry(Entry const& src)
	{ this->operator=(src); }

Entry::~Entry()
	{}

Entry& Entry::operator=(Entry const& rhs)
{
	_name = rhs._name;
	_type = rhs._type;
	return *this;
}

Entry::Entry(struct dirent* input)
	:_name(input->d_name, input->d_name + input->d_namlen), _type(input->d_type) {}
