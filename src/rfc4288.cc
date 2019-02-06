#include "rfc4288.hh"

// -- Media Type (RFC 4288) Parser Helpers. ------------------------------------
inline namespace rfc4288 {

//       type-name = reg-name
optional<string> read_type_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       subtype-name = reg-name
optional<string> read_subtype_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name = 1*127reg-name-chars
optional<string> read_reg_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name-chars = ALPHA / DIGIT / "!" /
//                       "#" / "$" / "&" / "." /
//                       "+" / "-" / "^" / "_"
optional<string> read_reg_name_char(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

}
