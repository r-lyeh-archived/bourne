medea
=====

Medea is a lightweight and tiny serializer for data interchange written in C++11

Medea draft is *not* public yet

spec
====
```
    // i believe this scheme is self-contained

    bool: true
    bool: false
    string: "quoted ""string"""
    character: "a" // string of one
    binary: stringof( base91x(data...) )
    number: double
    number: uint64
    number: int64

    value @ key
    value [@ null]

    [] sequence
    {} unsorted keymap
    <> sorted keymap

    // types
    typedef std::string             string;
    typedef std::string::value_type character;

    typedef std::nullptr_t null;
    typedef bool           boolean;
    typedef float          float32;
    typedef double         float64;

    typedef std::int8_t    int8;
    typedef std::int16_t   int16;
    typedef std::int32_t   int32;
    typedef std::int64_t   int64;

    typedef std::uint8_t   uint8;
    typedef std::uint16_t  uint16;
    typedef std::uint32_t  uint32;
    typedef std::uint64_t  uint64;

    std::string quote( const medea::string &t ) {
        std::string out;
        for( auto &it : t )
            if( it != '\"' )
                out += it;
            else
                out += "\"\"";
        return std::string() + '\"' + out + '\"';
    }

    // both sequence and associative containers
    template<typename container>
    std::string to_medea( const container &t ) {
        std::stringstream ss;
        for( const auto &it : t )
            ss << medea::to_json(it) << ',';
        const char in = '[', out = ']';
        std::string text = ss.str();
        return std::string() + in + ( text.size() ? ( text.back() = out, text ) : text + out );
    }

    // pairs for associative containers
    template<typename K, typename V>
    std::string to_medea( const std::pair<K,V> &t ) {
        std::stringstream ss;
        ss << medea::to_json(t.second) << "@" << medea::to_json(t.first);
        return ss.str();
    }
```
