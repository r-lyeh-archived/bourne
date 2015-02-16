Bourne
======

- Bourne is a lightweight JSON serializer (C++11).
- Bourne is handy. First-class classes and most STL containers are automatically serialized.
- Bourne is cross-platform. Builds on Windows/Linux/MacosX. Compiles on g++/clang/msvc.
- Bourne is self-contained. Standard dependencies only.
- Bourne is tiny. Header only.
- Bourne is BOOST licensed.

sample
======

```c++
#include "bourne.hpp"

int main() {
    std::unordered_map< std::string, std::vector< std::string > > contacts = {
        { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
        { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
        { "lisa",   {"marge", "homer",  "bart", "maggie" } },
        { "bart",   {"marge",  "lisa", "homer", "maggie" } },
        { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
    };
    // first-class classes are automatically serialized
    std::string json = bourne::to_json( contacts );
    std::cout << json << std::endl;
}
```

possible output
===============

```json
{
    "homer":["marge","lisa","bart","maggie"],
    "marge":["homer","lisa","bart","maggie"],
    "bart":["marge","lisa","homer","maggie"],
    "lisa":["marge","homer","bart","maggie"],
    "maggie":["marge","lisa","bart","homer"]
}
```

cons
====

- Custom classes require a thin wrapper. Ie,

```c++
#include "bourne.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

BOURNE_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

int main() {
    phones ph = { "uk", {123,456}, "another contact" };
    std::string json = bourne::to_json( ph );
    std::cout << json << std::endl;
}
```
