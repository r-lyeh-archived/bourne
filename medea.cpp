#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "medea.hpp"

namespace medea
{
#if 0
    // json draft
    const bool pair_iskv = true;
    const char separator = ',';
    const char separator_kv = ':';
    const char open_seq = '[';
    const char open_umap = '{';
    const char open_smap = '{';
    const char open_rope = '\"';
    const char close_seq = ']';
    const char close_umap = '}';
    const char close_smap = '}';
    const char close_rope = '\"';
    // to_quote, from_quote
#else
    // medea draft
    const bool pair_iskv = true;
    const char separator = ',';
    const char separator_kv = ':';
    const char open_seq   = '[';
    const char open_umap  = '{';
    const char open_smap  = '<';
    const char open_rope = '\"';
    const char close_seq  = ']';
    const char close_umap = '}';
    const char close_smap = '>';
    const char close_rope = '\"';
#endif

    // utils
    std::string utils::to_quote( const std::string &t ) {
//      return utils::encode( t );

        std::string out;
        for( auto &it : t )
            if( it != open_rope )
                out += it;
            else
                out += "\\\"";

        return std::string() + open_rope + out + close_rope;
    }

    bool utils::from_quote( std::string &t, std::istream &is ) {
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

    bool utils::next( const std::string &token, std::istream &is) {
        char ch;
        bool ok = true;
        for( auto &it : token )
            ok &= ( is >> ch ? ch == it : false );
        return ok;
    }
    bool utils::prev( const std::string &token, std::istream &is) {
        for( auto &it : token ) is.unget();
        return is.good();
    }
    bool utils::next( const char &ch, std::istream &is ) {
        return utils::next( std::string() + ch, is );
    }
    bool utils::prev( const char &ch, std::istream &is ) {
        return utils::prev( std::string() + ch, is );
    }


    // exports
    std::string to_json( std::string::value_type * const &t )    { return to_json( std::string(t) ); }
    std::string to_json( const std::string::value_type *t )      { return to_json( std::string(t) ); }
    std::string to_json( const std::string::value_type &t )      { return to_json( std::string() + t ); }
    std::string to_json( const std::string &t )                  { return utils::to_quote( t ); }

    std::string to_json( const std::uint8_t &t )      { return to_json( std::uint64_t(t) ); }
    std::string to_json( const std::uint16_t &t )     { return to_json( std::uint64_t(t) ); }
    std::string to_json( const std::uint32_t &t )     { return to_json( std::uint64_t(t) ); }
    std::string to_json( const std::uint64_t &t )     { return std::to_string(t); }

    std::string to_json( const std::int8_t &t )       { return to_json( std::int64_t(t) ); }
    std::string to_json( const std::int16_t &t )      { return to_json( std::int64_t(t) ); }
    std::string to_json( const std::int32_t &t )      { return to_json( std::int64_t(t) ); }
    std::string to_json( const std::int64_t &t )      { return std::to_string(t); }

    std::string to_json( const bool &t )              { return t ? "true" : "false"; }
    std::string to_json( const float &t )             { return to_json( double(t) ); }
    std::string to_json( const double &t )            { return std::to_string(t); }
    std::string to_json( const std::nullptr_t &t )    { return "null"; }

    // imports
    bool from_json( double &t, std::istream &json );
    bool from_json( std::int64_t &t, std::istream &json );
    bool from_json( std::uint64_t &t, std::istream &json );
    bool from_json( std::string  &t, std::istream &json );

    bool from_json( std::string::value_type &t, std::istream &json ) { std::string xx; return from_json(xx,json) ? t = xx[0], true : false; }
    bool from_json( std::string &t, std::istream &json )             { return utils::from_quote(t,json) ? true : false; }

    bool from_json( std::uint8_t &t, std::istream &json )      { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint8_t )(xx), true : false; }
    bool from_json( std::uint16_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint16_t)(xx), true : false; }
    bool from_json( std::uint32_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = (std::uint32_t)(xx), true : false; }
    bool from_json( std::uint64_t &t, std::istream &json )     { return json >> t ? true : false; }

    bool from_json( std::int8_t &t, std::istream &json )       { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int8_t )(xx), true : false; }
    bool from_json( std::int16_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int16_t)(xx), true : false; }
    bool from_json( std::int32_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = (std::int32_t)(xx), true : false; }
    bool from_json( std::int64_t &t, std::istream &json )      { return json >> t ? true : false; }

    bool from_json( std::nullptr_t &t, std::istream &json ) {
        if( utils::next( "null", json ) ) return true;
        return utils::prev( "null", json );
    }
    bool from_json( bool &t, std::istream &json ) {
        if( utils::next(  "true", json ) ) return t = true, true;
        if(!utils::prev(  "true", json ) ) return false;
        if( utils::next( "false", json ) ) return t = false, true;
        return false;
    }

    bool from_json( float &t, std::istream &json )             { double xx; return from_json(xx,json) ? t = float(xx), true : false; }
    bool from_json( double &t, std::istream &json )            { return json >> t ? true : false; }
}   // medea::



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

#include <string>

namespace medea {
namespace utils
{
    #if 0 // henke's original
        static const unsigned char dectab[256] = {
            91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //000..015
            91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, 91, //016..031
            91, 62, 90, 63, 64, 65, 66, 91, 67, 68, 69, 70, 71, 91, 72, 73, //032..047
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 74, 75, 76, 77, 78, 79, //048..063
            80,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, //064..079
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 81, 91, 82, 83, 84, //080..095
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
        }, enctab[91] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', //00..12
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', //13..25
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', //26..38
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', //39..51
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '#', '$', //52..64
            '%', '&', '(', ')', '*', '+', ',', '.', '/', ':', ';', '<', '=', //65..77
            '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', '"'  //78..90
        };
    #else // rlyeh's modification
        static const unsigned char enctab[91] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', //00..12
            'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', //13..25
            'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', //26..38
            'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', //39..51
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '!', '#', '$', //52..64
            '%', '&', '(', ')', '*', '+', ',', '.', '/', ':', ';', '-', '=', //65..77
            '\\','?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~', '\'' //78..90
        }, dectab[256] = {
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
    #endif

    std::string encode( const std::string &binary ) {
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

    std::string decode( const std::string &text ) {
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
}
}
