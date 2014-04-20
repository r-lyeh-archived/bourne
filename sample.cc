#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "medea.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

MEDEA_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

/* or...
namespace medea {
    std::string to_json( const phones &it ) {
        auto p = std::make_tuple( it.country, it.phonelist, it.detail );
        std::string json = medea::to_json( p );
        return json;
    }
    bool from_json( phones &it, std::istream &is ) {
        auto p = std::make_tuple( it.country, it.phonelist, it.detail );
        if( !medea::from_json( p, is ) )
            return false;
        tie( it.country, it.phonelist, it.detail ) = p;
        return true;
    }
}
*/

int main() {

    // built-in types
    {
        std::map< std::string, std::vector< std::string > > contacts = {
            { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
            { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
            { "lisa",   {"marge", "homer",  "bart", "maggie" } },
            { "bart",   {"marge",  "lisa", "homer", "maggie" } },
            { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
        };
        std::string json = medea::to_json( contacts );
        std::cout << json << std::endl;
    }

    {
        // user defined types
        std::unordered_map<int,phones> p, copy;
        p[0].country = "+\"34\"";
        p[0].phonelist.push_back( 123456 );

        std::string json = medea::to_json( p );
        medea::from_json( copy, json );

        assert( copy.size() == p.size() );
        assert( copy[0].country == p[0].country );
        assert( copy[0].phonelist == p[0].phonelist );
        assert( copy[0].details == p[0].details );

        std::cout << json << std::endl;
    }

    std::cout << "All ok." << std::endl;

    return 0;
}
