#ifndef RFC5234_HH_INCLUDED_20190201
#define RFC5234_HH_INCLUDED_20190201

// -- ABNF (RFC 5234) Parser Helpers. ------------------------------------------
// [RFC 5234](https://tools.ietf.org/html/rfc5234)

#include "parser_helpers.hh"

inline namespace rfc5234 {

optional <string> read_htab(std::istream &is);
optional <string> read_sp(std::istream &is);
optional <string> read_wsp(std::istream &is);

}

#endif //RFC5234_HH_INCLUDED_20190201
