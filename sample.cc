#include "bourne.hpp"

#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

int main() {
    // first-class objects are automatically serialized
    std::unordered_map< std::string, std::vector< std::string > > contacts = {
        { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
        { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
        { "lisa",   {"marge", "homer",  "bart", "maggie" } },
        { "bart",   {"marge",  "lisa", "homer", "maggie" } },
        { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
    }, built;
    std::cout << bourne::to_json( contacts ) << std::endl;

    // saving/loading showcase
    std::string json = bourne::to_json( contacts );
    bourne::from_json( built, json );
    assert( built == contacts );

    // custom objects require a thin BOURNE_DEFINE() wrapper
    phones list = { "uk", {123,456}, "just a few contacts" };
    std::cout << bourne::to_json( list ) << std::endl;
}
