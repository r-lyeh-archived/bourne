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

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string detail;

    bool operator==( const phones &other ) const {
        return country == other.country && phonelist == other.phonelist && detail == other.detail;
    }
    bool operator!=( const phones &other ) const {
        return !operator==( other );
    }
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.detail) );

namespace tests {
    static bool verbose = false;
    static unsigned errors = 0;

    template<typename T>
    static void verify( const T &set, const char *c_str ) {
        std::cout << "[    ] " << c_str << " ...";
        auto cpy = set;
        auto clr = decltype(cpy) {};
        bool is_ok = bourne::from_json( clr, bourne::to_json(cpy) );
        auto json1 = bourne::to_json(cpy);
        auto json2 = bourne::to_json(clr);
        /* check serialization */
        if( is_ok && cpy == clr ) {
            if( verbose ) {
                std::cout << " ( " << json1 << " )";
            }
        } else {
            errors++;
            if( verbose ) {
                std::cout << " (error: " << json1 << " vs " << json2 << " )";
            }
        }
        /* check cleaning */
        bourne::from_json( cpy, "{}" );
        if( cpy != decltype(cpy) {} ) {
            errors++;
            std::cout << " (error: cleaning)";
        }
        std::cout << (errors ? "\r[FAIL]" : "\r[ OK ]") << std::endl;
    }
};

int main( int argc, const char **argv )
{
    using namespace tests;
#   define VERIFY( ... ) verify( __VA_ARGS__, #__VA_ARGS__ )

    // quick tests

    verbose = ( argc > 1 && ( 0 == strcmp( argv[1], "-v" ) || 0 == strcmp( argv[1], "--verbose" ) ) );

    VERIFY( std::nullptr_t {} );
    VERIFY( bool {} );
    VERIFY( bool { false } );
    VERIFY( bool { true } );
    VERIFY( char {} );
    VERIFY( char { '\0' } );
    VERIFY( char { 'a' } );
    VERIFY( int {} );
    VERIFY( int { 0 } );
    VERIFY( int { 1 } );
    VERIFY( int { -1 } );
    VERIFY( float {} );
    VERIFY( float { 0.0 } );
    VERIFY( float { 1.0 } );
    VERIFY( float { -1.0 } );
    VERIFY( double {} );
    VERIFY( double { 0.0 } );
    VERIFY( double { 1.0 } );
    VERIFY( double { -1.0 } );
    VERIFY( size_t {} );
    VERIFY( size_t { 0 } );
    VERIFY( size_t { ~0u } );
    VERIFY( unsigned {} );
    VERIFY( unsigned { 0 } );
    VERIFY( unsigned { ~0u } );
    VERIFY( std::string {} );
    VERIFY( std::string { "abc" } );
    VERIFY( std::string { "hello \"world\"" } );
    VERIFY( std::vector<int> {} );
    VERIFY( std::vector<int> { 1,2,3 } );
    VERIFY( std::vector<std::string> {} );
    VERIFY( std::vector<std::string> { "a","b","c" } );
    VERIFY( std::list<int> {} );
    VERIFY( std::list<int> { 1,2,3 } );
    VERIFY( std::list<bool> {} );
    VERIFY( std::list<bool> { false,true,false } );
    VERIFY( std::list<std::string> {} );
    VERIFY( std::list<std::string> { "a","b","c" } );
    //VERIFY( std::forward_list<int> {} );
    //VERIFY( std::forward_list<int> { 1,2,3 } );
    //VERIFY( std::forward_list<bool> {} );
    //VERIFY( std::forward_list<bool> { false,true,false } );
    //VERIFY( std::forward_list<std::string> {} );
    //VERIFY( std::forward_list<std::string> { "a","b","c" } );
    VERIFY( std::deque<int> {} );
    VERIFY( std::deque<int> { 1,2,3 } );
    VERIFY( std::deque<bool> {} );
    VERIFY( std::deque<bool> { false,true,false } );
    VERIFY( std::deque<std::string> {} );
    VERIFY( std::deque<std::string> { "a","b","c" } );
    VERIFY( std::set<int> {} );
    VERIFY( std::set<int> { 1,2,3 } );
    VERIFY( std::set<bool> {} );
    VERIFY( std::set<bool> { true,false } );
    VERIFY( std::set<std::string> {} );
    VERIFY( std::set<std::string> { "a","b","c" } );
    VERIFY( std::unordered_set<int> {} );
    VERIFY( std::unordered_set<int> { 1,2,3 } );
    VERIFY( std::unordered_set<bool> {} );
    VERIFY( std::unordered_set<bool> { true,false } );
    VERIFY( std::unordered_set<std::string> {} );
    VERIFY( std::unordered_set<std::string> { "a","b","c" } );
    VERIFY( std::map<int,std::string>{} );
    VERIFY( std::map<int,std::string>{ {0, "a"}, {-1, "b"}} );
    VERIFY( std::map<std::string,int>{} );
    VERIFY( std::map<std::string,int>{ {"a",0}, {"b", -1}} );
    VERIFY( std::unordered_map<int,std::string>{} );
    VERIFY( std::unordered_map<int,std::string>{ {0, "a"}, {-1, "b"}} );
    VERIFY( std::unordered_map<std::string,int>{} );
    VERIFY( std::unordered_map<std::string,int>{ {"a",0}, {"b", -1}} );

    VERIFY( std::tuple<int>{} );
    VERIFY( std::tuple<int>{1} );
    VERIFY( std::tuple<int,int,int>{} );
    VERIFY( std::tuple<int,int,int>{ 1,2,3 } );

    VERIFY( std::vector<char> ( 12,'$' ) );

    // nested
    VERIFY( std::vector<std::map<int,std::string>> { { {0,"a"},{1,"b"} }, { {1,"i"},{11,"ii"} } } );
    VERIFY( std::vector<std::tuple<int,int,int>>{ std::tuple<int,int,int>{0,1,2}, std::tuple<int,int,int>{3,4,5} } );
    VERIFY( std::vector<std::string> { "a", "ab", "abc" } );
    VERIFY( std::map<int,std::string> { {0,"a"}, {1, "ab"}, {2,"abc"} } );

    // tuples
    VERIFY( std::make_tuple(1,'a',1.2f,std::string("hel \"lo\" world")) );
    
    auto tuple = std::make_tuple(1,'a',1.2f,std::string("hel \"lo\" world"));
    bourne::from_json( tuple, "[-4,\"a\",\n1.2,\"hel \\\"omg\\\" lo\"]" );
    VERIFY( tuple );
   
    // custom objects
    {
        VERIFY( phones{} );
        VERIFY( phones{"spain",{687936564, 687936565},"bla bla"});
        VERIFY( std::unordered_map< unsigned, phones > { {34, phones{"spain",{123,456}}}, {30, phones{"greece",{789,789}}} } );
    }

    // @todo

    // SFINAE trait
    // VERIFY( std::wstring {} );
    // VERIFY( std::wstring { L"abc" } );
    // VERIFY( std::wstring { L"hello \"world\"" } );
    // VERIFY( std::bitset<16>{} ); 
    // VERIFY( std::complex<int>{} );
    // VERIFY( std::queue<int>{} );
    // VERIFY( std::stack<int>{} );

    // 'insert' : not a member
    // VERIFY( std::array<int,4>{} ); 
    // VERIFY( std::forward_list<int>{} );

    // VERIFY( std::memory<>{} );
    // VERIFY( std::utility<>{} );

    // VERIFY( std::chrono<>{} );
    
    std::cout << (errors ? "Errors found!" : "All ok.") << std::endl;
    return errors ? -1 : 0;
}
