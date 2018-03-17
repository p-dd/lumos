#include "DTO.h"	

inline void DTO::reloc ()
{
	/*if (pos > REL_LEN) {
		data_->erase (data_->begin (), data_->begin () + pos);
		pos = 0;
	}*/
}


DTO::DTO () : pos (sizeof (size_t)), data_ (new std::vector<unsigned char> (sizeof (size_t), 0)) {}

DTO::DTO (boost::shared_ptr <std::vector <unsigned char> > data) : pos (sizeof (size_t)), data_ (new std::vector<unsigned char> (sizeof (size_t), 0))
{
	data_->insert (data->end (), data->begin (), data->end ());
	*((size_t *)data_->data()) = data->size (); // &(*data_)[0]
};

DTO & DTO::operator << (const std::string & b) {
	(*this) << b.size ();
	data_->insert (data_->end (), (char *)&b[0], (char *)&b[0] + b.size ());
	*((size_t *)data_->data()) += b.size (); // &(*data_)[0]
	/*for (size_t i = 0; i < data_->size (); i++) printf ("%d ", (int)(*data_)[i]);
	printf ("\n");*/
	return (*this);
}


DTO & DTO::operator >> (std::string & b) {
	size_t n;
	(*this) >> n;
	//printf ("n: %d\n", n);
	//b.append (&(*data_)[pos], &(*data_)[pos + n]);
	//b.append (data_->begin() + pos, data_->begin() + pos + n);
	b = std::string(data_->begin() + pos, data_->begin() + pos + n);
	pos += n;
	//		cout << b;
	reloc ();
	return (*this);
}

size_t DTO::size () { return data_->size () - sizeof(size_t); }

void DTO::clear () 
{
	data_->clear ();
	pos = sizeof (size_t);
}

