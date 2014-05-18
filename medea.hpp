/* Medea is a lightweight and tiny serializer written in C++11.
 * Copyright (c) 2013,2014 Mario 'rlyeh' Rodriguez

 * Contains code based on basE91 encoder/decoder by Joachim Henke.
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

namespace medea
{
    enum spec {
        MEDEA,
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
                scope( std::istream &_is ) : is( _is )
                {}
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

    template <>
    struct traits<MEDEA> {
        // medea draft
        static const bool pair_iskv = false;
        static const char separator = ',',  separator_kv = ':';
        static const char open_seq  = '[',  close_seq    = ']';
        static const char open_umap = '{',  close_umap   = '}';
        static const char open_smap = '<',  close_smap   = '>';
        static const char open_rope = '\"', close_rope   = '\"';

        static std::string quote( const std::string &t ) {
            return traits<JSON>::quote( t );
    //      return specs::encode( t );
        }

        static bool unquote( std::string &t, std::istream &is ) {
            return traits<JSON>::unquote( t, is );
    //      return specs::decode( t );
        }
    };

    // specs, common
    template< spec SPEC >
    struct specs : public traits<SPEC> {
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
            ss << medea::to<SPEC>(*it) << SPEC::separator;
        }
        std::string text = ss.str();
        auto in = SPEC::open_seq, out = SPEC::close_seq;
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out );
    }

    // pairs for associative containers
    template<class SPEC, typename K, typename V>
    std::string to( const std::pair<K,V> &t ) {
        std::stringstream ss;
        auto key = medea::to<SPEC>( t.first);
        auto val = medea::to<SPEC>( t.second);
        auto in = SPEC::open_seq, out = SPEC::close_seq, sep_kv = SPEC::separator_kv;
        if( SPEC::pair_iskv ) ss << in << key << sep_kv << val << out;
        else                  ss << in << val << sep_kv << key << out;
        return ss.str();
    }

    // associative containers, different encoding in/out characters
#   define $medea_expand(TYPE, OPEN, CLOSE) \
    template<class SPEC, typename K, typename V, typename... Params> \
    std::string to( const TYPE <K,V,Params...> &t ) {  \
        auto in = SPEC::OPEN, out = SPEC::CLOSE; \
        auto sep = SPEC::separator, sep_kv = SPEC::separator_kv; \
        std::stringstream ss; \
        for( const auto &kv : t ) { \
            auto key = medea::to<SPEC>(kv.first); \
            auto val = medea::to<SPEC>(kv.second); \
            if( SPEC::pair_iskv ) ss << key << sep_kv << val << sep; \
            else                  ss << val << sep_kv << key << sep; \
        } \
        std::string text = ss.str(); \
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out ); \
    }
    $medea_expand( std::map, open_smap, close_smap );
    $medea_expand( std::multimap, open_smap, close_smap );
    $medea_expand( std::unordered_map, open_umap, close_umap );
    $medea_expand( std::unordered_multimap, open_umap, close_umap );
#   undef $medea_expand

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
        ok = ok && medea::from<SPEC>( t.first, is );
        ok = ok && SPEC::next( SPEC::separator_kv, is );
        ok = ok && medea::from<SPEC>( t.second, is );
        } else {
        ok = ok && medea::from<SPEC>( t.second, is );
        ok = ok && SPEC::next( SPEC::separator_kv, is );
        ok = ok && medea::from<SPEC>( t.first, is );
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
            ok = ok && medea::from<SPEC>(val, is );
            ok = ok && (std::inserter(t,t.end()) = val, true);
        } while( ok && SPEC::next( SPEC::separator, is ) );
        ok = ok && SPEC::prev( SPEC::separator, is );
        ok = ok && SPEC::next( SPEC::close_seq, is );
        return ok;
    }

    // associative containers, different encoding in/out characters
#   define $medea_expand(TYPE,OPEN,CLOSE) \
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
            ok = ok && medea::from<SPEC>(val.first, is ); \
            ok = ok && SPEC::next( SPEC::separator_kv, is ); \
            ok = ok && medea::from<SPEC>(val.second, is ); \
            } else { \
            ok = ok && medea::from<SPEC>(val.second, is ); \
            ok = ok && SPEC::next( SPEC::separator_kv, is ); \
            ok = ok && medea::from<SPEC>(val.first, is ); \
            } \
            ok = ok && (std::inserter(t,t.end()) = val, true); \
        } while( ok && SPEC::next( SPEC::separator, is ) ); \
        ok = ok && SPEC::prev( SPEC::separator, is ); \
        ok = ok && SPEC::next( SPEC::CLOSE, is ); \
        return ok; \
    }
    $medea_expand( std::map, open_smap, close_smap )
    $medea_expand( std::multimap, open_smap, close_smap )
    $medea_expand( std::unordered_map, open_umap, close_umap )
    $medea_expand( std::unordered_multimap, open_umap, close_umap )
#   undef  $medea_expand

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
                return medea::tuple<SPEC, TUPLE, N-1>::to(t) + SPEC::separator + medea::to<SPEC>( std::get<N-1>(t) );
            }
            static bool from( TUPLE& t, std::istream &is ) {
                bool ok = true;
                ok = ok && medea::tuple<SPEC, TUPLE, N-1>::from( t, is );
                ok = ok && SPEC::next( SPEC::separator, is );
                ok = ok && medea::from<SPEC>( std::get<N-1>(t), is );
                return ok;
            }
        };
        template<class SPEC, class TUPLE>
        struct tuple<SPEC, TUPLE, 1> {
            static std::string to( const TUPLE& t ) {
                return medea::to<SPEC>( std::get<0>(t) );
            }
            static bool from( TUPLE& t, std::istream &is ) {
                return medea::from<SPEC>( std::get<0>(t), is );
            }
        };
    }
    template<class SPEC, class... Args>
    std::string to( const std::tuple<Args...>& t ) {
        std::stringstream ss;
        ss << SPEC::open_seq;
        ss << medea::tuple<SPEC, decltype(t), sizeof...(Args)>::to(t);
        ss << SPEC::close_seq;
        return ss.str();
    }
    template<class SPEC, class... Args>
    bool from( std::tuple<Args...> &t, std::istream &is ) {
        bool ok = true;
        ok = ok && SPEC::next( SPEC::open_seq, is );
        if(  ok && SPEC::next( SPEC::close_seq, is ) ) return ok;
        ok = ok && SPEC::prev( SPEC::close_seq, is );
        ok = ok && medea::tuple<SPEC, decltype(t), sizeof...(Args)>::from(t, is );
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
#define MEDEA_DEFINE( OBJECT, PARGS ) \
namespace medea { \
    template<class SPEC> \
    std::string to( const OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::to<SPEC>( _tpl ); \
    } \
    template<class SPEC> \
    bool from( OBJECT, std::istream &is ) { \
        auto _tpl = std::make_tuple PARGS; \
        return medea::from<SPEC>( _tpl, is ) ? (std::tie PARGS = _tpl, true) : false;  \
    } \
}

namespace medea {

    template <typename T>
    std::string to_json( const T &t ) {
        return to<specs<JSON>>( t );
    }
    template <typename T>
    std::string to_medea( const T &t ) {
        return to<specs<MEDEA>>( t );
    }

    template <typename T, typename ISTREAM>
    bool from_json( T &t, ISTREAM &is ) {
        return from<specs<JSON>>( t, is );
    }
    template <typename T, typename ISTREAM >
    bool from_medea( T &t, ISTREAM &is ) {
        return from<specs<MEDEA>>( t, is );
    }
}

#if 0
namespace std {
    using medea::to_json;
    using medea::from_json;
}
#endif
