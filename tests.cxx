#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "bourne.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string detail;
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.detail) );

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
        std::cout << bourne::to_json(a) << std::endl; \
    }()

#   define TESTW1(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(0); \
        std::istringstream json( bourne::to_json(copy) ); \
        if( !bourne::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
            if( newcopy != copy ) { \
                std::cout <<"[FAIL] " << bourne::to_json(newcopy) << " vs " << bourne::to_json(copy) << " failed!" << std::endl; \
            } \
        } \
    }()
#   define TESTW2(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(0); \
        assert( newcopy != copy ); \
        std::istringstream json( bourne::to_json(copy) ); \
        if( !bourne::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
            if( newcopy != copy ) { \
                std::cout <<"[FAIL] " << bourne::to_json(newcopy) << " vs " << bourne::to_json(copy) << " failed!" << std::endl; \
            } \
        } \
    }()
#   define TESTW3(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(); \
        assert( newcopy != copy ); \
        std::istringstream json( bourne::to_json(copy) ); \
        if( !bourne::from_json(newcopy,json) ) { \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
        } else { \
		  if( newcopy != copy ) { \
              std::cout << "[FAIL] " << bourne::to_json(newcopy) << " vs " << bourne::to_json(copy) << " failed!" << std::endl; \
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
            std::string json = bourne::to_json( map );
            std::cout << json << std::endl;

            std::cout << "copy)" << std::endl;
            map.clear();
            bourne::from_json( map, json );
            std::cout << bourne::to_json( map ) << std::endl;

        assert( map == saved );

        // ensure void jsondoc does cleaning
        assert( bourne::from_json(saved, "{}" ) );
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
        bourne::print( map );

        std::cout << "save-then-clear)" << std::endl;
        bourne::save( map );
        bourne::clear( map );
        bourne::print( map );

        std::cout << "load)" << std::endl;
        bourne::load( map );
        bourne::print( map );

        std::cout << std::string(15, '-') << std::endl;
    }

    std::cout << bourne::specs<bourne::JSON>::encode("687936564") << std::endl;

  #if 1
    {
        // user defined types
        std::unordered_map<int,phones> p;
        p[0].country = "+\"34\"";
        p[0].phonelist.push_back( 123456 );
        bourne::print( p );
        bourne::save( p );
        bourne::clear( p );
        bourne::print( p );
        bourne::load( p );
        bourne::print( p );
    }
    #endif

    std::cout << "All ok." << std::endl;

    return 0;
}
