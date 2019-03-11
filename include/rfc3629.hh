#ifndef RFC3629_HH_INCLUDED_20190311
#define RFC3629_HH_INCLUDED_20190311

// -- Support for RFC 3629: UTF-8, a transformation format of ISO 10646. -------
// * [RFC 3629](https://tools.ietf.org/html/rfc3629)

#include <iosfwd>
#include <string>
#include <optional>

inline namespace rfc3629 {

using std::string;
using std::optional;
using std::nullopt;

optional<string> read_utf8_2(std::istream &is);
optional<string> read_utf8_3(std::istream &is);
optional<string> read_utf8_4(std::istream &is);

optional<string> read_utf8_tail(std::istream &is);

}

#endif //RFC3629_HH_INCLUDED_20190311
