#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "medea.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string detail;
};

MEDEA_DEFINE( phones &it, (it.country, it.phonelist, it.detail) );

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

    bool b = true;
    char c = 'y';
    int i = 10;
    unsigned u = ~0;
    float f = 11.f;
    double d = 3.14159;
    const char *cstr = "hello \"world\"";
    std::string string = "hello \"world\"";

    std::vector<char> buf(12,'$');
    std::vector<std::string> bufs(3); bufs[0]="a", bufs[1]="ab", bufs[2]="abc";
    std::map<int,std::string> map; map[0]="a", map[1]="ab", map[2]="abc";
    typedef std::vector<std::map<int,std::string>> purestl;
    purestl list(2); list[0]=map; list[1]=map;

#   define TESTR(a) [&]() { \
        std::cout << medea::to_json(a) << std::endl; \
    }()

#   define TESTW1(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(0); \
        std::istringstream json( medea::to_json(copy) ); \
        if( !medea::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
            if( newcopy != copy ) { \
                std::cout <<"[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
            } \
        } \
    }()
#   define TESTW2(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(0); \
        assert( newcopy != copy ); \
        std::istringstream json( medea::to_json(copy) ); \
        if( !medea::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
            if( newcopy != copy ) { \
                std::cout <<"[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
            } \
        } \
    }()
#   define TESTW3(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(); \
        assert( newcopy != copy ); \
        std::istringstream json( medea::to_json(copy) ); \
        if( !medea::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
		  if( newcopy != copy ) { \
              std::cout << "[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
            } \
        } \
    }()

#   define TESTRW1(a) [&](){ \
        TESTR(a); \
        TESTW1(a); \
    }()

#   define TESTRW2(a) [&](){ \
        TESTR(a); \
        TESTW2(a); \
    }()
#   define TESTRW3(a) [&](){ \
        TESTR(a); \
        TESTW3(a); \
    }()

    TESTR( cstr );

    TESTRW1( nullptr );
    TESTRW3( b );
    TESTRW3( c );
    TESTRW3( i );
    TESTRW3( u );
    TESTRW3( f );
    TESTRW3( d );
    TESTRW3( string );
    TESTRW3( buf );
    TESTRW3( bufs );
    TESTRW3( map );
    TESTRW3( list );

    std::cout << std::string(15, '-') << std::endl;

    {
        typedef unsigned idx;
        typedef std::pair<std::string,std::vector<int>> /*country,phonelist*/ phones;
        std::map< idx, phones > map;

        map[ 34 ].first = std::string("spain");
        map[ 34 ].second.push_back(687936564);
        map[ 34 ].second.push_back(687936565);
        map[ 30 ].first = std::string("greece");
        map[ 30 ].second.push_back(123);
        map[ 30 ].second.push_back(456);

        auto saved = map;

            std::cout << "original)" << std::endl;
            std::string json = medea::to_json( map );
            std::cout << json << std::endl;

            std::cout << "copy)" << std::endl;
            map.clear();
            medea::from_json( map, json );
            std::cout << medea::to_json( map ) << std::endl;

        assert( map == saved );

        // ensure void jsondoc does cleaning
        assert( medea::from_json(saved, "{}" ) );
        assert( saved.size() == 0 );

        std::cout << std::string(15, '-') << std::endl;
    }


    // high level
    {
        typedef unsigned idx;
        typedef std::pair<std::string,std::vector<int>> /*country,phonelist*/ phones;
        std::map< idx, phones > map;

        map[ 34 ].first = "spain";
        map[ 34 ].second.push_back(687936564);
        map[ 34 ].second.push_back(687936565);
        map[ 30 ].first = "greece";
        map[ 30 ].second.push_back(123);
        map[ 30 ].second.push_back(456);

        std::cout << "original)" << std::endl;
        medea::print( map );

        std::cout << "save-then-clear)" << std::endl;
        medea::save( map );
        medea::clear( map );
        medea::print( map );

        std::cout << "load)" << std::endl;
        medea::load( map );
        medea::print( map );

        std::cout << std::string(15, '-') << std::endl;
    }

    std::cout << medea::specs<medea::MEDEA>::encode("687936564") << std::endl;

  #if 1
    {
        // user defined types
        std::unordered_map<int,phones> p;
        p[0].country = "+\"34\"";
        p[0].phonelist.push_back( 123456 );
        medea::print( p );
        medea::save( p );
        medea::clear( p );
        medea::print( p );
        medea::load( p );
        medea::print( p );
    }
    #endif

    std::cout << "All ok." << std::endl;

    return 0;
}
