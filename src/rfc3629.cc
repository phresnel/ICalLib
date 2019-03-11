#include "rfc3629.hh"
#include "parser_helpers.hh"
#include <iostream>

// -- Support for RFC 3629: UTF-8, a transformation format of ISO 10646. -------

inline namespace rfc3629 {

//   UTF8-octets = *( UTF8-char )
//   UTF8-char   = UTF8-1 / UTF8-2 / UTF8-3 / UTF8-4
//   UTF8-1      = %x00-7F


//   UTF8-tail   = %x80-BF
bool is_utf8_tail(int c) {
        return (c >= 0x80) && (c <= 0xBF);
}
optional<string> read_utf8_tail(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        if (is_utf8_tail(i)) {
                ptran.commit();
                return string() + char(i);
        }
        return nullopt;
}

//   UTF8-2      = %xC2-DF UTF8-tail
optional<string> read_utf8_2(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto a = is.get();
        if (a == EOF) return nullopt;

        const auto b = is.get();
        if (b == EOF) return nullopt;

        const auto match = ((a>=0xC2) && (a<=0xDF)) &&
                           is_utf8_tail(b);
        if (!match)
                return nullopt;

        ptran.commit();
        return string() + char(a) + char(b);
}

//   UTF8-3      = %xE0 %xA0-BF UTF8-tail /
//                 %xE1-EC 2( UTF8-tail ) /
//                 %xED %x80-9F UTF8-tail /
//                 %xEE-EF 2( UTF8-tail )
optional<string> read_utf8_3(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto a = is.get();
        if (a == EOF) return nullopt;

        const auto b = is.get();
        if (b == EOF) return nullopt;

        const auto c = is.get();
        if (c == EOF) return nullopt;

        const auto A = (a==0xE0) &&
                       ((b>=0xA0) && (b<=0xBF)) &&
                       is_utf8_tail(c);
        const auto B = ((a>=0xE1) && (a<=0xEC)) &&
                       is_utf8_tail(b) &&
                       is_utf8_tail(c);
        const auto C = (a==0xED) &&
                       ((b>=0x80) && (b<=0x9F)) &&
                       is_utf8_tail(c);
        const auto D = ((a>=0xEE) && (a<=0xEF)) &&
                       is_utf8_tail(b) &&
                       is_utf8_tail(c);

        const auto match = A || B || C || D;

        if (!match)
                return nullopt;

        ptran.commit();
        return string() + char(a) + char(b) + char(c);
}

//   UTF8-4      = %xF0 %x90-BF 2( UTF8-tail ) /
//                 %xF1-F3 3( UTF8-tail ) /
//                 %xF4 %x80-8F 2( UTF8-tail )
optional<string> read_utf8_4(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto a = is.get();
        if (a == EOF) return nullopt;

        const auto b = is.get();
        if (b == EOF) return nullopt;

        const auto c = is.get();
        if (c == EOF) return nullopt;

        const auto d = is.get();
        if (d == EOF) return nullopt;

        const auto A = (a==0xF0) &&
                       ((b>=0x90) && (b<=0xBF)) &&
                       is_utf8_tail(c) &&
                       is_utf8_tail(d);
        const auto B = ((a>=0xF1) && (a<=0xF3)) &&
                       is_utf8_tail(b) &&
                       is_utf8_tail(c) &&
                       is_utf8_tail(d);
        const auto C = (a==0xF4) &&
                       ((b>=0x80) && (b<=0x8F)) &&
                       is_utf8_tail(c) &&
                       is_utf8_tail(d);

        const auto match = A || B || C;

        if (!match)
                return nullopt;

        ptran.commit();
        return string() + char(a) + char(b) + char(c) + char(d);
}

}
