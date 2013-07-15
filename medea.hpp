// Medea is a lightweight JSON serializer for most STL containers.
// - rlyeh ~~ listening to incredible hog / execution.

#pragma once
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace medea
{
    // utils
    namespace utils {
        std::string to_quote( const std::string &t );
        bool from_quote( std::string &t, std::istream &is );
        bool peeknext( std::istream &is, char ch, char ch2 = 0 );
        bool next( std::istream &is, char ch, char ch2 = 0 );
        bool next( std::istream &is );
        bool prev( std::istream &is );
    }

    // exports
    std::string to_json( const std::string  &t );

    template<size_t N>
    std::string to_json( const std::string::value_type (&t)[N] );
    std::string to_json( std::string::value_type * const &t );
    std::string to_json( const std::string::value_type *t );
    std::string to_json( const std::string::value_type &t );

    std::string to_json( const std::uint8_t &t );
    std::string to_json( const std::uint16_t &t );
    std::string to_json( const std::uint32_t &t );
    std::string to_json( const std::uint64_t &t );

    std::string to_json( const std::int8_t &t );
    std::string to_json( const std::int16_t &t );
    std::string to_json( const std::int32_t &t );
    std::string to_json( const std::int64_t &t );

    std::string to_json( const bool &t );
    std::string to_json( const float &t );
    std::string to_json( const double &t );
    std::string to_json( const std::nullptr_t &t );

    // both sequence and associative containers
    template<typename CONTAINER>
    std::string to_json( const CONTAINER &t ) {
        std::stringstream ss;
        for( const auto &it : t )
            ss << medea::to_json(it) << ',';
        const char in = '[', out = ']';
        std::string text = ss.str();
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out );
    }

    // pairs for associative containers
    template<typename K, typename V>
    std::string to_json( const std::pair<K,V> &t ) {
        std::stringstream ss;
        auto key = medea::to_json(t.first);
        auto val = medea::to_json(t.second);
        ss << '{' << key << ':' << val << '}';
        return ss.str();
    }

    // associative containers, different encoding in/out characters
#   define $expand(TYPE) \
    template<typename K, typename V> \
    std::string to_json( const TYPE <K,V> &t ) {  \
        std::stringstream ss; \
        for( const auto &kv : t ) { \
            auto key = medea::to_json(kv.first); \
            auto val = medea::to_json(kv.second); \
            ss << key << ':' << val << ','; \
        } \
        const char in = '{', out = '}'; \
        std::string text = ss.str(); \
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out ); \
    }
    $expand( std::map );
    $expand( std::multimap );
    $expand( std::unordered_map );
    $expand( std::unordered_multimap );
#   undef $expand

    // imports
    bool from_json( std::string::value_type &t, std::istream &json );
    bool from_json( std::string &t, std::istream &json );

    bool from_json( std::uint8_t &t, std::istream &json );
    bool from_json( std::uint16_t &t, std::istream &json );
    bool from_json( std::uint32_t &t, std::istream &json );
    bool from_json( std::uint64_t &t, std::istream &json );

    bool from_json( std::int8_t &t, std::istream &json );
    bool from_json( std::int16_t &t, std::istream &json );
    bool from_json( std::int32_t &t, std::istream &json );
    bool from_json( std::int64_t &t, std::istream &json );

    bool from_json( std::nullptr_t &t, std::istream &json );
    bool from_json( bool &t, std::istream &json );

    bool from_json( float &t, std::istream &json );
    bool from_json( double &t, std::istream &json );

#   define next(...) \
    medea::utils::next(__VA_ARGS__)
#   define prev(...) \
    medea::utils::prev(__VA_ARGS__)

    // pairs for associative containers
    template<typename K, typename V>
    bool from_json( std::pair<K,V> &t, std::istream &is ) {
        t = std::pair<K,V>();
        bool ok = true;
        ok = ok && next(is,'{');
        if(  ok && next(is,'}') ) return ok;
        ok = ok && prev(is);
        ok = ok && from_json(t.first, is);
        ok = ok && next(is,':');
        ok = ok && from_json(t.second, is);
        ok = ok && next(is,'}');
        return ok;
    }

    // both sequence and associative containers
    template<typename CONTAINER>
    bool from_json( CONTAINER &t, std::istream &is ) {
        using namespace medea::utils;
        typename CONTAINER::value_type val;
        t = CONTAINER();
        bool ok = true;
        ok = ok && next(is,'[');
        if(  ok && next(is,']') ) return ok;
        ok = ok && prev(is);
        do {
            ok = ok && from_json(val, is);
            ok = ok && (std::inserter(t,t.end()) = val, true);
        } while( ok && next(is,',') );
        ok = ok && prev(is);
        ok = ok && next(is,']');
        return ok;
    }

    // associative containers, different encoding in/out characters
#   define $expand(TYPE) \
    template<typename K, typename V> \
    bool from_json( TYPE <K,V> &t, std::istream &is ) { \
        using namespace medea::utils; \
        std::pair<K,V> val; \
        t = TYPE<K,V>(); \
        bool ok = true; \
        ok = ok && next(is,'{'); \
        if(  ok && next(is,'}') ) return ok; \
        ok = ok && prev(is); \
        do { \
            ok = ok && from_json(val.first, is); \
            ok = ok && next(is,':'); \
            ok = ok && from_json(val.second, is); \
            ok = ok && (std::inserter(t,t.end()) = val, true); \
        } while( ok && next(is,',') ); \
        ok = ok && prev(is); \
        ok = ok && next(is,'}'); \
        return ok; \
    }
    $expand( std::map )
    $expand( std::multimap )
    $expand( std::unordered_map )
    $expand( std::unordered_multimap )
#   undef  $expand

#   undef next
#   undef prev

    template<typename T>
    bool from_json( T &t, const std::string &s ) {
        std::stringstream ss( s );
        return from_json( t, ss );
    }

    // high level object saving/loading

    struct obj {
        template<typename T>
        static T &get(int idx) {
            typedef std::pair<std::thread::id,int> pair;
            static std::map<pair, T> all;
            pair p( std::this_thread::get_id(), idx );
            return all[ p ] = all[ p ];
        }
    };

    template<typename T>
    void save( const T& t ) {
        std::string json = medea::to_json( t );
        obj::get<std::string>((int)&t) = json;
    }
    template<typename T>
    bool load( T& t ) {
        std::string json = obj::get<std::string>((int)&t);
        return from_json( t, json );
    }
    template<typename T>
    void clear( T& t ) {
        t = T();
    }
    template<typename T>
    std::ostream &print( const T &t, std::ostream &os = std::cout ) {
        return os << medea::to_json(t) << std::endl, os;
    }
}

#if 0
namespace std {
    using medea::to_json;
    using from_json;
}
#endif
