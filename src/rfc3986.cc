#include "rfc3986.hh"
#include "parser_helpers.hh"
#include <istream>

// -- URI (RFC 3986) Parser Helpers. ------------------------------------

inline namespace rfc3986 {
//      gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
optional<string> read_gen_delims(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch (i) {
        default:
        case EOF:
                return nullopt;
        case ':':
        case '/':
        case '?':
        case '#':
        case '[':
        case ']':
        case '@':
                break;
        }
        ptran.commit();
        return string() + char(i);
}

//      sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
optional<string> read_sub_delims(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch (i) {
        default:
        case EOF:
                return nullopt;
        case '!':
        case '$':
        case '&':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
        case ';':
        case '=':
                break;
        }
        ptran.commit();
        return string() + char(i);
}

//      reserved    = gen-delims / sub-delims
optional<string> read_reserved(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_gen_delims(is)) {
                ret = *v;
        } else if (auto v = read_sub_delims(is)) {
                ret = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//      unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
optional<string> read_unreserved(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_alpha(is)) ret = *v;
        else if (auto v = read_digit(is)) ret = *v;
        else if (auto v = read_token(is, "-")) ret = *v;
        else if (auto v = read_token(is, ".")) ret = *v;
        else if (auto v = read_token(is, "_")) ret = *v;
        else if (auto v = read_token(is, "~")) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

// pct-encoded = "%" HEXDIG HEXDIG
optional<string> read_pct_encoded(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_token(is, "%")) ret = *v;
        else if (auto v = read_hex(is)) ret = *v;
        else if (auto v = read_hex(is)) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//      pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
optional<string> read_pchar(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_unreserved(is)) ret = *v;
        else if (auto v = read_pct_encoded(is)) ret = *v;
        else if (auto v = read_sub_delims(is)) ret = *v;
        else if (auto v = read_token(is, ":")) ret = *v;
        else if (auto v = read_token(is, "@")) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//      segment       = *pchar
optional<string> read_segment(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        while (auto c = read_pchar(is)) {
                ret += *c;
        }
        ptran.commit();
        return ret;
}

//      segment-nz    = 1*pchar
optional<string> read_segment_nz(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_pchar(is)) {
                ret += *v;
        } else {
                return nullopt;
        }
        while (auto v = read_pchar(is)) {
                ret += *v;
        }
        ptran.commit();
        return ret;
}

//      segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//                    ; non-zero-length segment without any colon ":"
optional<string> read_segment_nz_nc_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_unreserved(is)) ret = *v;
        else if (auto v = read_pct_encoded(is)) ret = *v;
        else if (auto v = read_sub_delims(is)) ret = *v;
        else if (auto v = read_token(is, "@")) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

optional<string> read_segment_nz_nc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_segment_nz_nc_single(is)) {
                ret += *v;
        } else {
                return nullopt;
        }
        while (auto v = read_segment_nz_nc_single(is)) {
                ret += *v;
        }
        ptran.commit();
        return ret;
}

//      path-abempty  = *( "/" segment )
optional<string> read_path_abempty(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        while (auto sep = read_token(is, "/")) {
                ret += *sep;
                if (auto v = read_segment(is)) {
                        ret += *v;
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      path-absolute = "/" [ segment-nz *( "/" segment ) ]
optional<string> read_path_absolute(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto sep = read_token(is, "/")) {
                ret += *sep;
        } else {
                return nullopt;
        }
        // [ segment-nz *( "/" segment ) ]
        if (auto segnz = read_segment_nz(is)) {
                ret += *segnz;
                // *( "/" segment ) ]
                while (auto sep = read_token(is, "/")) {
                        ret += *sep;
                        if (auto seg = read_segment(is)) {
                                ret += *seg;
                        } else {
                                return nullopt;
                        }
                }
        }
        ptran.commit();
        return ret;
}

//      path-noscheme = segment-nz-nc *( "/" segment )
optional<string> read_path_noscheme(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto segnznc = read_segment_nz_nc(is)) {
                ret += *segnznc;
        } else {
                return nullopt;
        }
        while (auto sep = read_token(is, "/")) {
                ret += *sep;
                if (auto seg = read_segment(is)) {
                        ret += *seg;
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      path-rootless = segment-nz *( "/" segment )
optional<string> read_path_rootless(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto segnz = read_segment_nz(is)) {
                ret += *segnz;
        } else {
                return nullopt;
        }
        while (auto sep = read_token(is, "/")) {
                ret += *sep;
                if (auto seg = read_segment(is)) {
                        ret += *seg;
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      path-empty    = 0<pchar>
optional<string> read_path_empty(std::istream &is) {
        CALLSTACK;
        return "";
}

//      path          = path-abempty    ; begins with "/" or is empty
//                    / path-absolute   ; begins with "/" but not "//"
//                    / path-noscheme   ; begins with a non-colon segment
//                    / path-rootless   ; begins with a segment
//                    / path-empty      ; zero characters
optional<string> read_path(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;
        if (auto v = read_path_abempty(is)) ret = *v;
        else if (auto v = read_path_absolute(is)) ret = *v;
        else if (auto v = read_path_noscheme(is)) ret = *v;
        else if (auto v = read_path_rootless(is)) ret = *v;
        else if (auto v = read_path_empty(is)) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//      port        = *DIGIT
optional<string> read_port(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      reg-name    = *( unreserved / pct-encoded / sub-delims )
optional<string> read_reg_name(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;

        while(true) {
                if (auto v = read_unreserved(is)) ret += *v;
                else if (auto v = read_pct_encoded(is)) ret += *v;
                else if (auto v = read_sub_delims(is)) ret += *v;
                else break;
        }

        ptran.commit();
        return ret;
}

//      IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
optional<string> read_IPv4address(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;

        if (auto v = read_dec_octet(is)) ret += *v;
        else return nullopt;

        if (auto v = read_token(is, ".")) ret += *v;
        else return nullopt;

        if (auto v = read_dec_octet(is)) ret += *v;
        else return nullopt;

        if (auto v = read_token(is, ".")) ret += *v;
        else return nullopt;

        if (auto v = read_dec_octet(is)) ret += *v;
        else return nullopt;

        if (auto v = read_token(is, ".")) ret += *v;
        else return nullopt;

        if (auto v = read_dec_octet(is)) ret += *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//      dec-octet   = DIGIT                 ; 0-9
//                  / %x31-39 DIGIT         ; 10-99
//                  / "1" 2DIGIT            ; 100-199
//                  / "2" %x30-34 DIGIT     ; 200-249
//                  / "25" %x30-35          ; 250-255
optional<string> read_dec_octet(std::istream &is) {
        CALLSTACK;

        {
                // "25" %x30-35          ; 250-255
                save_input_pos ptran(is);
                const auto a = read_digit(is, 2, 2);
                const auto b = read_digit(is, 5, 5);
                const auto c = read_digit(is, 0, 5);
                if (a && b && c) {
                        ptran.commit();
                        return *a + *b + *c;
                }
        }
        {
                // "2" %x30-34 DIGIT     ; 200-249
                save_input_pos ptran(is);
                const auto a = read_digit(is, 2, 2);
                const auto b = read_digit(is, 0, 4);
                const auto c = read_digit(is);
                if (a && b && c) {
                        ptran.commit();
                        return *a + *b + *c;
                }
        }
        {
                // "1" 2DIGIT            ; 100-199
                save_input_pos ptran(is);
                const auto a = read_digit(is, 1, 1);
                const auto b = read_digit(is);
                const auto c = read_digit(is);
                if (a && b && c) {
                        ptran.commit();
                        return *a + *b + *c;
                }
        }
        {
                // %x31-39 DIGIT         ; 10-99
                save_input_pos ptran(is);
                const auto a = read_digit(is, 1, 9);
                const auto b = read_digit(is);
                if (a && b) {
                        ptran.commit();
                        return *a + *b;
                }
        }
        {
                // DIGIT                 ; 0-9
                save_input_pos ptran(is);
                const auto a = read_digit(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        return nullopt;
}

//      IPv6address =                            6( h16 ":" ) ls32
//                  /                       "::" 5( h16 ":" ) ls32
//                  / [               h16 ] "::" 4( h16 ":" ) ls32
//                  / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
//                  / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
//                  / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
//                  / [ *4( h16 ":" ) h16 ] "::"              ls32
//                  / [ *5( h16 ":" ) h16 ] "::"              h16
//                  / [ *6( h16 ":" ) h16 ] "::"
optional<string> read_IPv6address(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//
//      ls32        = ( h16 ":" h16 ) / IPv4address
//                  ; least-significant 32 bits of address
optional<string> read_ls32(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//
//      h16         = 1*4HEXDIG
//                  ; 16 bits of address represented in hexadecimal
optional<string> read_h16(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
optional<string> read_IP_literal(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;

        if (auto v = read_token(is, "[")) {
                ret += *v;
        } else {
                return nullopt;
        }

        if (auto v = read_IPv6address(is)) {
                ret += *v;
        } else if (auto v = read_IPvFuture(is)) {
                ret += *v;
        } else {
                return nullopt;
        }

        if (auto v = read_token(is, "]")) {
                ret += *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//      IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
optional<string> read_IPvFuture(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      host        = IP-literal / IPv4address / reg-name
optional<string> read_host(std::istream &is) {
        CALLSTACK;
        if (auto v = read_IP_literal(is)) return v;
        if (auto v = read_IPv4address(is)) return v;
        if (auto v = read_reg_name(is)) return v;
        return nullopt;
}

//      userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
optional<string> read_userinfo(std::istream &is) {
        CALLSTACK;
        string ret;
        while(true) {
                if (auto v = read_unreserved(is)) ret += *v;
                else if (auto v = read_pct_encoded(is)) ret += *v;
                else if (auto v = read_sub_delims(is)) ret += *v;
                else if (auto v = read_token(is, ":")) ret += *v;
                else break;
        }
        return ret;
}

//      scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
optional<string> read_scheme(std::istream &is) {
        save_input_pos ptran(is);
        string ret;

        if (auto v = read_alpha(is)) {
                ret += *v;
        } else {
                return nullopt;
        }

        while (true) {
                if (auto v = read_alpha(is)) ret += *v;
                else if (auto v = read_digit(is)) ret += *v;
                else if (auto v = read_token(is, "+")) ret += *v;
                else if (auto v = read_token(is, "-")) ret += *v;
                else if (auto v = read_token(is, ".")) ret += *v;
                else break;
        }

        ptran.commit();
        return ret;
}

//      authority   = [ userinfo "@" ] host [ ":" port ]
optional<Authority> read_authority(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Authority ret;

        if (auto v = read_userinfo(is)) {
                ret.userinfo = v;
                if (!read_token(is, "@"))
                        return nullopt;
        }

        if (auto v = read_host(is)) {
                ret.host = *v;
        } else {
                return nullopt;
        }

        if (read_token(is, ":")) {
                if (auto v = read_port(is)) {
                        ret.port = v;
                } else {
                        return nullopt;
                }
        }

        ptran.commit();
        return ret;
}

//      relative-part = "//" authority path-abempty
//                    / path-absolute
//                    / path-noscheme
//                    / path-empty
optional<string> read_relative_part(std::istream &is) {
        CALLSTACK;
        {
                save_input_pos ptran(is);
                const auto a = read_token(is, "//");
                const auto b = read_authority(is);
                const auto c = read_path_abempty(is);
                if (a && b && c) {
                        ptran.commit();
                        return *a + to_string(*b) + *c;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_absolute(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_noscheme(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_empty(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        return nullopt;
}

//      URI-reference = URI / relative-ref
optional<string> read_URI_reference(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      fragment    = *( pchar / "/" / "?" )
optional<string> read_fragment(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      query       = *( pchar / "/" / "?" )
optional<string> read_query(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      hier-part   = "//" authority path-abempty
//                  / path-absolute
//                  / path-rootless
//                  / path-empty
optional<string> read_hier_part(std::istream &is) {
        CALLSTACK;
        {
                save_input_pos ptran(is);
                const auto a = read_token(is, "//");
                const auto b = read_authority(is);
                const auto c = read_path_abempty(is);
                if (a && b && c) {
                        ptran.commit();
                        return *a + to_string(*b) + *c;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_absolute(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_rootless(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        {
                save_input_pos ptran(is);
                const auto a = read_path_empty(is);
                if (a) {
                        ptran.commit();
                        return *a;
                }
        }
        return nullopt;
}

//      URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
optional<Uri> read_URI(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Uri ret;

        if (auto v = read_scheme(is)) {
                ret.scheme = *v;
        } else {
                return nullopt;
        }

        if (!read_token(is, ":"))
                return nullopt;

        if (auto v = read_hier_part(is)) {
                ret.hier_part = *v;
        } else {
                return nullopt;
        }

        if (read_token(is, "?")) {
                ret.query = read_query(is);
                if (!ret.query)
                        return nullopt;
        }

        if (read_token(is, "#")) {
                ret.fragment = read_fragment(is);
                if (!ret.fragment)
                        return nullopt;
        }

        ptran.commit();
        return ret;
}

//      absolute-URI  = scheme ":" hier-part [ "?" query ]
optional<string> read_absolute_URI(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//      relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
optional<string> read_relative_ref(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}


std::ostream& operator<<(std::ostream& os, Uri const &v) {
        return os << to_string(v);
}

}

