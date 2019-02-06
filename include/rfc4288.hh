#ifndef RFC4288_HH_INCLUDED_20190201
#define RFC4288_HH_INCLUDED_20190201

// -- Media Type (RFC 4288) Parser Helpers. ------------------------------------
// [RFC 4288](https://tools.ietf.org/html/rfc4288)

#include "parser_helpers.hh"

inline namespace rfc4288 {

optional<string> read_type_name(std::istream &is);
optional<string> read_subtype_name(std::istream &is);
optional<string> read_reg_name(std::istream &is);
optional<string> read_reg_name_char(std::istream &is);

}

#endif //RFC4288_HH_INCLUDED_20190201
