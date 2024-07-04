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
	name = rhs.name;
	type = rhs.type;
	return *this;
}

Entry::Entry(struct dirent* input)
	:name(input->d_name, input->d_name + input->d_namlen), type(input->d_type) {}

Entry::Entry(string n, char t) : name(n), type(t) {
}
