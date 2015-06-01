/* Bourne is a lightweight JSON serializer written in C++11 w/ SFINAE.
 * Copyright (c) 2013,2014,2015 Mario 'rlyeh' Rodriguez, zlib/libpng licensed.

 * - rlyeh ~~ listening to Incredible Hog / Execution.
 */

#pragma once

#include <cstdint>

#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#define BOURNE_VERSION "1.0.1" /* (2015/06/01) Allow helper macros in header files
#define BOURNE_VERSION "1.0.0" // (2015/02/25) SFINAE version
#define BOURNE_VERSION "0.0.0" // (2013/07/15) Initial commit */

namespace bourne {

    // imports & exports

    namespace imports {
        template<typename T>
        static void basic( std::istream &in, T &t );

        static void basic( std::istream &in, std::string &t ) {
            for( std::string line; !in.fail() && std::getline(in, line, '\"'); ) {
                t.clear();
                for( ; !in.fail() && std::getline(in, line, '\"'); ) {
                    if( line.size() && line.back() == '\\' ) {
                        line.back() = '\"';
                        t += line;
                    } else {
                        t += line;
                        return;
                    }
                }
            }
        }

        static void basic( std::istream &in, std::string::value_type &t ) { std::string i; basic(in,i); t = i[0]; }

        static void basic( std::istream &in, std::uint64_t &t )     { in >> t; }
        static void basic( std::istream &in, std::uint32_t &t )     { std::uint64_t i;  basic( in, i ); t = (std::uint32_t)(i); }
        static void basic( std::istream &in, std::uint16_t &t )     { std::uint64_t i;  basic( in, i ); t = (std::uint16_t)(i); }
        static void basic( std::istream &in, std::uint8_t  &t )     { std::uint64_t i;  basic( in, i ); t = (std::uint8_t )(i); }

        static void basic( std::istream &in, std::int64_t  &t )     { in >> t; }
        static void basic( std::istream &in, std::int32_t  &t )     { std::int64_t  i;  basic( in, i ); t = (std::int32_t )(i); }
        static void basic( std::istream &in, std::int16_t  &t )     { std::int64_t  i;  basic( in, i ); t = (std::int16_t )(i); }
        static void basic( std::istream &in, std::int8_t   &t )     { std::int64_t  i;  basic( in, i ); t = (std::int8_t  )(i); }

        static void basic( std::istream &in, double &t )            { in >> t; }
        static void basic( std::istream &in, float &t )             { double i; basic( in, i ); t = float(i); }

        static void basic( std::istream &in, bool &t ) {
            std::string i;
            in >> i;
            while( i.size() && (i.back() < 'a' || i.back() > 'z') ) {
                i.pop_back();
                in.unget();
            }
            /**/ if( i == "true"  ) t = true;
            else if( i == "false" ) t = false;
            else t = false, in.setstate( std::ios::failbit );
        }

        static void basic( std::istream &in, std::nullptr_t &t ) {
            std::string i;
            in >> i;
            while( i.size() && (i.back() < 'a' || i.back() > 'z') ) {
                i.pop_back();
                in.unget();
            }
            if( i != "null" ) in.setstate( std::ios::failbit );
        }
    }

    namespace exports {
        template<typename T>
        static void basic( std::ostream &out, const T &t );

        static void basic( std::ostream &out, const std::string &t ) {
            out << '\"';
            for( const auto &ch : t ) {
                if( ch != '\"' ) out << ch;
                else             out << "\\\"";
            }
            out << '\"';
        }

        static void basic( std::ostream &out, std::string::value_type * const &t )    { basic( out, std::string(t) ); }
        static void basic( std::ostream &out, const std::string::value_type *t )      { basic( out, std::string(t) ); }
        static void basic( std::ostream &out, const std::string::value_type &t )      { basic( out, std::string() + t ); }

        static void basic( std::ostream &out, const std::uint64_t &t )     { out << std::to_string(t); }
        static void basic( std::ostream &out, const std::uint32_t &t )     { basic( out, std::uint64_t(t) ); }
        static void basic( std::ostream &out, const std::uint16_t &t )     { basic( out, std::uint64_t(t) ); }
        static void basic( std::ostream &out, const std::uint8_t &t )      { basic( out, std::uint64_t(t) ); }

        static void basic( std::ostream &out, const std::int64_t &t )      { out << std::to_string(t); }
        static void basic( std::ostream &out, const std::int32_t &t )      { basic( out, std::int64_t(t) ); }
        static void basic( std::ostream &out, const std::int16_t &t )      { basic( out, std::int64_t(t) ); }
        static void basic( std::ostream &out, const std::int8_t &t )       { basic( out, std::int64_t(t) ); }

        static void basic( std::ostream &out, const double &t )            { out << std::to_string(t); }
        static void basic( std::ostream &out, const float &t )             { basic( out, double(t) ); }
        static void basic( std::ostream &out, const bool &t )              { out << ( t ? "true" : "false" ); }
        static void basic( std::ostream &out, const std::nullptr_t &t )    { out << "null"; }
    }

    // traits

    struct trait {
        typedef char true_type;
        typedef long false_type;
    };

    // C/C++ string trait

    template<class T>
    struct is_string : std::integral_constant<
        bool,
        std::is_same<char const *, typename std::decay<T>::type>::value ||
        std::is_same<char       *, typename std::decay<T>::type>::value
    > {};

    template <>
    struct is_string<std::string> : std::true_type {};

    // associative container trait (has_iterator && has_key_type && has_mapped_type)

    template <typename T>
    struct is_associative : trait {
        template <typename U> static true_type  has_iterator(typename U::iterator *);
        template <typename U> static false_type has_iterator(...);
        template <typename U> static true_type  has_key(typename U::key_type *);
        template <typename U> static false_type has_key(...);
        template <typename U> static true_type  has_mapped(typename U::mapped_type *);
        template <typename U> static false_type has_mapped(...);
        enum {
            value = ( true
                && (sizeof(has_iterator<T>(0)) == sizeof(true_type))
                && (sizeof(has_key<T>(0)) == sizeof(true_type))
                && (sizeof(has_mapped<T>(0)) == sizeof(true_type))
            )
        };
    };

    // sequential container trait (has_iterator && has_value && !is_string)

    template <typename T>
    struct is_sequential : trait {
        template <typename U> static true_type  has_iterator(typename U::iterator *);
        template <typename U> static false_type has_iterator(...);
        template <typename U> static true_type  has_value(typename U::value_type *);
        template <typename U> static false_type has_value(...);
        enum {
            value = ( true
                && (!is_string<T>::value)
                && (!is_associative<T>::value)
                && (sizeof(has_iterator<T>(0)) == sizeof(true_type))
                && (sizeof(has_value<T>(0)) == sizeof(true_type))
            )
        };
    };

    // pair container trait (has_first && has_second)

    template <typename T>
    struct is_pair : trait {
        template <typename U> static true_type  has_first(typename U::first_type *);
        template <typename U> static false_type has_first(...);
        template <typename U> static true_type  has_second(typename U::second_type *);
        template <typename U> static false_type has_second(...);
        enum {
            value = ( true
                && (sizeof(has_first<T>(0)) == sizeof(true_type))
                && (sizeof(has_second<T>(0)) == sizeof(true_type))
            )
        };
    };

    // helpers (type which holds an unsigned integer value)

    template<unsigned> struct int_{};

    // de/serializer

    struct parser {

        template <class Tuple, unsigned Pos>
        static void tuple( std::istream &in, Tuple &t, int_<Pos> ) {
            char sep;
            parse( in, std::get< std::tuple_size<Tuple>::value-Pos >(t) );
            in >> ( sep = ',' );
            tuple( in, t, int_<Pos-1>() );
        }

        template <class Tuple>
        static void tuple( std::istream &in, Tuple &t, int_<1> ) {
            parse( in, std::get<std::tuple_size<Tuple>::value-1>(t) );
        }

        // generic parsers

        template <class... Args>
        static void
        parse( std::istream &in, std::tuple<Args...> &t ) {
            t = std::tuple<Args...> {};
            char sep;
            in >> ( sep = '[' );
            tuple( in, t, int_<sizeof...(Args)>() );
            in >> ( sep = ']' );
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && !is_associative<T>::value && !is_pair<T>::value >::type
        parse( std::istream &in, T &t ) {
            t = T {};
            bourne::imports::
            basic( in, t );
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && !is_associative<T>::value && is_pair<T>::value >::type
        parse( std::istream &in, T &t ) {
            t = T {};
            char sep;
            in >> ( sep = '{' );
            parse( in, t.first );
            in >> ( sep = ':' );
            parse( in, t.second );
            in >> ( sep = '}' );
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && is_associative<T>::value && !is_pair<T>::value >::type
        parse( std::istream &in, T &t ) {
            t = T {};
            char sep;
            in >> ( sep = '{' );
            std::pair< typename std::remove_const<typename T::key_type>::type, typename T::mapped_type > value {};
            while( !in.fail() && sep != '}' ) {
                parse( in, value.first );
                in >> ( sep = ':' );
                parse( in, value.second );
                if( !in.fail() ) std::inserter(t,t.end()) = value;
                in >> ( sep = ',' );
            }
        }

        template <typename T>
        static typename std::enable_if< is_sequential<T>::value && !is_associative<T>::value && !is_pair<T>::value >::type
        parse( std::istream &in, T &t) {
            t = T {};
            char sep;
            in >> ( sep = '[' );
            typename T::value_type value = {};
            while( !in.fail() && sep != ']' ) {
                parse( in, value );
                if( !in.fail() ) std::inserter(t,t.end()) = value;
                in >> ( sep = ',' );
            }
        }
    };

    struct printer {

        template <class Tuple, unsigned Pos>
        static void tuple( std::ostream &out, const Tuple &t, int_<Pos> ) {
            print( out, std::get< std::tuple_size<Tuple>::value-Pos >(t) );
            out << ", ";
            tuple( out, t, int_<Pos-1>() );
        }

        template <class Tuple>
        static void tuple( std::ostream &out, const Tuple &t, int_<1> ) {
            print( out, std::get<std::tuple_size<Tuple>::value-1>(t) );
        }

        // generic printers

        template <class... Args>
        static void
        print( std::ostream &out, const std::tuple<Args...> &t ) {
            out << "[ ";
            tuple( out, t, int_<sizeof...(Args)>() );
            out << " ]";
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && !is_associative<T>::value && !is_pair<T>::value >::type
        print( std::ostream &out, T const &t ) {
            bourne::exports::
            basic( out, t );
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && !is_associative<T>::value && is_pair<T>::value >::type
        print( std::ostream &out, T const &t ) {
            out << "{ ";
            print( out, t.first );
            out << " : ";
            print( out, t.second );
            out << " }";
        }

        template <typename T>
        static typename std::enable_if< !is_sequential<T>::value && is_associative<T>::value && !is_pair<T>::value >::type
        print( std::ostream &out, T const &t ) {
            std::string sep;
            out << "{ ";
            for( const auto &it : t ) {
                out << sep;
                print( out, it.first );
                out << " : ";
                print( out, it.second );
                sep = ", ";
            }
            out << " }";
        }

        template <typename T>
        static typename std::enable_if< is_sequential<T>::value && !is_associative<T>::value && !is_pair<T>::value >::type
        print( std::ostream &out, T const &t ) {
            std::string sep;
            out << "[ ";
            for( const auto &it : t ) {
                out << sep;
                print( out, it );
                sep = ", ";
            }
            out << " ]";
        }
    };

    // API

    template <typename T>
    bool to_json( std::ostream &os, const T &t ) {
        std::stringstream ss;
        return !os.fail() && ( printer::print( ss, t ), !ss.str().empty() ) ? ( os << ss.rdbuf(), true ) : false;
    }

    template <typename T>
    bool to_json( std::string &dst, const T &t ) {
        std::stringstream ss;
        return ( printer::print( ss, t ), t = ss.str(), !t.empty() ) ? true : false;
    }

    template <typename T>
    bool from_json( T &t, std::istream &is ) {
        parser::parse( is, t );
        return !is.fail() ? true : ( t = T {}, true /* ahem O:) */ );
    }

    template <typename T>
    bool from_json( T &t, const std::string &s ) {
        std::stringstream ss;
        return ss << s ? from_json( t, ss ) : false;
    }

    // syntax sugars

    template <typename T>
    std::string to_json( const T &t ) {
        std::stringstream ss;
        return to_json( ss, t ) ? ss.str() : std::string();
    }

    template <typename T>
    T from_json( std::istream &is ) {
        T t;
        return from_json( t, is ) ? t : T {};
    }

    template <typename T>
    T from_json( const std::string &s ) {
        T t;
        return from_json( t, s ) ? t : T {};
    }
}

// optional custom-class autoserialization macro
#define BOURNE_LOAD( OBJECT, PARGS ) \
namespace bourne { namespace imports { \
    template<> inline void basic( std::istream &is, OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        bourne::from_json( _tpl, is ); \
        std::tie PARGS = _tpl; \
    } \
} }

#define BOURNE_SAVE( OBJECT, PARGS ) \
namespace bourne { namespace exports { \
    template<> inline void basic( std::ostream &os, const OBJECT ) { \
        auto _tpl = std::make_tuple PARGS; \
        bourne::to_json( os, _tpl ); \
    } \
} }

#define BOURNE_DEFINE( OBJECT, PARGS ) \
    BOURNE_LOAD( OBJECT, PARGS ) \
    BOURNE_SAVE( OBJECT, PARGS )
