Bourne <a href="https://travis-ci.org/r-lyeh/bourne"><img src="https://api.travis-ci.org/r-lyeh/bourne.svg?branch=master" align="right" /></a>
======

- Bourne is a lightweight JSON de/serializer (C++11).
- Bourne is handy. First-class objects and most STL containers are auto serialized (SFINAE).
- Bourne is cross-platform. Builds on Windows/Linux/MacosX. Compiles on g++/clang/msvc.
- Bourne is tiny, self-contained, header-only.
- Bourne is zlib/libpng licensed.

## Current STL status

| standard template | Supported? |
| :-----------------|:----------:|
| std::array        | no         |
| std::bitset       | no         |
| std::chrono       | no         |
| std::complex      | no         |
| std::deque        | yes        |
| std::forward_list | no         |
| std::list         | yes        |
| std::map          | yes        |
| std::queue        | no         |
| std::set          | yes        |
| std::stack        | no         |
| std::string       | yes        |
| std::tuple        | yes        |
| std::unordered_map | yes       |
| std::unordered_set | yes       |
| std::vector       | yes        |
| std::wstring      | no         |

## Quick tutorial
```c++
namespace bourne {
    // save, serialization
    bool to_json( std::string &t, const T &t );
    bool to_json( std::ostream &os, const T &t );
    // load, deserialization
    bool from_json( T &t, std::istream &is );
    bool from_json( T &t, const std::string &s );
    // syntax sugars
    std::string to_json( const T &t );
    T from_json( std::istream &is );
    T from_json( const std::string &str );
}
```

## Notes
- Bourne uses a relaxed JSON spec that allows de/serialization of associative keys of any kind.
- Therefore, if you plan to produce compliant JSON files then use strings as keys types in associative containers.

## Sample
```c++
#include "bourne.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

int main() {
    // first-class objects are automatically serialized
    std::unordered_map< std::string, std::vector< std::string > > contacts = {
        { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
        { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
        { "lisa",   {"marge", "homer",  "bart", "maggie" } },
        { "bart",   {"marge",  "lisa", "homer", "maggie" } },
        { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
    }, built;
    std::cout << bourne::to_json( contacts ) << std::endl;

    // saving/loading showcase
    std::string json = bourne::to_json( contacts );
    bourne::from_json( built, json );
    assert( built == contacts );

    // custom objects require a thin BOURNE_DEFINE() wrapper
    phones list = { "uk", {123,456}, "just a few contacts" };
    std::cout << bourne::to_json( list ) << std::endl;
}
```

## Possible output
```json
{
    "homer":["marge","lisa","bart","maggie"],
    "marge":["homer","lisa","bart","maggie"],
    "bart":["marge","lisa","homer","maggie"],
    "lisa":["marge","homer","bart","maggie"],
    "maggie":["marge","lisa","bart","homer"]
}
[ "uk", [ 123, 456 ], "just a few contacts" ]
```

## Changelog
- v1.0.1 (2015/06/01)
  - Allow helper macros in header files
- v1.0.0 (2015/02/25)
  - SFINAE version
- v0.0.0 (2013/07/15)
  - Initial commit
