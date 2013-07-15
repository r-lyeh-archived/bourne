#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "medea.hpp"


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
        if( !medea::from_json(newcopy,json) ) \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
            else \
        if( newcopy != copy ) \
            std::cout <<"[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
    }()
#   define TESTW2(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(0); \
        assert( newcopy != copy ); \
        std::istringstream json( medea::to_json(copy) ); \
        if( !medea::from_json(newcopy,json) ) \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
            else \
        if( newcopy != copy ) \
            std::cout <<"[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
    }()
#   define TESTW3(original) [&](){ \
        auto copy = original; auto newcopy = decltype(copy)(); \
        assert( newcopy != copy ); \
        std::istringstream json( medea::to_json(copy) ); \
        if( !medea::from_json(newcopy,json) ) \
            std::cout <<"[FAIL] cant convert from json" << std::endl; \
            else \
		if( newcopy != copy ) \
            std::cout << "[FAIL] " << medea::to_json(newcopy) << " vs " << medea::to_json(copy) << " failed!" << std::endl; \
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
    }

    // user def
    #if 0
    {
        struct phones {
            std::string country;
            std::vector<int> phonelist;

            void proxy() {
                medea::save( *this );
                *this = phones();
                medea::load( *this );
            }
        };
        typedef unsigned idx;
        std::map< idx, phones > map;

        map[ 34 ].country = "spain";
        map[ 34 ].phonelist.push_back(687936564);
        map[ 34 ].phonelist.push_back(687936565);
        map[ 30 ].country = "greece";
        map[ 30 ].phonelist.push_back(123);
        map[ 30 ].phonelist.push_back(456);

        std::cout << "original)" << std::endl;
        medea::print( map );

        std::cout << "save-then-clear)" << std::endl;
        medea::save( map );
        medea::clear( map );
        medea::print( map );

        std::cout << "load)" << std::endl;
        medea::load( map );
        medea::print( map );
    }
    #endif

    std::cout << "All ok." << std::endl;

    return 0;
}
