medea
=====

- Medea is a lightweight and tiny serializer. In mythology, also Jason's wife.
- First-class classes and most STL containers are automatically serialized.
- Support for JSON and MEDEA serialization formats.
- Cross-platform. Builds on Windows/Linux/MacosX. Compiles on g++/clang/msvc.
- OS dependencies only. No third party dependencies.
- Tiny. Header only.
- MIT licensed.

sample
======

```c++
#include "medea.hpp"

int main() {
    std::unordered_map< std::string, std::vector< std::string > > contacts = {
        { "homer",  {"marge",  "lisa",  "bart", "maggie" } },
        { "marge",  {"homer",  "lisa",  "bart", "maggie" } },
        { "lisa",   {"marge", "homer",  "bart", "maggie" } },
        { "bart",   {"marge",  "lisa", "homer", "maggie" } },
        { "maggie", {"marge",  "lisa",  "bart",  "homer" } }
    };
    // first-class classes are automatically serialized
    std::string json = medea::to_json( contacts );
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
#include "medea.hpp"

struct phones {
    std::string country;
    std::vector<int> phonelist;
    std::string details;
};

MEDEA_DEFINE( phones &it, (it.country, it.phonelist, it.details) );

int main() {
    phones ph = { "spain", {123,456}, "" };
    std::string json = medea::to_json( ph );
    std::cout << json << std::endl;
}
```

spec
====

- Medea serialization draft is *not* public yet.
- Medea spec should be more uniform than JSON and should support binary data as well.

```c++
    bool: true
    bool: false
    string: "quoted ""string"""
    character: "a" // string of one
    binary: stringof( base91x(data...) )
    number: double
    number: uint64
    number: int64

    keyvalues: value @ key [, value @ key [ , ... ] ]
    values: value [@ null] [, value [ @ nulll ] [ , ... ] ]

    [] sequence
    {} unsorted keymap
    <> sorted keymap
```
