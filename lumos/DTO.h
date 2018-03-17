//#include "declaration.h"

#ifndef DTO_H
#define DTO_H
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

//class talk_to_db;

class DTO { // Data Transfer Object
private:
	const int REL_LEN = 1024;
	boost::shared_ptr <std::vector <unsigned char> > data_;
	size_t pos;
	inline void reloc ();
public:
	DTO ();
	DTO (boost::shared_ptr <std::vector <unsigned char> > data);

	DTO & operator << (const std::string & b);

	template <class data_type>
	DTO & operator << (const data_type & b)
	{
		data_->insert (data_->end (), (char *)&b, (char *)&b + sizeof(b));
		*((size_t *)data_->data()) += sizeof (b); // &(*data_)[0]
		/*for (size_t i = 0; i < data_->size (); i++) printf ("%d ", (int)(*data_)[i]);
		printf ("\n");*/
		return (*this);
	}
	
	template <class data_type>
	DTO & operator >> (data_type & b)
	{
		unsigned char * a = (unsigned char *)&b;
		for (int i = 0; i < sizeof (b); i++) a[i] = (*data_)[pos + i];
		pos += sizeof (b);
		/*for (int i = 0; i < sizeof (b); i++) printf ("%d ", (int)a[i]);
		printf ("\n");*/
		reloc ();
		return (*this);
	}
	DTO & operator >> (std::string & b);

	size_t size ();
	void clear ();
	boost::shared_ptr <std::vector <unsigned char> > data () { return data_; }

};

#endif