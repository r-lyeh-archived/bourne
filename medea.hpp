// Medea is a lightweight JSON serializer for most STL containers.
// - rlyeh ~~ listening to incredible hog / execution.

#pragma once
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace medea
{
    // constants
    extern const bool pair_iskv;
    extern const char separator;
    extern const char separator_kv;
    extern const char open_seq;
    extern const char open_umap;
    extern const char open_smap;
    extern const char open_rope;
    extern const char open_data;
    extern const char close_seq;
    extern const char close_umap;
    extern const char close_smap;
    extern const char close_rope;
    extern const char close_data;

    // utils
    namespace utils {
        std::string to_quote( const std::string &t );
        bool from_quote( std::string &t, std::istream &is );
        bool next( const char &ch, std::istream &is );
        bool prev( const char &ch, std::istream &is );
        bool next( const std::string &token, std::istream &is );
        bool prev( const std::string &token, std::istream &is );
        std::string encode( const std::string &binary ); // base91
        std::string decode( const std::string &text );   // base91
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

    typedef std::nullptr_t undefined;

    // both sequence and associative containers
    template<typename CONTAINER>
    static inline std::string to_json( const CONTAINER &t ) {
        std::stringstream ss;
        for( auto it = std::begin(t); it != std::end(t); ++it )
            ss << medea::to_json(*it) << separator;
        std::string text = ss.str();
        const char in = open_seq, out = close_seq;
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out );
    }

    // pairs for associative containers
    template<typename K, typename V>
    static inline std::string to_json( const std::pair<K,V> &t ) {
        std::stringstream ss;
        auto key = medea::to_json(t.first);
        auto val = medea::to_json(t.second);
        if( pair_iskv )
        ss << open_seq << key << separator_kv << val << close_seq;
        else
        ss << open_seq << val << separator_kv << key << close_seq;
        return ss.str();
    }

    // associative containers, different encoding in/out characters
#   define $expand(TYPE,open,close) \
    template<typename K, typename V> \
    static inline std::string to_json( const TYPE <K,V> &t ) {  \
        std::stringstream ss; \
        for( const auto &kv : t ) { \
            auto key = medea::to_json(kv.first); \
            auto val = medea::to_json(kv.second); \
            if( pair_iskv ) \
            ss << key << separator_kv << val << separator; \
            else \
            ss << val << separator_kv << key << separator; \
        } \
        std::string text = ss.str(); \
        const char in = open, out = close; \
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out ); \
    }
    $expand( std::map, open_smap, close_smap );
    $expand( std::multimap, open_smap, close_smap );
    $expand( std::unordered_map, open_umap, close_umap );
    $expand( std::unordered_multimap, open_umap, close_umap );
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

    // pairs for associative containers
    template<typename K, typename V>
    static inline bool from_json( std::pair<K,V> &t, std::istream &is ) {
        t = std::pair<K,V>();
        bool ok = true;
        ok = ok && medea::utils::next(open_seq,is);
        if(  ok && medea::utils::next(close_seq,is) ) return ok;
        ok = ok && medea::utils::prev(close_seq,is);
        if( pair_iskv ) {
        ok = ok && medea::from_json(t.first, is);
        ok = ok && medea::utils::next(separator_kv,is);
        ok = ok && medea::from_json(t.second, is);
        } else {
        ok = ok && medea::from_json(t.second, is);
        ok = ok && medea::utils::next(separator_kv,is);
        ok = ok && medea::from_json(t.first, is);
        }
        ok = ok && medea::utils::next(close_seq,is);
        return ok;
    }

    // both sequence and associative containers
    template<typename CONTAINER>
    static inline bool from_json( CONTAINER &t, std::istream &is ) {
        typename CONTAINER::value_type val;
        t = CONTAINER();
        bool ok = true;
        ok = ok && medea::utils::next(open_seq,is);
        if(  ok && medea::utils::next(close_seq,is) ) return ok;
        ok = ok && medea::utils::prev(close_seq,is);
        do {
            ok = ok && medea::from_json(val, is);
            ok = ok && (std::inserter(t,t.end()) = val, true);
        } while( ok && medea::utils::next(separator,is) );
        ok = ok && medea::utils::prev(separator,is);
        ok = ok && medea::utils::next(close_seq,is);
        return ok;
    }

    // associative containers, different encoding in/out characters
#   define $expand(TYPE,open,close) \
    template<typename K, typename V> \
    static inline bool from_json( TYPE <K,V> &t, std::istream &is ) { \
        using namespace medea::utils; \
        std::pair<K,V> val; \
        t = TYPE<K,V>(); \
        bool ok = true; \
        ok = ok && medea::utils::next(open,is); \
        if(  ok && medea::utils::next(close,is) ) return ok; \
        ok = ok && medea::utils::prev(close,is); \
        do { \
            if( pair_iskv ) { \
            ok = ok && medea::from_json(val.first, is); \
            ok = ok && medea::utils::next(separator_kv,is); \
            ok = ok && medea::from_json(val.second, is); \
            } else { \
            ok = ok && medea::from_json(val.second, is); \
            ok = ok && medea::utils::next(separator_kv,is); \
            ok = ok && medea::from_json(val.first, is); \
            } \
            ok = ok && (std::inserter(t,t.end()) = val, true); \
        } while( ok && medea::utils::next(separator,is) ); \
        ok = ok && medea::utils::prev(separator,is); \
        ok = ok && medea::utils::next(close,is); \
        return ok; \
    }
    $expand( std::map, open_smap, close_smap )
    $expand( std::multimap, open_smap, close_smap )
    $expand( std::unordered_map, open_umap, close_umap )
    $expand( std::unordered_multimap, open_umap, close_umap )
#   undef  $expand

    template<typename T>
    static inline bool from_json( T &t, const std::string &s ) {
        std::stringstream ss( s );
        return from_json( t, ss );
    }

    // tuples

    namespace {
        template<class TUPLE, std::size_t N>
        struct tuple {
            static std::string to_json( const TUPLE& t ) {
                return medea::tuple<TUPLE, N-1>::to_json(t) + separator + medea::to_json( std::get<N-1>(t) );
            }
            static bool from_json( TUPLE& t, std::istream &is ) {
                bool ok = true;
                ok = ok && medea::tuple<TUPLE, N-1>::from_json( t, is );
                ok = ok && medea::utils::next(separator,is);
                ok = ok && medea::from_json( std::get<N-1>(t), is );
                return ok;
            }
        };
        template<class TUPLE>
        struct tuple<TUPLE, 1> {
            static std::string to_json( const TUPLE& t ) {
                return medea::to_json( std::get<0>(t) );
            }
            static bool from_json( TUPLE& t, std::istream &is) {
                return medea::from_json( std::get<0>(t), is );
            }
        };
    }
    template<class... Args>
    static inline std::string to_json( const std::tuple<Args...>& t ) {
        std::stringstream ss;
        ss << open_seq;
        ss << medea::tuple<decltype(t), sizeof...(Args)>::to_json(t);
        ss << close_seq;
        return ss.str();
    }
    template<class... Args>
    static inline bool from_json( std::tuple<Args...> &t, std::istream &is ) {
        bool ok = true;
        ok = ok && medea::utils::next(open_seq,is);
        if(  ok && medea::utils::next(close_seq,is) ) return ok;
        ok = ok && medea::utils::prev(close_seq,is);
        ok = ok && medea::tuple<decltype(t), sizeof...(Args)>::from_json(t,is);
        ok = ok && medea::utils::next(close_seq,is);
        return ok;
    }

    // high level object saving/loading

    class archive {
        std::stringstream ss;
        const bool is_loading;
        const bool is_saving;
        //others...
        const bool is_json;

        explicit archive( bool L, bool S ) : is_loading(L), is_saving(S), is_json(true) {
        }

        template<typename T>
        archive &operator ,( T &t ) {
            if( is_loading ) ss << to_json(t);
            if( is_saving  ) from_json(t,ss);
            return *this;
        }
    };

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
    template<typename T>
    std::string diff( const T &t0, const T &t1 ) {
        // @todo
        return std::string();
    }
    template<typename T>
    bool patch( T &t, const std::string &diff ) {
        // @todo
        return false;
    }
}

// optional custom-class autoserialization macro
#define MEDEA_DEFINE( OBJECT, PARGS ) \
namespace medea { \
    std::string to_json( const OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::to_json( _tpl ); \
    } \
    bool from_json( OBJECT, std::istream &is ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::from_json( _tpl, is ) ? (std::tie PARGS = _tpl, true) : false;  \
    } \
}

#if 0
namespace std {
    using medea::to_json;
    using from_json;
}
#endif
