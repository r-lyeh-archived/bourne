#include <cassert>
#include <iostream>
#include <map>
#include <vector>
#include <string>

#include "medea.hpp"

namespace medea
{
    // utils
    std::string utils::to_quote( const std::string &t ) {
        std::string out;
        for( auto &it : t )
            if( it != '\"' )
                out += it;
            else
                out += "\\\"";
        return std::string() + '\"' + out + '\"';
    }

    bool utils::from_quote( std::string &t, std::istream &is ) {
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
                if( ch == '\"' ) is >> std::noskipws, status++;
                else return false;
            }
            else
            if( status == 1 ) {
                if( ch == '\"' ) return raii.counter = 0, is >> std::skipws, true;
                if( ch != '\\' ) t += ch;
                else status++;
            }
            else
            if( status == 2 ) {
                if( ch == '\"' ) status--, t += ch;
                else return false;
            }
        }
        return false;
    }

    bool utils::next( std::istream &is, char ch, char ch2 ) {
        return ( is >> ch2 ? ch == ch2 : false );
    }
    bool utils::prev( std::istream &is ) {
        return is.unget(), is.good();
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

    bool from_json( std::uint8_t &t, std::istream &json )      { std::uint64_t xx; return from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::uint16_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::uint32_t &t, std::istream &json )     { std::uint64_t xx; return from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::uint64_t &t, std::istream &json )     { return json >> t ? true : false; }

    bool from_json( std::int8_t &t, std::istream &json )       { std::int64_t xx; return  from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::int16_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::int32_t &t, std::istream &json )      { std::int64_t xx; return  from_json( xx, json ) ? t = decltype(t)(xx), true : false; }
    bool from_json( std::int64_t &t, std::istream &json )      { return json >> t ? true : false; }

    bool from_json( std::nullptr_t &t, std::istream &json )              {
        int rolls = 0;
        if(                          json.peek() == 'n' ) {
            if( rolls++, json.get(), json.peek() == 'u' )
            if( rolls++, json.get(), json.peek() == 'l' )
            if( rolls++, json.get(), json.peek() == 'l' )
                return t = nullptr, true;
        }
        while( rolls-- ) json.unget();
        return false;
    }
    bool from_json( bool &t, std::istream &json )              {
        int rolls = 0;
        if(                          json.peek() == 't' ) {
            if( rolls++, json.get(), json.peek() == 'r' )
            if( rolls++, json.get(), json.peek() == 'u' )
            if( rolls++, json.get(), json.peek() == 'e' )
                return t = true, true;
        }
        else
        if(                          json.peek() == 'f' ) {
            if( rolls++, json.get(), json.peek() == 'a' )
            if( rolls++, json.get(), json.peek() == 'l' )
            if( rolls++, json.get(), json.peek() == 's' )
            if( rolls++, json.get(), json.peek() == 'e' )
                return t = false, true;
        }
        while( rolls-- ) json.unget();
        return false;
    }

    bool from_json( float &t, std::istream &json )             { double xx; return from_json(xx,json) ? t = decltype(t)(xx), true : false; }
    bool from_json( double &t, std::istream &json )            { return json >> t ? true : false; }
}   // medea::
