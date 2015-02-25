#include "bourne.hpp"

#include <cassert>
#include <iostream>

// @todo (taken from cereal) {
#include <array> 
#include <bitset> 
#include <chrono> 
#include <complex> 
#include <deque> 
#include <forward_list> 
#include <list> 
#include <map> 
#include <memory> 
#include <queue> 
#include <set> 
#include <stack> 
#include <string> 
#include <tuple> 
#include <unordered_map> 
#include <unordered_set> 
#include <utility> 
#include <vector> 
// }


#include <cassert>
#include <string.h>

unsigned fails = 0;
bool verbose = false;

#define BOURNE_TEST( ... ) do { \
            unsigned errors = 0; \
            std::cout << "[    ] " #__VA_ARGS__ << " ..."; \
            auto cpy = __VA_ARGS__; \
            auto clr = decltype(cpy) {}; \
            bool is_ok = bourne::from_json(clr, bourne::to_json(cpy) ); \
            auto json1 = bourne::to_json(cpy); \
            auto json2 = bourne::to_json(clr); \
            /* check serialization */ \
            if( is_ok && cpy == clr ) { \
                if( verbose ) { \
                    std::cout << " ( " << json1 << " )"; \
                } \
            } else { \
                errors++; \
                if( verbose ) { \
                    std::cout << " (error: " << json1 << " vs " << json2 << " )"; \
                } \
            } \
            /* check cleaning */ \
            bourne::from_json( cpy, "{}" ); \
            if( cpy != decltype(cpy) {} ) { \
                errors++; \
                std::cout << " (error: cleaning)"; \
            } \
            std::cout << (errors ? (fails++, "\r[FAIL]") : "\r[ OK ]") << std::endl; \
    } while( 0 )

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string detail;

    bool operator==( const phones &other ) const {
        return country == other.country && phonelist == other.phonelist && detail == other.detail;
    }
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.detail) );

int main( int argc, const char **argv )
{
    verbose = ( argc > 1 && ( 0 == strcmp( argv[1], "-v" ) || 0 == strcmp( argv[1], "--verbose" ) ) );

    // quick tests
    BOURNE_TEST( std::nullptr_t {} );
    BOURNE_TEST( bool {} );
    BOURNE_TEST( bool { false } );
    BOURNE_TEST( bool { true } );
    BOURNE_TEST( char {} );
    BOURNE_TEST( char { '\0' } );
    BOURNE_TEST( char { 'a' } );
    BOURNE_TEST( int {} );
    BOURNE_TEST( int { 0 } );
    BOURNE_TEST( int { 1 } );
    BOURNE_TEST( int { -1 } );
    BOURNE_TEST( float {} );
    BOURNE_TEST( float { 0.0 } );
    BOURNE_TEST( float { 1.0 } );
    BOURNE_TEST( float { -1.0 } );
    BOURNE_TEST( double {} );
    BOURNE_TEST( double { 0.0 } );
    BOURNE_TEST( double { 1.0 } );
    BOURNE_TEST( double { -1.0 } );
    BOURNE_TEST( size_t {} );
    BOURNE_TEST( size_t { 0 } );
    BOURNE_TEST( size_t { ~0u } );
    BOURNE_TEST( unsigned {} );
    BOURNE_TEST( unsigned { 0 } );
    BOURNE_TEST( unsigned { ~0u } );
    BOURNE_TEST( std::string {} );
    BOURNE_TEST( std::string { "abc" } );
    BOURNE_TEST( std::string { "hello \"world\"" } );
    BOURNE_TEST( std::vector<int> {} );
    BOURNE_TEST( std::vector<int> { 1,2,3 } );
    BOURNE_TEST( std::vector<std::string> {} );
    BOURNE_TEST( std::vector<std::string> { "a","b","c" } );
    BOURNE_TEST( std::list<int> {} );
    BOURNE_TEST( std::list<int> { 1,2,3 } );
    BOURNE_TEST( std::list<bool> {} );
    BOURNE_TEST( std::list<bool> { false,true,false } );
    BOURNE_TEST( std::list<std::string> {} );
    BOURNE_TEST( std::list<std::string> { "a","b","c" } );
    BOURNE_TEST( std::set<int> {} );
    BOURNE_TEST( std::set<int> { 1,2,3 } );
    BOURNE_TEST( std::set<bool> {} );
    BOURNE_TEST( std::set<bool> { true,false } );
    BOURNE_TEST( std::set<std::string> {} );
    BOURNE_TEST( std::set<std::string> { "a","b","c" } );
    BOURNE_TEST( std::map<int,std::string>{} );
    BOURNE_TEST( std::map<int,std::string>{ {0, "a"}, {-1, "b"}} );
    BOURNE_TEST( std::map<std::string,int>{} );
    BOURNE_TEST( std::map<std::string,int>{ {"a",0}, {"b", -1}} );
    BOURNE_TEST( std::unordered_map<int,std::string>{} );
    BOURNE_TEST( std::unordered_map<int,std::string>{ {0, "a"}, {-1, "b"}} );
    BOURNE_TEST( std::unordered_map<std::string,int>{} );
    BOURNE_TEST( std::unordered_map<std::string,int>{ {"a",0}, {"b", -1}} );

    BOURNE_TEST( std::list<int>{} );
    BOURNE_TEST( std::map<int,int>{} );
    BOURNE_TEST( std::set<int>{} );
    BOURNE_TEST( std::string{} );
    BOURNE_TEST( std::tuple<int>{} );
    BOURNE_TEST( std::unordered_map<int,int>{} );
    BOURNE_TEST( std::unordered_set<int>{} );
    BOURNE_TEST( std::vector<int>{} );

    BOURNE_TEST( std::vector<char> ( 12,'$' ) );
    BOURNE_TEST( std::vector<std::string> { "a", "ab", "abc" } );
    BOURNE_TEST( std::map<int,std::string> { {0,"a"}, {1, "ab"}, {2,"abc"} } );

    // nested
    BOURNE_TEST( std::vector<std::map<int,std::string>> { { {0,"a"},{1,"b"} }, { {1,"i"},{11,"ii"} } } );

    // tuples
    BOURNE_TEST( std::make_tuple(1,'a',1.2f,std::string("hel \"lo\" world")) );
    
    auto tuple = std::make_tuple(1,'a',1.2f,std::string("hel \"lo\" world"));
    bourne::from_json( tuple, "[-4,\"a\",\n1.2,\"hel \\\"omg\\\" lo\"]" );
    BOURNE_TEST( tuple );
   
    // custom objects
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

        BOURNE_TEST( map );
    }

    {
        std::unordered_map<int,phones> p;
        p[0].country = "+\"34\"";
        p[0].phonelist.push_back( 123456 );

        BOURNE_TEST( p );
    }
    
    std::cout << (!fails ? "All ok." : "Errors found.") << std::endl;

    return 0;
}

#if 0
    // @todo

    // basic<T>(std::istream &,T &)' declared but not defined
    //BOURNE_TEST( std::wstring {} );
    //BOURNE_TEST( std::wstring { L"abc" } );
    //BOURNE_TEST( std::wstring { L"hello \"world\"" } );
    //BOURNE_TEST( std::bitset<16>{} ); 
    //BOURNE_TEST( std::complex<int>{} );
    //BOURNE_TEST( std::deque<int>{} );
    //BOURNE_TEST( std::queue<int>{} );
    //BOURNE_TEST( std::stack<int>{} );

    // 'insert' : not a member
    //BOURNE_TEST( std::array<int,4>{} ); 
    //BOURNE_TEST( std::forward_list<int>{} ); // 'insert' : not a member

    //BOURNE_TEST( std::memory<>{} );
    //BOURNE_TEST( std::utility<>{} );

    //BOURNE_TEST( std::chrono<>{} );
#endif
