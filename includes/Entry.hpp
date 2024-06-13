#ifndef ENTRY_H
# define ENTRY_H

# include <iostream>
# include <dirent.h>

using std::string;

class Entry
{
	string	_name;
	char	_type;

	Entry();

public:
	Entry(struct dirent* input);
	Entry(Entry const& src);
	Entry& operator=(Entry const& rhs);
	~Entry();
};

#endif

/* File types
 *
 * #define DT_UNKNOWN       0
 * #define DT_FIFO          1
 * #define DT_CHR           2
 * #define DT_DIR           4
 * #define DT_BLK           6
 * #define DT_REG           8
 * #define DT_LNK          10
 * #define DT_SOCK         12
 * #define DT_WHT          14
*/
