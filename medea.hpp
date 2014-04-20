// Medea is a lightweight JSON serializer for most STL containers.
// - rlyeh ~~ listening to incredible hog / execution.

/*
 * A custom base91 encoder/decoder (XML and JSON friendly).
 * Copyright (c) 2011-2013 Mario 'rlyeh' Rodriguez
 *
 * Original basE91 encoder/decoder by Joachim Henke.
 * Copyright (c) 2000-2006 Joachim Henke
 * http://base91.sourceforge.net/ (v0.6.0)

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  - Neither the name of Joachim Henke nor the names of his contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.

 * - rlyeh
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
    class utils { public:
        static std::string encode( const std::string &binary ) {
            // rlyeh's modification
            static const unsigned char enctab[91] = {
                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', //00..12
                'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', //13..25
                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', //26..38
                'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', //39..51
                '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '#', '$', //52..64
                '%', '&', '(', ')', '*', '+', ',', '.', '/', ':', ';', '-', '=', //65..77
                '\\','?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', '\'' //78..90
            };
            std::string ob;
            const unsigned char *ib = (unsigned char *) binary.c_str();
            unsigned long queue = 0;
            unsigned int nbits = 0;
            for( size_t len = binary.size(); len--; ) {
                queue |= *ib++ << nbits;
                nbits += 8;
                if (nbits > 13) {   /* enough bits in queue */
                    unsigned int val = queue & 8191;

                    if (val > 88) {
                        queue >>= 13;
                        nbits -= 13;
                    } else {    /* we can take 14 bits */
                        val = queue & 16383;
                        queue >>= 14;
                        nbits -= 14;
                    }
                    ob.push_back( enctab[val % 91] );
                    ob.push_back( enctab[val / 91] );
                }
            }
            /* process remaining bits from bit queue; write up to 2 bytes */
            if (nbits) {
                ob.push_back( enctab[queue % 91] );
                if (nbits > 7 || queue > 90)
                    ob.push_back( enctab[queue / 91] );
            }
            /* return text data */
            return ob;
        }

        static std::string decode( const std::string &text ) {
            // rlyeh's modification
            static const unsigned char dectab[256] = {
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //000..015
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //016..031
                91, 62, 91, 63, 64, 65, 66, 90, 67, 68, 69, 70, 71, 76, 72, 73, //032..047 // @34: ", @39: ', @45: -
                52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 74, 75, 91, 77, 91, 79, //048..063 // @60: <, @62: >
                80,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //064..079
                15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 81, 78, 82, 83, 84, //080..095 // @92: slash
                85, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, //096..111
                41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 86, 87, 88, 89, 91, //112..127
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //128..143
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //144..159
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //160..175
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //176..191
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //192..207
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //208..223
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //224..239
                91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91  //240..255
            };
            std::string ob;
            const unsigned char *ib = (unsigned char *) text.c_str();
            unsigned long queue = 0;
            unsigned int nbits = 0;
            int val = -1;
            for( size_t len = text.size(); len--; ) {
                unsigned int d = dectab[*ib++];
                if (d == 91)
                    continue;   /* ignore non-alphabet chars */
                if (val == -1)
                    val = d;    /* start next value */
                else {
                    val += d * 91;
                    queue |= val << nbits;
                    nbits += (val & 8191) > 88 ? 13 : 14;
                    do {
                        ob.push_back( char( queue ) );
                        queue >>= 8;
                        nbits -= 8;
                    } while (nbits > 7);
                    val = -1;   /* mark value complete */
                }
            }
            /* process remaining bits; write at most 1 byte */
            if (val != -1)
                ob.push_back( char( queue | val << nbits ) );
            /* return original binary data */
            return ob;
        }

        static std::string to_quote( const std::string &t ) {
    //      return utils::encode( t );
            std::string out;
            for( auto &it : t )
                if( it != open_rope )
                    out += it;
                else
                    out += "\\\"";

            return std::string() + open_rope + out + close_rope;
        }

        static bool from_quote( std::string &t, std::istream &is ) {
    //      return utils::decode( t );
            struct raiiss {
                std::istream &is;
                unsigned counter;
                raiiss( std::istream &_is ) : counter(0), is(_is) {}
                ~raiiss() { while( counter ) counter--, is.unget(); }
                bool operator >>( char &ch ) {
                    return (!is.eof()) && (++counter) && (is >> ch);
                }
            } raii(is);

            t = std::string();
            int status = 0;
            char ch;
            while( raii >> ch ) {
                if( status == 0 ) {
                    if( ch == open_rope ) is >> std::noskipws, status++;
                    else return false;
                }
                else
                if( status == 1 ) {
                    if( ch == close_rope ) return raii.counter = 0, is >> std::skipws, true;
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

        static bool next( const std::string &token, std::istream &is) {
            char ch;
            bool ok = true;
            for( auto &it : token )
                ok &= ( is >> ch ? ch == it : false );
            return ok;
        }
        static bool prev( const std::string &token, std::istream &is) {
            for( auto &it : token ) is.unget();
            return is.good();
        }
        static bool next( const char &ch, std::istream &is ) {
            return utils::next( std::string() + ch, is );
        }
        static bool prev( const char &ch, std::istream &is ) {
            return utils::prev( std::string() + ch, is );
        }
    };

    // exports

    //template<size_t N>
    //std::string to_json( const std::string::value_type (&t)[N] );

    static inline std::string to_json( const std::string &t )                  { return utils::to_quote( t ); }
    static inline std::string to_json( const std::string::value_type *t )      { return to_json( std::string(t) ); }
    static inline std::string to_json( const std::string::value_type &t )      { return to_json( std::string() + t ); }
    static inline std::string to_json( std::string::value_type * const &t )    { return to_json( std::string(t) ); }

    static inline std::string to_json( const std::uint64_t &t )     { return std::to_string(t); }
    static inline std::string to_json( const std::uint32_t &t )     { return to_json( std::uint64_t(t) ); }
    static inline std::string to_json( const std::uint16_t &t )     { return to_json( std::uint64_t(t) ); }
    static inline std::string to_json( const std::uint8_t &t )      { return to_json( std::uint64_t(t) ); }

    static inline std::string to_json( const std::int64_t &t )      { return std::to_string(t); }
    static inline std::string to_json( const std::int32_t &t )      { return to_json( std::int64_t(t) ); }
    static inline std::string to_json( const std::int16_t &t )      { return to_json( std::int64_t(t) ); }
    static inline std::string to_json( const std::int8_t &t )       { return to_json( std::int64_t(t) ); }

    static inline std::string to_json( const double &t )            { return std::to_string(t); }
    static inline std::string to_json( const float &t )             { return to_json( double(t) ); }
    static inline std::string to_json( const bool &t )              { return t ? "true" : "false"; }
    static inline std::string to_json( const std::nullptr_t &t )    { return "null"; }

    // imports

    static inline bool from_json( std::string &t, std::istream &json )             { return utils::from_quote(t,json) ? true : false; }
    static inline bool from_json( std::string::value_type &t, std::istream &json ) { std::string xx; return from_json(xx,json) ? t = xx[0], true : false; }

    static inline bool from_json( std::uint64_t &t, std::istream &json )     { return json >> t ? true : false; }
    static inline bool from_json( std::uint32_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint32_t)(xx), true : false; }
    static inline bool from_json( std::uint16_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint16_t)(xx), true : false; }
    static inline bool from_json( std::uint8_t &t, std::istream &json )      { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint8_t )(xx), true : false; }

    static inline bool from_json( std::int64_t &t, std::istream &json )      { return json >> t ? true : false; }
    static inline bool from_json( std::int32_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int32_t)(xx), true : false; }
    static inline bool from_json( std::int16_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int16_t)(xx), true : false; }
    static inline bool from_json( std::int8_t &t, std::istream &json )       { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int8_t )(xx), true : false; }

    static inline bool from_json( double &t, std::istream &json )            { return json >> t ? true : false; }
    static inline bool from_json( float &t, std::istream &json )             { double xx; return from_json(xx,json) ? t = float(xx), true : false; }

    static inline bool from_json( bool &t, std::istream &json ) {
        if( utils::next(  "true", json ) ) return t = true, true;
        if(!utils::prev(  "true", json ) ) return false;
        if( utils::next( "false", json ) ) return t = false, true;
        return false;
    }
    static inline bool from_json( std::nullptr_t &t, std::istream &json ) {
        if( utils::next( "null", json ) ) return true;
        return utils::prev( "null", json );
    }

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

        explicit archive( bool L, bool S ) : is_loading(L), is_saving(S), is_json(true)
        {}

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

    template<typename T>
    bool test( const T &obj ) {
        std::string before = medea::to_json( obj );
        auto copy = T();
        if( !medea::from_json( copy, before ) ) {
            return false;
        }
        std::string after = medea::to_json( copy );
        return before == after;
    }
}

// optional custom-class autoserialization macro
#define MEDEA_DEFINE( OBJECT, PARGS ) \
namespace medea { \
    std::string to_json( const ::OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::to_json( _tpl ); \
    } \
    bool from_json( ::OBJECT, std::istream &is ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::from_json( _tpl, is ) ? (std::tie PARGS = _tpl, true) : false;  \
    } \
}

#if 0
namespace std {
    using medea::to_json;
    using medea::from_json;
}
#endif
