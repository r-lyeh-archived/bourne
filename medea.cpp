#include "medea.hpp"

namespace medea
{
#if 1
    // json draft
    const bool pair_iskv = true;
    const char separator = ',';
    const char separator_kv = ':';
    const char open_seq = '[';
    const char open_umap = '{';
    const char open_smap = '{';
    const char open_rope = '\"';
    const char close_seq = ']';
    const char close_umap = '}';
    const char close_smap = '}';
    const char close_rope = '\"';
    // to_quote, from_quote
#else
    // medea draft
    const bool pair_iskv = true;
    const char separator = ',';
    const char separator_kv = ':';
    const char open_seq   = '[';
    const char open_umap  = '{';
    const char open_smap  = '<';
    const char open_rope = '\"';
    const char close_seq  = ']';
    const char close_umap = '}';
    const char close_smap = '>';
    const char close_rope = '\"';
#endif
}   // medea::

#if 0

// Thiemo Mättig

struct patch {
    unsigned begin = 0, end = 0;
    std::string append;

    patch()
    {}

    patch( const std::string &append_, unsigned b, unsigned e ) : append(append_), begin(b), end(e)
    {}

    std::string apply( const std::string &text ) const {
        return text.substr( 0, begin ) + append + text.substr( begin + end );
    }
};

MEDEA_DEFINE( patch &it, (it.begin, it.end, it.append) );

//compare these two strings
patch diff(std::string a, std::string b) {
  unsigned c = 0,                 //start from the first character
    d = a.size(), e = b.size();
  for (;        //and from the last characters of both strings
    a[c] &&                            //if not at the end of the string and
    a[c] == b[c];                      //if both strings are equal at this position
    c++);                              //go forward
  for (; d > c & e > c &               //stop at the position found by the first loop
    a[d - 1] == b[e - 1];              //if both strings are equal at this position
    d--) e--;                          //go backward
   //return position and lengths of the two substrings found
    if( e - c ) {
    std::cout << a.substr( 0, d - c ) << std::endl;
    std::cout << '-' << std::endl;
    std::cout << a.substr( c, d - c ) << std::endl;
    std::cout << '+' << std::endl;
    std::cout << b.substr( c, e - c ) << std::endl;
    }
  return patch( b.substr( c, e - c ), c, d - c );
}

int main() {

    main2();

    std::string from = "hello world";
    std::string to = "helloes worldes";

    std::cout << medea::to_json( diff(from, to) ) << std::endl;
    std::cout << medea::to_json( diff(from, to).apply( from ) ) << std::endl;

    return 0;
}

#endif
