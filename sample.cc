#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "bourne.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

/* or...
namespace bourne {
    std::string to_json( const phones &it ) {
        auto p = std::make_tuple( it.country, it.phonelist, it.detail );
        std::string json = bourne::to_json( p );
        return json;
    }
    bool from_json( phones &it, std::istream &is ) {
        auto p = std::make_tuple( it.country, it.phonelist, it.detail );
        if( !bourne::from_json( p, is ) )
            return false;
        tie( it.country, it.phonelist, it.detail ) = p;
        return true;
    }
}
*/

int main() {

    // first-class classes are automatically serialized
    {
        std::map< std::string, std::vector< std::string > > contacts = {
            { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
            { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
            { "lisa",   {"marge", "homer",  "bart", "maggie" } },
            { "bart",   {"marge",  "lisa", "homer", "maggie" } },
            { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
        };
        std::string json = bourne::to_json( contacts );
        std::cout << json << std::endl;
    }

    // user defined classes require a thin bourne_DEFINE() wrapper
    {
        std::unordered_map<int,phones> list, copy;
        list[0].country = "+\"12\"";
        list[0].phonelist.push_back( 345678 );

        std::string json = bourne::to_json( list );
        bourne::from_json( copy, json );

        assert( copy.size() == list.size() );
        assert( copy[0].country == list[0].country );
        assert( copy[0].phonelist == list[0].phonelist );
        assert( copy[0].details == list[0].details );

        std::cout << json << std::endl;
    }

    std::cout << "All ok." << std::endl;

    return 0;
}
