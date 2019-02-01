#include "rfc5234.hh"

inline namespace rfc5234 {

optional <string> read_htab(std::istream &is) {
        CALLSTACK;
        // HTAB = %x09
        return read_token(is, "\t");
}

optional <string> read_sp(std::istream &is) {
        CALLSTACK;
        // SP = %x20
        return read_token(is, " ");
}

optional <string> read_wsp(std::istream &is) {
        CALLSTACK;
        // WSP = SP / HTAB ; white space
        if (auto val = read_sp(is))
                return val;
        if (auto val = read_htab(is))
                return val;
        return nullopt;
}

}
