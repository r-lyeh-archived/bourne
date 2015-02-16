/* Bourne is a lightweight JSON serializer written in C++11.
 * Copyright (c) 2013,2014 Mario 'rlyeh' Rodriguez. BOOST licensed

 * - rlyeh ~~ listening to Incredible Hog / Execution.
 */

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

namespace bourne
{
    enum spec {
        JSON
    };

    template <spec>
    struct traits {};

    template <>
    struct traits<JSON> {
        // json spec
        static const bool pair_iskv = true;
        static const char separator = ',',  separator_kv = ':';
        static const char open_seq  = '[',  close_seq    = ']';
        static const char open_umap = '{',  close_umap   = '}';
        static const char open_smap = '{',  close_smap   = '}';
        static const char open_rope = '\"', close_rope   = '\"';

        static std::string quote( const std::string &t ) {
            std::string out;
            for( auto &it : t ) {
                if( it != open_rope ) {
                    out += it;
                } else {
                    out += "\\\"";
                }
            }
            return std::string() + open_rope + out + close_rope;
        }

        static bool unquote( std::string &t, std::istream &is ) {
            struct scope {
                std::istream &is;
                unsigned counter = 0;
                explicit scope( std::istream &_is ) : is( _is ) {
                }
                ~scope() {
                    while( counter-- ) {
                        is.unget();
                    }
                }
                bool operator >>( char &ch ) {
                    return (!is.eof()) && (++counter) && (is >> ch);
                }
            } scoped_is( is );

            t = std::string();
            int status = 0;
            char ch;
            while( scoped_is >> ch ) {
                if( status == 0 ) {
                    if( ch == open_rope ) is >> std::noskipws, status++;
                    else return false;
                }
                else
                if( status == 1 ) {
                    if( ch == close_rope ) return scoped_is.counter = 0, is >> std::skipws, true;
                    if( ch != '\\' ) t += ch;
                    else status++;
                }
                else
                if( status == 2 ) {
                    if( ch == close_rope ) status--, t += ch;
                    else return false;
                }
            }
            return false;
        }
    };

    // other traits here...
    // [...]

    // specs, common
    template< spec SPEC >
    struct specs : public traits<SPEC> {
        static std::string encode( const std::string &binary ) {
            return binary;
        }

        static std::string decode( const std::string &text ) {
            return text;
        }

        static bool next( const std::string &token, std::istream &is ) {
            char ch;
            bool ok = true;
            for( auto &it : token ) {
                ok &= ( is >> ch ? ch == it : false );
            }
            return ok;
        }
        static bool prev( const std::string &token, std::istream &is ) {
            for( auto &it : token ) is.unget();
            return is.good();
        }
        static bool next( const char &ch, std::istream &is ) {
            return specs::next( std::string() + ch, is );
        }
        static bool prev( const char &ch, std::istream &is ) {
            return specs::prev( std::string() + ch, is );
        }
    };

    // exports

    //template<size_t N>
    //std::string to_json( const std::string::value_type (&t)[N] );

    template<class SPEC> std::string to( const std::string &t )                  { return SPEC::quote(t); }
    template<class SPEC> std::string to( const std::string::value_type *t )      { return to<SPEC>( std::string(t) ); }
    template<class SPEC> std::string to( const std::string::value_type &t )      { return to<SPEC>( std::string() + t ); }
    template<class SPEC> std::string to( std::string::value_type * const &t )    { return to<SPEC>( std::string(t) ); }

    template<class SPEC> std::string to( const std::uint64_t &t )     { return std::to_string(t); }
    template<class SPEC> std::string to( const std::uint32_t &t )     { return to<SPEC>( std::uint64_t(t) ); }
    template<class SPEC> std::string to( const std::uint16_t &t )     { return to<SPEC>( std::uint64_t(t) ); }
    template<class SPEC> std::string to( const std::uint8_t &t )      { return to<SPEC>( std::uint64_t(t) ); }

    template<class SPEC> std::string to( const std::int64_t &t )      { return std::to_string(t); }
    template<class SPEC> std::string to( const std::int32_t &t )      { return to<SPEC>( std::int64_t(t) ); }
    template<class SPEC> std::string to( const std::int16_t &t )      { return to<SPEC>( std::int64_t(t) ); }
    template<class SPEC> std::string to( const std::int8_t &t )       { return to<SPEC>( std::int64_t(t) ); }

    template<class SPEC> std::string to( const double &t )            { return std::to_string(t); }
    template<class SPEC> std::string to( const float &t )             { return to<SPEC>( double(t) ); }
    template<class SPEC> std::string to( const bool &t )              { return t ? "true" : "false"; }
    template<class SPEC> std::string to( const std::nullptr_t &t )    { return "null"; }

    // imports

    template<class SPEC> bool from( std::string &t, std::istream &json )             { return SPEC::unquote(t,json) ? true : false; }
    template<class SPEC> bool from( std::string::value_type &t, std::istream &json ) { std::string i; return from<SPEC>(i,json) ? t = i[0], true : false; }

    template<class SPEC> bool from( std::uint64_t &t, std::istream &json )     { return json >> t ? true : false; }
    template<class SPEC> bool from( std::int64_t  &t, std::istream &json )     { return json >> t ? true : false; }
    template<class SPEC> bool from( std::uint32_t &t, std::istream &json )     { std::uint64_t i; return from<SPEC>( i, json ) ? t = (std::uint32_t)(i), true : false; }
    template<class SPEC> bool from( std::int32_t  &t, std::istream &json )     { std::int64_t  i; return from<SPEC>( i, json ) ? t = (std::int32_t )(i), true : false; }
    template<class SPEC> bool from( std::uint16_t &t, std::istream &json )     { std::uint64_t i; return from<SPEC>( i, json ) ? t = (std::uint16_t)(i), true : false; }
    template<class SPEC> bool from( std::int16_t  &t, std::istream &json )     { std::int64_t  i; return from<SPEC>( i, json ) ? t = (std::int16_t )(i), true : false; }
    template<class SPEC> bool from( std::uint8_t  &t, std::istream &json )     { std::uint64_t i; return from<SPEC>( i, json ) ? t = (std::uint8_t )(i), true : false; }
    template<class SPEC> bool from( std::int8_t   &t, std::istream &json )     { std::int64_t  i; return from<SPEC>( i, json ) ? t = (std::int8_t  )(i), true : false; }

    template<class SPEC> bool from( double &t, std::istream &json )            { return json >> t ? true : false; }
    template<class SPEC> bool from( float &t, std::istream &json )             { double i; return from<SPEC>(i,json) ? t = float(i), true : false; }

    template<class SPEC> bool from( bool &t, std::istream &json ) {
        if( SPEC::next(  "true", json ) ) return t = true, true;
        if(!SPEC::prev(  "true", json ) ) return false;
        if( SPEC::next( "false", json ) ) return t = false, true;
        return false;
    }
    template<class SPEC> bool from( std::nullptr_t &t, std::istream &json ) {
        if( SPEC::next( "null", json ) ) return true;
        return SPEC::prev( "null", json );
    }

    // both sequence and associative containers
    template<class SPEC, typename CONTAINER>
    std::string to( const CONTAINER &t ) {
        std::stringstream ss;
        for( auto it = std::begin(t); it != std::end(t); ++it ) {
            ss << bourne::to<SPEC>(*it) << SPEC::separator;
        }
        std::string text = ss.str();
        auto in = SPEC::open_seq, out = SPEC::close_seq;
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out );
    }

    // pairs for associative containers
    template<class SPEC, typename K, typename V>
    std::string to( const std::pair<K,V> &t ) {
        std::stringstream ss;
        auto key = bourne::to<SPEC>( t.first);
        auto val = bourne::to<SPEC>( t.second);
        auto in = SPEC::open_seq, out = SPEC::close_seq, sep_kv = SPEC::separator_kv;
        if( SPEC::pair_iskv ) ss << in << key << sep_kv << val << out;
        else                  ss << in << val << sep_kv << key << out;
        return ss.str();
    }

    // associative containers, different encoding in/out characters
#   define bourne$$expand(TYPE, OPEN, CLOSE) \
    template<class SPEC, typename K, typename V, typename... Params> \
    std::string to( const TYPE <K,V,Params...> &t ) {  \
        auto in = SPEC::OPEN, out = SPEC::CLOSE; \
        auto sep = SPEC::separator, sep_kv = SPEC::separator_kv; \
        std::stringstream ss; \
        for( const auto &kv : t ) { \
            auto key = bourne::to<SPEC>(kv.first); \
            auto val = bourne::to<SPEC>(kv.second); \
            if( SPEC::pair_iskv ) ss << key << sep_kv << val << sep; \
            else                  ss << val << sep_kv << key << sep; \
        } \
        std::string text = ss.str(); \
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out ); \
    }
    bourne$$expand( std::map, open_smap, close_smap );
    bourne$$expand( std::multimap, open_smap, close_smap );
    bourne$$expand( std::unordered_map, open_umap, close_umap );
    bourne$$expand( std::unordered_multimap, open_umap, close_umap );
#   undef bourne$$expand

    // imports

    // pairs for associative containers
    template<class SPEC, typename K, typename V>
    bool from( std::pair<K,V> &t, std::istream &is ) {
        t = std::pair<K,V>();
        bool ok = true;
        ok = ok && SPEC::next( SPEC::open_seq, is );
        if(  ok && SPEC::next( SPEC::close_seq, is ) ) return ok;
        ok = ok && SPEC::prev( SPEC::close_seq, is );
        if( SPEC::pair_iskv ) {
        ok = ok && bourne::from<SPEC>( t.first, is );
        ok = ok && SPEC::next( SPEC::separator_kv, is );
        ok = ok && bourne::from<SPEC>( t.second, is );
        } else {
        ok = ok && bourne::from<SPEC>( t.second, is );
        ok = ok && SPEC::next( SPEC::separator_kv, is );
        ok = ok && bourne::from<SPEC>( t.first, is );
        }
        ok = ok && SPEC::next( SPEC::close_seq, is );
        return ok;
    }

    // both sequence and associative containers
    template<class SPEC,typename CONTAINER>
    bool from( CONTAINER &t, std::istream &is ) {
        typename CONTAINER::value_type val;
        t = CONTAINER();
        bool ok = true;
        ok = ok && SPEC::next( SPEC::open_seq, is );
        if(  ok && SPEC::next( SPEC::close_seq, is ) ) return ok;
        ok = ok && SPEC::prev( SPEC::close_seq, is );
        do {
            ok = ok && bourne::from<SPEC>(val, is );
            ok = ok && (std::inserter(t,t.end()) = val, true);
        } while( ok && SPEC::next( SPEC::separator, is ) );
        ok = ok && SPEC::prev( SPEC::separator, is );
        ok = ok && SPEC::next( SPEC::close_seq, is );
        return ok;
    }

    // associative containers, different encoding in/out characters
#   define bourne$$expand(TYPE,OPEN,CLOSE) \
    template<class SPEC,typename K, typename V, typename... Params> \
    bool from( TYPE <K,V,Params...> &t, std::istream &is ) { \
        std::pair<K,V> val; \
        t = TYPE<K,V,Params...>(); \
        bool ok = true; \
        ok = ok && SPEC::next( SPEC::OPEN, is ); \
        if(  ok && SPEC::next( SPEC::CLOSE, is ) ) return ok; \
        ok = ok && SPEC::prev( SPEC::CLOSE, is ); \
        do { \
            if( SPEC::pair_iskv ) { \
            ok = ok && bourne::from<SPEC>(val.first, is ); \
            ok = ok && SPEC::next( SPEC::separator_kv, is ); \
            ok = ok && bourne::from<SPEC>(val.second, is ); \
            } else { \
            ok = ok && bourne::from<SPEC>(val.second, is ); \
            ok = ok && SPEC::next( SPEC::separator_kv, is ); \
            ok = ok && bourne::from<SPEC>(val.first, is ); \
            } \
            ok = ok && (std::inserter(t,t.end()) = val, true); \
        } while( ok && SPEC::next( SPEC::separator, is ) ); \
        ok = ok && SPEC::prev( SPEC::separator, is ); \
        ok = ok && SPEC::next( SPEC::CLOSE, is ); \
        return ok; \
    }
    bourne$$expand( std::map, open_smap, close_smap )
    bourne$$expand( std::multimap, open_smap, close_smap )
    bourne$$expand( std::unordered_map, open_umap, close_umap )
    bourne$$expand( std::unordered_multimap, open_umap, close_umap )
#   undef  bourne$$expand

    template<class SPEC, typename T>
    bool from( T &t, const std::string &s ) {
        std::stringstream ss( s );
        return from<SPEC>( t, ss );
    }

    // tuples

    namespace {
        template<class SPEC, class TUPLE, std::size_t N>
        struct tuple {
            static std::string to( const TUPLE& t ) {
                return bourne::tuple<SPEC, TUPLE, N-1>::to(t) + SPEC::separator + bourne::to<SPEC>( std::get<N-1>(t) );
            }
            static bool from( TUPLE& t, std::istream &is ) {
                bool ok = true;
                ok = ok && bourne::tuple<SPEC, TUPLE, N-1>::from( t, is );
                ok = ok && SPEC::next( SPEC::separator, is );
                ok = ok && bourne::from<SPEC>( std::get<N-1>(t), is );
                return ok;
            }
        };
        template<class SPEC, class TUPLE>
        struct tuple<SPEC, TUPLE, 1> {
            static std::string to( const TUPLE& t ) {
                return bourne::to<SPEC>( std::get<0>(t) );
            }
            static bool from( TUPLE& t, std::istream &is ) {
                return bourne::from<SPEC>( std::get<0>(t), is );
            }
        };
    }
    template<class SPEC, class... Args>
    std::string to( const std::tuple<Args...>& t ) {
        std::stringstream ss;
        ss << SPEC::open_seq;
        ss << bourne::tuple<SPEC, decltype(t), sizeof...(Args)>::to(t);
        ss << SPEC::close_seq;
        return ss.str();
    }
    template<class SPEC, class... Args>
    bool from( std::tuple<Args...> &t, std::istream &is ) {
        bool ok = true;
        ok = ok && SPEC::next( SPEC::open_seq, is );
        if(  ok && SPEC::next( SPEC::close_seq, is ) ) return ok;
        ok = ok && SPEC::prev( SPEC::close_seq, is );
        ok = ok && bourne::tuple<SPEC, decltype(t), sizeof...(Args)>::from(t, is );
        ok = ok && SPEC::next( SPEC::close_seq, is );
        return ok;
    }

    // high level object saving/loading

    struct obj {
        template<typename T>
        static T &get(const void *ptr) {
            typedef std::pair<std::thread::id,const void *> pair;
            static std::map<pair, T> all;
            pair p( std::this_thread::get_id(), ptr );
            return all[ p ] = all[ p ];
        }
    };

    template<typename T>
    void save( const T& t ) {
        std::string json = to<specs<JSON>>( t );
        obj::get<std::string>(&t) = json;
    }
    template<typename T>
    bool load( T& t ) {
        std::string json = obj::get<std::string>(&t);
        return from<specs<JSON>>( t, json );
    }
    template<typename T>
    void clear( T& t ) {
        t = T();
    }
    template<typename T>
    std::ostream &print( const T &t, std::ostream &os = std::cout ) {
        return os << to<specs<JSON>>(t) << std::endl, os;
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

    template<typename T>
    bool test( const T &obj ) {
        std::string before = to<specs<JSON>>( obj );
        auto copy = T();
        if( !from<specs<JSON>>( copy, before ) ) {
            return false;
        }
        std::string after = to<specs<JSON>>( copy );
        return before == after;
    }
}

// optional custom-class autoserialization macro
#define BOURNE_DEFINE( OBJECT, PARGS ) \
namespace bourne { \
    template<class SPEC> \
    std::string to( const OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        return bourne::to<SPEC>( _tpl ); \
    } \
    template<class SPEC> \
    bool from( OBJECT, std::istream &is ) { \
        auto _tpl = std::make_tuple PARGS; \
        return bourne::from<SPEC>( _tpl, is ) ? (std::tie PARGS = _tpl, true) : false;  \
    } \
}

namespace bourne {

    template <typename T>
    std::string to_json( const T &t ) {
        return to<specs<JSON>>( t );
    }

    template <typename T, typename ISTREAM>
    bool from_json( T &t, ISTREAM &is ) {
        return from<specs<JSON>>( t, is );
    }
}

#if 0
namespace std {
    using bourne::to_json;
    using bourne::from_json;
}
#endif
