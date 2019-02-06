#ifndef RFC5646_HH_INCLUDED_20190201
#define RFC5646_HH_INCLUDED_20190201

// -- Tags for Identifying Languages (RFC 5646) Parser Helpers. ----------------
// [RFC 5646](https://tools.ietf.org/html/rfc5646)

#include "parser_helpers.hh"

inline namespace rfc5646 {

optional<string> read_language_tag(std::istream &is);

}


#endif //RFC5646_HH_INCLUDED_20190201
