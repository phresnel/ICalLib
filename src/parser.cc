#include "parser.hh"

#include <istream>
#include <string>
#include <vector>
#include <optional>
#include <iostream>

class CallStack {
public:
        class Entry {
        public:
                Entry() = delete;
                Entry(Entry const &) = delete;
                Entry& operator= (Entry const &) = delete;

                Entry(Entry &&other) {
                        entries_ = other.entries_;
                        other.entries_ = nullptr;
                }
                Entry& operator= (Entry &&other) {
                        entries_ = other.entries_;
                        other.entries_ = nullptr;
                }

                Entry(std::vector<const char*> &entries, const char *what)
                        : entries_(&entries)
                {
                        entries_->push_back(what);

                        std::cerr << "  ";
                        for (auto i=0; i<entries_->size(); ++i)
                                std::cerr << "  ";
                        std::cerr << "entered " << what << "\n";
                }
                ~Entry() {
                        if (entries_)
                                entries_->pop_back();
                }
        private:
                friend class CallStack;
                std::vector<const char*> *entries_;
        };

        CallStack() = default;
        CallStack(CallStack const&) = default;
        CallStack& operator= (CallStack const&) = default;

        Entry push(const char* name) {
                Entry ret {entries_, name};
                //std::cerr << *this;
                return std::move(ret);
        }

        friend std::ostream& operator<< (
                std::ostream& os, CallStack const &cs
        ) {
                os << "CallStack{\n";
                for (auto i=0; i!=cs.entries_.size(); ++i) {
                        /*os << " ..";
                        for (auto x=0; x!=i; ++x) {
                                os << "..";
                        }*/
                        os << "  " << cs.entries_[i] << "\n";
                }
                os << "}\n";
                return os;
        }
private:

        std::vector<const char*> entries_;
};

//CallStack callStack;
//#define CALLSTACK CallStack::Entry call_stack_entry_##__LINE__ = callStack.push(__func__);
//*/
#define CALLSTACK

#define NOT_IMPLEMENTED do{throw not_implemented(is.tellg(), __func__);}while(false)

void print_location(std::istream::pos_type pos, std::istream &is) {
        const auto where = pos;
        is.seekg(0);
        int line = 1, col = 1;
        while(is.tellg() < where) {
                if(read_newline(is)) {
                        col = 1;
                        ++line;
                } else {
                        auto g = is.get();
                        //std::cout << "[" << (char)g << "]";
                }
        }
        std::cerr << " (in line " << line << ":" << col << ")\n";
}

// -- Utils. -------------------------------------------------------------------
class save_flags final {
        std::ios *s_;
        std::ios::fmtflags flags_;
public:
        explicit save_flags(std::ios &s) : s_(&s), flags_(s.flags()) { }
        ~save_flags() {
                if (s_ != nullptr) {
                        try {
                                s_->flags(flags_);
                        } catch(...) {
                                // must not throw here.
                        }
                }
        }

        save_flags(save_flags &&) = default;
        save_flags& operator= (save_flags &&) = default;

        save_flags() = delete;
        save_flags(save_flags const &) = delete;
        save_flags& operator= (save_flags const &) = delete;

        void commit() { s_ = nullptr; }
};

class save_input_pos final {
        std::istream *s_;
        std::istream::pos_type pos_;
public:
        explicit save_input_pos(std::istream &s) : s_(&s), pos_(s.tellg()) { }
        ~save_input_pos() {
                if (s_ != nullptr) {
                        try { s_->seekg(pos_); }
                        catch(...) { /* must not throw here. */ }
                }
        }

        save_input_pos(save_input_pos &&) = default;
        save_input_pos& operator= (save_input_pos &&) = default;

        save_input_pos() = delete;
        save_input_pos(save_input_pos const &) = delete;
        save_input_pos& operator= (save_input_pos const &) = delete;

        void commit() { s_ = nullptr; }
};

// -- Parser Helpers. ----------------------------------------------------------
void expect_token(std::istream &is, string const &tok) {
        CALLSTACK;
        save_input_pos ptran(is);
        for (auto &c : tok) {
                const auto i = is.get();
                if(i == EOF) {
                        throw unexpected_token(is.tellg(), tok);
                }
                if(i != c) {
                        throw unexpected_token(is.tellg(), tok);
                }
        }
        ptran.commit();
}

bool read_token(std::istream &is, string const &tok) {
        CALLSTACK;
        try {
                expect_token(is, tok);
                return true;
        } catch(syntax_error &) {
                return false;
        }
}

bool read_eof(std::istream &is) {
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != EOF)
                return false;
        ptran.commit();
        return true;
}

void expect_newline(std::istream &is) {
        CALLSTACK;
        // Even though RFC 5545 says just "CRLF", we also handle "CR" and "LF".
        if (!(read_token(is, "\r\n") ||
              read_token(is, "\n") ||
              read_token(is, "\r") ||
              read_eof(is))
                ) {
                throw unexpected_token(is.tellg());
        }
}

bool read_newline(std::istream &is) {
        CALLSTACK;
        try {
                expect_newline(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

bool read_hex(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch(i) {
        default:
        case EOF:
                return false;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case 'a':
        case 'A':
        case 'b':
        case 'B':
        case 'c':
        case 'C':
        case 'd':
        case 'D':
        case 'e':
        case 'E':
        case 'f':
        case 'F':
                break;
        }

        ptran.commit();
        return true;
}

void expect_alpha(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch(i) {
        default:
        case EOF:
                throw syntax_error(is.tellg());
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
                break;
        }
        ptran.commit();
}

bool read_alpha(std::istream &is) {
        CALLSTACK;
        try {
                expect_alpha(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}


void expect_digit(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        if (i<'0' || i>'9')
                throw syntax_error(is.tellg(), "expected digit");
        ptran.commit();
}

bool read_digit(std::istream &is) {
        CALLSTACK;
        try {
                expect_digit(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

bool read_digits(std::istream &is, int at_least, int at_most = -1) {
        CALLSTACK;
        save_input_pos ptran(is);
        int c = 0;
        while (read_digit(is) && (at_most<0 || c<at_most)) {
                ++c;
        }

        if (at_least >= 0 && c<at_least)
                return false;
        if (at_most >= 0 && c>at_most)
                return false;

        ptran.commit();
        return true;
}

void expect_alnum(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_alpha(is) || read_digit(is);
        if (!success) {
                throw syntax_error(is.tellg(), "expected alpha or digit");
        }
        ptran.commit();
}

bool read_alnum(std::istream &is) {
        CALLSTACK;
        try {
                expect_alnum(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}


// -- URI (RFC 3986) Parser Helpers. ------------------------------------

inline namespace rfc3986 {
//      gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
bool read_gen_delims(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch (i) {
        default:
        case EOF:
                return false;
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
        return true;
}

//      sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
bool read_sub_delims(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch (i) {
        default:
        case EOF:
                return false;
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
        return true;
}

//      reserved    = gen-delims / sub-delims
bool read_reserved(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_gen_delims(is) ||
                read_sub_delims(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
bool read_unreserved(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_alpha(is) ||
                read_digit(is) ||
                read_token(is, "-") ||
                read_token(is, ".") ||
                read_token(is, "_") ||
                read_token(is, "~");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

// pct-encoded = "%" HEXDIG HEXDIG
bool read_pct_encoded(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "%") ||
                read_hex(is) ||
                read_hex(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
bool read_pchar(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_unreserved(is) ||
                read_pct_encoded(is) ||
                read_sub_delims(is) ||
                read_token(is, ":") ||
                read_token(is, "@");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      segment       = *pchar
bool read_segment(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_pchar(is)) {
        }
        ptran.commit();
        return true;
}

//      segment-nz    = 1*pchar
bool read_segment_nz(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_pchar(is))
                return false;
        while (read_pchar(is)) {
        }
        ptran.commit();
        return true;
}

//      segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//                    ; non-zero-length segment without any colon ":"
bool read_segment_nz_nc_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_unreserved(is) ||
                read_pct_encoded(is) ||
                read_sub_delims(is) ||
                read_token(is, "@");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

bool read_segment_nz_nc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_segment_nz_nc_single(is))
                return false;
        while (read_segment_nz_nc_single(is)) {
        }
        ptran.commit();
        return true;
}

//      path-abempty  = *( "/" segment )
bool read_path_abempty(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, "/")) {
                if (!read_segment(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//      path-absolute = "/" [ segment-nz *( "/" segment ) ]
bool read_path_absolute(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_token(is, "/"))
                return false;
        if (read_segment_nz(is)) {
                while (read_token(is, "/")) {
                        if (!read_segment(is))
                                return false;
                }
        }
        ptran.commit();
        return true;
}

//      path-noscheme = segment-nz-nc *( "/" segment )
bool read_path_noscheme(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_segment_nz_nc(is))
                return false;
        while (read_token(is, "/")) {
                if (!read_segment(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//      path-rootless = segment-nz *( "/" segment )
bool read_path_rootless(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_segment_nz(is))
                return false;
        while (read_token(is, "/")) {
                if (!read_segment(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//      path-empty    = 0<pchar>
bool read_path_empty(std::istream &is) {
        CALLSTACK;
        return true;
}

//      path          = path-abempty    ; begins with "/" or is empty
//                    / path-absolute   ; begins with "/" but not "//"
//                    / path-noscheme   ; begins with a non-colon segment
//                    / path-rootless   ; begins with a segment
//                    / path-empty      ; zero characters
bool read_path(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_path_abempty(is) ||
                read_path_absolute(is) ||
                read_path_noscheme(is) ||
                read_path_rootless(is) ||
                read_path_empty(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      port        = *DIGIT
bool read_port(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      reg-name    = *( unreserved / pct-encoded / sub-delims )
bool read_reg_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
bool read_IPv4address(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      dec-octet   = DIGIT                 ; 0-9
//                  / %x31-39 DIGIT         ; 10-99
//                  / "1" 2DIGIT            ; 100-199
//                  / "2" %x30-34 DIGIT     ; 200-249
//                  / "25" %x30-35          ; 250-255
bool read_dec_octet(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
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
bool read_IPv6address(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//
//      ls32        = ( h16 ":" h16 ) / IPv4address
//                  ; least-significant 32 bits of address
bool read_ls32(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//
//      h16         = 1*4HEXDIG
//                  ; 16 bits of address represented in hexadecimal
bool read_h16(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
bool read_IP_literal(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
bool read_IPvFuture(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      host        = IP-literal / IPv4address / reg-name
bool read_host(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
bool read_userinfo(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      authority   = [ userinfo "@" ] host [ ":" port ]
bool read_authority(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      relative-part = "//" authority path-abempty
//                    / path-absolute
//                    / path-noscheme
//                    / path-empty
bool read_relative_part(std::istream &is) {
        CALLSTACK;
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, "//") &&
                        read_authority(is) &&
                        read_path_abempty(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        {
                save_input_pos ptran(is);
                const auto success = read_path_absolute(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        {
                save_input_pos ptran(is);
                const auto success = read_path_noscheme(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        {
                save_input_pos ptran(is);
                const auto success = read_path_empty(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}

//      URI-reference = URI / relative-ref
bool read_URI_reference(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      fragment    = *( pchar / "/" / "?" )
bool read_fragment(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      query       = *( pchar / "/" / "?" )
bool read_query(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      hier-part   = "//" authority path-abempty
//                  / path-absolute
//                  / path-rootless
//                  / path-empty
bool read_hier_part(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
bool read_URI(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      absolute-URI  = scheme ":" hier-part [ "?" query ]
bool read_absolute_URI(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
bool read_relative_ref(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success = false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}
}

// -- Media Type (RFC 4288) Parser Helpers. ------------------------------------
inline namespace rfc4288 {
//       type-name = reg-name
bool read_type_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       subtype-name = reg-name
bool read_subtype_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name = 1*127reg-name-chars
bool read_reg_name(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name-chars = ALPHA / DIGIT / "!" /
//                       "#" / "$" / "&" / "." /
//                       "+" / "-" / "^" / "_"
bool read_reg_name_char(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}
}

// -- Tags for Identifying Languages (RFC 5646) Parser Helpers. ----------------
inline namespace rfc5646 {
// alphanum      = (ALPHA / DIGIT)     ; letters and numbers

// regular       = "art-lojban"        ; these tags match the 'langtag'
//               / "cel-gaulish"       ; production, but their subtags
//               / "no-bok"            ; are not extended language
//               / "no-nyn"            ; or variant subtags: their meaning
//               / "zh-guoyu"          ; is defined by their registration
//               / "zh-hakka"          ; and all of these are deprecated
//               / "zh-min"            ; in favor of a more modern
//               / "zh-min-nan"        ; subtag or sequence of subtags
//               / "zh-xiang"

// irregular     = "en-GB-oed"         ; irregular tags do not match
//               / "i-ami"             ; the 'langtag' production and
//               / "i-bnn"             ; would not otherwise be
//               / "i-default"         ; considered 'well-formed'
//               / "i-enochian"        ; These tags are all valid,
//               / "i-hak"             ; but most are deprecated
//               / "i-klingon"         ; in favor of more modern
//               / "i-lux"             ; subtags or subtag
//               / "i-mingo"           ; combination
//               / "i-navajo"
//               / "i-pwn"
//               / "i-tao"
//               / "i-tay"
//               / "i-tsu"
//               / "sgn-BE-FR"
//               / "sgn-BE-NL"
//               / "sgn-CH-DE"

// grandfathered = irregular           ; non-redundant tags registered
//               / regular             ; during the RFC 3066 era

// privateuse    = "x" 1*("-" (1*8alphanum))

//                                     ; Single alphanumerics
//                                     ; "x" reserved for private use
// singleton     = DIGIT               ; 0 - 9
//               / %x41-57             ; A - W
//               / %x59-5A             ; Y - Z
//               / %x61-77             ; a - w
//               / %x79-7A             ; y - z

// extension     = singleton 1*("-" (2*8alphanum))

// variant       = 5*8alphanum         ; registered variants
//               / (DIGIT 3alphanum)

// region        = 2ALPHA              ; ISO 3166-1 code
//               / 3DIGIT              ; UN M.49 code

// script        = 4ALPHA              ; ISO 15924 code

// extlang       = 3ALPHA              ; selected ISO 639 codes
//                 *2("-" 3ALPHA)      ; permanently reserved

// language      = 2*3ALPHA            ; shortest ISO 639 code
//                 ["-" extlang]       ; sometimes followed by
//                                     ; extended language subtags
//               / 4ALPHA              ; or reserved for future use
//               / 5*8ALPHA            ; or registered language subtag

//  langtag       = language
//                 ["-" script]
//                 ["-" region]
//                 *("-" variant)
//                 *("-" extension)
//                 ["-" privateuse]

//  Language-Tag  = langtag             ; normal language tags
//               / privateuse          ; private use tag
//               / grandfathered       ; grandfathered tags
bool read_language_tag(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}
}

// -- ABNF (RFC 5234) Parser Helpers. ------------------------------------------
inline namespace rfc5234 {
bool read_htab(std::istream &is) {
        CALLSTACK;
        // HTAB = %x09
        return read_token(is, "\t");
}

bool read_sp(std::istream &is) {
        CALLSTACK;
        // SP = %x20
        return read_token(is, " ");
}

bool read_wsp(std::istream &is) {
        CALLSTACK;
        // WSP = SP / HTAB ; white space
        return read_sp(is) || read_htab(is);
}

// -- Parser helpers specific to ICal, but generic within ICal. ----------------
void expect_key_value(std::istream &is, string const &k, string const &v) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, k) &&
                read_token(is, ":") &&
                read_token(is, v) &&
                read_newline(is);
        if (!success)
                throw key_value_pair_expected(is.tellg(), k, v);
        ptran.commit();
}

bool read_key_value(std::istream &is, string const &k, string const &v) {
        CALLSTACK;
        try {
                expect_key_value(is, k, v);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}


//     +------------------------+-------------------+
//     | Character name         | Decimal codepoint |
//     +------------------------+-------------------+
//     | HTAB                   | 9                 |
//     | LF                     | 10                |
//     | CR                     | 13                |
//     | DQUOTE                 | 22                |
//     | SPACE                  | 32                |
//     | PLUS SIGN              | 43                |
//     | COMMA                  | 44                |
//     | HYPHEN-MINUS           | 45                |
//     | PERIOD                 | 46                |
//     | SOLIDUS                | 47                |
//     | COLON                  | 58                |
//     | SEMICOLON              | 59                |
//     | LATIN CAPITAL LETTER N | 78                |
//     | LATIN CAPITAL LETTER T | 84                |
//     | LATIN CAPITAL LETTER X | 88                |
//     | LATIN CAPITAL LETTER Z | 90                |
//     | BACKSLASH              | 92                |
//     | LATIN SMALL LETTER N   | 110               |
//     +------------------------+-------------------+
//
//     ; This ABNF is just a general definition for an initial parsing
//     ; of the content line into its property name, parameter list,
//     ; and value string
//
//     ; When parsing a content line, folded lines MUST first
//     ; be unfolded according to the unfolding procedure
//     ; described above.  When generating a content line, lines
//     ; longer than 75 octets SHOULD be folded according to
//     ; the folding procedure described above.
}

//    contentline   = name *(";" param ) ":" value CRLF
void expect_contentline(std::istream &is) {
        CALLSTACK;
        save_input_pos ts(is);
        expect_name(is);
        while (read_token(is, ";")) {
                expect_param(is);
        }
        expect_token(is, ":");
        expect_value(is);
        expect_newline(is);
        ts.commit();
}
bool read_contentline(std::istream &is) {
        CALLSTACK;
        try {
                expect_contentline(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//     name          = iana-token / x-name
void expect_name(std::istream &is) {
        CALLSTACK;
        const auto success =
                read_iana_token(is) ||
                read_x_name(is);
        if (!success) {
                throw syntax_error(is.tellg(), "expected iana-token or x-name");
        }
}

//     iana-token    = 1*(ALPHA / DIGIT / "-")
//     ; iCalendar identifier registered with IANA
bool read_iana_token_char(std::istream &is) {
        CALLSTACK;
        return read_alnum(is) || read_token(is, "-");
}
bool read_iana_token(std::istream &is) {
        CALLSTACK;
        if (!read_iana_token_char(is))
                return false;
        while (read_iana_token_char(is))
                ;
        // TODO: IANA iCalendar identifiers
        return true;
}
void expect_iana_token(std::istream &is) {
        CALLSTACK;
        if (!read_iana_token(is))
                throw unexpected_token(is.tellg());
}

//     vendorid      = 3*(ALPHA / DIGIT)
//     ; Vendor identification
void expect_vendorid(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        expect_alnum(is);
        expect_alnum(is);
        expect_alnum(is);

        while (read_alnum(is)) {
        }

        ptran.commit();
}
bool read_vendorid(std::istream &is) {
        CALLSTACK;
        try {
                expect_vendorid(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//     SAFE-CHAR     = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-7E / NON-US-ASCII
//     ; Any character except CONTROL, DQUOTE, ";", ":", ","
bool read_safe_char(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (read_wsp(is)) {
                ptran.commit();
                return true;
        }
        const auto i = is.get();
        switch(i) {
                // %x21
        case '!':
                // %x23-2B
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
                // %x2D-39
        case '-':
        case '.':
        case '/':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
                // %x3C-7E
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '_':
        case '`':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '{':
        case '|':
        case '}':
        case '~':
                ptran.commit();
                return true;
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return false;
}

//     VALUE-CHAR    = WSP / %x21-7E / NON-US-ASCII
//     ; Any textual character
bool read_value_char(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (read_wsp(is)) {
                ptran.commit();
                return true;
        }
        const auto i = is.get();
        switch(i) {
                // %x21-7E
        case '!':
        case '"':
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':
        case ',':
        case '-':
        case '.':
        case '/':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case ':':
        case ';':
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '_':
        case '`':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '{':
        case '|':
        case '}':
        case '~':
                ptran.commit();
                return true;
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return false;
}

//     QSAFE-CHAR    = WSP / %x21 / %x23-7E / NON-US-ASCII
//     ; Any character except CONTROL and DQUOTE
bool read_qsafe_char(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     NON-US-ASCII  = UTF8-2 / UTF8-3 / UTF8-4
//     ; UTF8-2, UTF8-3, and UTF8-4 are defined in [RFC3629]
bool read_non_us_ascii(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     CONTROL       = %x00-08 / %x0A-1F / %x7F
//     ; All the controls except HTAB
bool read_control(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     value         = *VALUE-CHAR
void expect_value(std::istream &is) {
        CALLSTACK;
        while (read_value_char(is)) {
        }
}


//     param         = param-name "=" param-value *("," param-value)
//     ; Each property defines the specific ABNF for the parameters
//     ; allowed on the property.  Refer to specific properties for
//     ; precise parameter ABNF.
//
void expect_param(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_param_name(is);
        expect_token(is, "=");
        expect_param_value(is);
        while (read_token(is, ","))
                expect_param_value(is);
        ptran.commit();
}

//     param-name    = iana-token / x-name
void expect_param_name(std::istream &is) {
        CALLSTACK;
        const auto success =
                read_iana_token(is) ||
                read_x_name(is);
        if (!success) {
                throw syntax_error(is.tellg(), "expected param-name");
        }
}


//     param-value   = paramtext / quoted-string
bool read_param_value(std::istream &is) {
        CALLSTACK;
        return read_paramtext(is) || read_quoted_string(is);
}
void expect_param_value(std::istream &is) {
        CALLSTACK;
        if (!read_param_value(is))
                throw syntax_error(is.tellg());
}

//     paramtext     = *SAFE-CHAR
bool read_paramtext(std::istream &is) {
        CALLSTACK;
        while (read_safe_char(is))
                ;
        return true;
}

//     quoted-string = DQUOTE *QSAFE-CHAR DQUOTE
bool read_quoted_string(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        if (!read_dquote(is))
                return false;

        while (read_qsafe_char(is)) {
        }

        if (!read_dquote(is))
                return false;

        ptran.commit();
        return true;
}


// -- Actual, "semantic" parser functions. -------------------------------------
// Short description:
//     BEGIN:VCALENDAR
//     [...]
//     END:VCALENDAR
// Grammar (see https://tools.ietf.org/html/rfc5545#section-3.4):
//     icalstream = 1*icalobject
//
//     icalobject = "BEGIN" ":" "VCALENDAR" CRLF
//                  icalbody
//                  "END" ":" "VCALENDAR" CRLF
void expect_ical(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_key_value(is, "BEGIN", "VCALENDAR");
        expect_icalbody(is);
        expect_key_value(is, "END", "VCALENDAR");
        ptran.commit();
}

bool read_ical(std::istream &is) {
        CALLSTACK;
        try {
                expect_ical(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       icalbody   = calprops component
void expect_icalbody(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_calprops(is);
        expect_component(is);
        ptran.commit();
}

//       calprops   = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  prodid / version /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  calscale / method /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  x-prop / iana-prop
//                  ;
//                  )
bool read_calprop(std::istream &is) {
        CALLSTACK;
        const auto success =
                read_prodid(is) ||
                read_version(is) ||
                read_calscale(is) ||
                read_method(is) ||
                read_x_prop(is) ||
                read_iana_prop(is);
        return success;
}
void expect_calprops(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_calprop(is)) {
        }
        //std::cerr << "expect_calprops() /test semantics\n";
        ptran.commit();
}

//       prodid     = "PRODID" pidparam ":" pidvalue CRLF
void expect_prodid(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_token(is, "PRODID");
        expect_pidparam(is);
        expect_token(is, ":");
        expect_pidvalue(is);
        expect_newline(is);
        ptran.commit();
}
bool read_prodid(std::istream &is) {
        CALLSTACK;
        try {
                expect_prodid(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       version    = "VERSION" verparam ":" vervalue CRLF
void expect_version(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_token(is, "VERSION");
        expect_verparam(is);
        expect_token(is, ":");
        expect_vervalue(is);
        expect_newline(is);
        ptran.commit();
}
bool read_version(std::istream &is) {
        CALLSTACK;
        try {
                expect_version(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       verparam   = *(";" other-param)
void expect_verparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while(read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       vervalue   = "2.0"         ;This memo
//                  / maxver
//                  / (minver ";" maxver)
//
//       minver     = <A IANA-registered iCalendar version identifier>
//       ;Minimum iCalendar version needed to parse the iCalendar object.
//
//       maxver     = <A IANA-registered iCalendar version identifier>
//       ;Maximum iCalendar version needed to parse the iCalendar object.
void expect_vervalue(std::istream &is) {
        CALLSTACK;
        expect_text(is);
}

//       calscale   = "CALSCALE" calparam ":" calvalue CRLF
void expect_calscale(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_token(is, "CALSCALE");
        expect_calparam(is);
        expect_token(is, ":");
        expect_calvalue(is);
        expect_newline(is);
        ptran.commit();
}
bool read_calscale(std::istream &is) {
        CALLSTACK;
        try {
                expect_calscale(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       calparam   = *(";" other-param)
void expect_calparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       calvalue   = "GREGORIAN"
void expect_calvalue(std::istream &is) {
        CALLSTACK;
        expect_token(is, "GREGORIAN");
}

//       method     = "METHOD" metparam ":" metvalue CRLF
bool read_method(std::istream &is) {
        CALLSTACK;
        try {
                expect_method(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}
void expect_method(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_token(is, "METHOD");
        expect_metparam(is);
        expect_token(is, ":");
        expect_metvalue(is);
        expect_newline(is);
        ptran.commit();
}

//       metparam   = *(";" other-param)
void expect_metparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       metvalue   = iana-token
void expect_metvalue(std::istream &is) {
        CALLSTACK;
        expect_iana_token(is);
}

//       x-prop = x-name *(";" icalparameter) ":" value CRLF
void expect_x_prop(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_x_name(is);
        while (read_token(is, ";"))
                expect_icalparameter(is);
        expect_token(is, ":");
        expect_value(is);
        expect_newline(is);
        ptran.commit();
}
bool read_x_prop(std::istream &is) {
        CALLSTACK;
        try {
                expect_x_prop(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}


//     x-name        = "X-" [vendorid "-"] 1*(ALPHA / DIGIT / "-")
//     ; Reserved for experimental use.
void expect_x_name(std::istream &is) {
        CALLSTACK;
        save_input_pos ts(is);

        // "X-"
        if (!read_token(is, "X-"))
                throw syntax_error(is.tellg());

        // [vendorid "-"]
        if (read_vendorid(is)) {
                expect_token(is, "-");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (!read_alnum(is) && !read_token(is, "-"))
                throw syntax_error(is.tellg());
        while (read_alnum(is) || read_token(is, "-")) {
        }

        ts.commit();
}
bool read_x_name(std::istream &is) {
        CALLSTACK;
        try {
                expect_x_name(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       iana-prop = iana-token *(";" icalparameter) ":" value CRLF
void expect_iana_prop(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_iana_token(is);
        while (read_token(is, ";"))
                expect_icalparameter(is);
        expect_token(is, ":");
        expect_value(is);
        expect_newline(is);
        ptran.commit();
}
bool read_iana_prop(std::istream &is) {
        CALLSTACK;
        return false; // TODO
        try {
                expect_iana_prop(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       pidparam   = *(";" other-param)
void expect_pidparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//     other-param   = (iana-param / x-param)
//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
void expect_other_param(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       pidvalue   = text
//       ;Any text that describes the product and version
//       ;and that is generally assured of being unique.
void expect_pidvalue(std::istream &is) {
        CALLSTACK;
        expect_text(is);
}


//       ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
//          ; \\ encodes \, \N or \n encodes newline
//          ; \; encodes ;, \, encodes ,
bool read_escaped_char(std::istream &is) {
        CALLSTACK;
        return read_token(is, "\\\\")
               || read_token(is, "\\;")
               || read_token(is, "\\,")
               || read_token(is, "\\N")
               || read_token(is, "\\n");
}

//       TSAFE-CHAR = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-5B /
//                    %x5D-7E / NON-US-ASCII
//          ; Any character except CONTROLs not needed by the current
//          ; character set, DQUOTE, ";", ":", "\", ","
bool read_tsafe_char(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (read_wsp(is)) {
                ptran.commit();
                return true;
        }
        const auto i = is.get();
        switch(i) {
                // %x21
        case '!':

                // %x23-2B
        case '#':
        case '$':
        case '%':
        case '&':
        case '\'':
        case '(':
        case ')':
        case '*':
        case '+':

                // %x2D-39
        case '-':
        case '.':
        case '/':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':

                // %x3C-5B
        case '<':
        case '=':
        case '>':
        case '?':
        case '@':
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'I':
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
        case 'O':
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'W':
        case 'X':
        case 'Y':
        case 'Z':
        case '[':

                // %x5D-7E
        case ']':
        case '^':
        case '_':
        case '`':
        case 'a':
        case 'b':
        case 'c':
        case 'd':
        case 'e':
        case 'f':
        case 'g':
        case 'h':
        case 'i':
        case 'j':
        case 'k':
        case 'l':
        case 'm':
        case 'n':
        case 'o':
        case 'p':
        case 'q':
        case 'r':
        case 's':
        case 't':
        case 'u':
        case 'v':
        case 'w':
        case 'x':
        case 'y':
        case 'z':
        case '{':
        case '|':
        case '}':
        case '~':
                ptran.commit();
                return true;
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return false;
}


//       text       = *(TSAFE-CHAR / ":" / DQUOTE / ESCAPED-CHAR)
//          ; Folded according to description above
//
bool read_text_char(std::istream &is) {
        CALLSTACK;
        return read_tsafe_char(is)
               || read_token(is, ":")
               || read_dquote(is)
               || read_escaped_char(is);
}
void expect_text(std::istream &is) {
        CALLSTACK;
        while(read_text_char(is)) {
        }
}

// DQUOTE: ASCII 22
bool read_dquote(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != 22)
                return false;
        ptran.commit();
        return true;
}

bool read_text(std::istream &is) {
        CALLSTACK;
        try {
                expect_text(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

bool read_binary(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       float      = (["+"] / "-") 1*DIGIT ["." 1*DIGIT]
bool read_float(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        // (["+"] / "-")
        if (!read_token(is, "+"))
                read_token(is, "-");

        // 1*DIGIT
        if (!read_digit(is))
                return false;
        while (read_digit(is)) {
        }

        // ["." 1*DIGIT]
        if (read_token(is, ".")) {
                if (!read_digit(is))
                        return false;
                while (read_digit(is)) {
                }
        }

        ptran.commit();
        return true;
}
//       integer    = (["+"] / "-") 1*DIGIT
bool read_integer(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        // (["+"] / "-")
        if (!read_token(is, "+"))
                read_token(is, "-");

        // 1*DIGIT
        if (!read_digit(is))
                return false;
        while (read_digit(is)) {
        }

        ptran.commit();
        return true;
}
//       actionvalue = "AUDIO" / "DISPLAY" / "EMAIL" / iana-token / x-name
bool read_actionvalue(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "AUDIO") ||
                read_token(is, "DISPLAY") ||
                read_token(is, "EMAIL") ||
                read_iana_token(is) ||
                read_x_name(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       actionparam = *(";" other-param)
bool read_actionparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}
//       action      = "ACTION" actionparam ":" actionvalue CRLF
bool read_action(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ACTION") &&
                read_actionparam(is) &&
                read_token(is, ":") &&
                read_actionvalue(is) &&
                read_newline(is) ;
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       trigabs    = *(
//                  ;
//                  ; The following is REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" "DATE-TIME") /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  ) ":" date-time
bool read_trigabs(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ACTION") &&
                read_actionparam(is) &&
                read_token(is, ":") &&
                read_actionvalue(is) &&
                read_newline(is) ;
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       trigrel    = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" "DURATION") /
//                  (";" trigrelparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  ) ":"  dur-value
bool read_trigrel_single(std::istream &is) {
        CALLSTACK;
        // (";" "VALUE" "=" "DURATION")
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        read_token(is, "DURATION");
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" trigrelparam)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_trigrelparam(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_other_param(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool read_trigrel(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_trigrel_single(is)) {
        }
        // ":"  dur-value
        const auto match = read_token(is, ":") && read_dur_value(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       trigger    = "TRIGGER" (trigrel / trigabs) CRLF
bool read_trigger(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "TRIGGER") &&
                (read_trigrel(is) || read_trigabs(is)) &&
                read_newline(is) ;
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       repparam   = *(";" other-param)
bool read_repparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}
//       repeat  = "REPEAT" repparam ":" integer CRLF  ;Default is "0", zero.
bool read_repeat(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "REPEAT") &&
                read_repparam(is) &&
                read_token(is, ":") &&
                read_integer(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       audioprop  = *(
//                  ;
//                  ; 'action' and 'trigger' are both REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  action / trigger /
//                  ;
//                  ; 'duration' and 'repeat' are both OPTIONAL,
//                  ; and MUST NOT occur more than once each;
//                  ; but if one occurs, so MUST the other.
//                  ;
//                  duration / repeat /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  attach /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  x-prop / iana-prop
//                  ;
//                  )
bool read_audioprop_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_action(is) || read_trigger(is) ||
                read_duration(is) || read_repeat(is) ||
                read_attach(is) ||
                read_x_prop(is) || read_iana_prop(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       dispprop   = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  action / description / trigger /
//                  ;
//                  ; 'duration' and 'repeat' are both OPTIONAL,
//                  ; and MUST NOT occur more than once each;
//                  ; but if one occurs, so MUST the other.
//                  ;
//                  duration / repeat /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  x-prop / iana-prop
//                  ;
//                  )
bool read_dispprop_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_action(is) || read_description(is) || read_trigger(is) ||
                read_duration(is) || read_repeat(is) ||
                read_attach(is) ||
                read_x_prop(is) || read_iana_prop(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       emailprop  = *(
//                  ;
//                  ; The following are all REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  action / description / trigger / summary /
//                  ;
//                  ; The following is REQUIRED,
//                  ; and MAY occur more than once.
//                  ;
//                  attendee /
//                  ;
//                  ; 'duration' and 'repeat' are both OPTIONAL,
//                  ; and MUST NOT occur more than once each;
//                  ; but if one occurs, so MUST the other.
//                  ;
//                  duration / repeat /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  attach / x-prop / iana-prop
//                  ;
//                  )
bool read_emailprop_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_action(is) || read_description(is) || read_trigger(is) ||
                read_summary(is) ||
                read_attendee(is) ||
                read_duration(is) || read_repeat(is) ||
                read_attach(is) || read_x_prop(is) || read_iana_prop(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
bool read_alarmc_prop(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_audioprop_single(is) ||
               read_dispprop_single(is) ||
               read_emailprop_single(is)) {
        }
        ptran.commit();
        return true;
}
//       alarmc     = "BEGIN" ":" "VALARM" CRLF
//                    (audioprop / dispprop / emailprop)
//                    "END" ":" "VALARM" CRLF
bool read_alarmc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VALARM") &&
                read_alarmc_prop(is) &&
                read_key_value(is, "END", "VALARM");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       date-fullyear      = 4DIGIT
bool read_date_fullyear(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digit(is) &&
                read_digit(is) &&
                read_digit(is) &&
                read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       date-month         = 2DIGIT        ;01-12
bool read_date_month(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digit(is) &&
                read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       date-mday          = 2DIGIT        ;01-28, 01-29, 01-30, 01-31
//                                          ;based on month/year
bool read_date_mday(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digit(is) &&
                read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       date-value         = date-fullyear date-month date-mday
bool read_date_value(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_date_fullyear(is) &&
                read_date_month(is) &&
                read_date_mday(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       date               = date-value
bool read_date(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_date_value(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       time-hour    = 2DIGIT        ;00-23
bool read_time_hour(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_digit(is) && read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       time-minute  = 2DIGIT        ;00-59
bool read_time_minute(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_digit(is) && read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       time-second  = 2DIGIT        ;00-60
//       ;The "60" value is used to account for positive "leap" seconds.
bool read_time_second(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_digit(is) && read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       time-utc     = "Z"
bool read_time_utc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_token(is, "Z");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       time         = time-hour time-minute time-second [time-utc]
bool read_time(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_time_hour(is) &&
                read_time_minute(is) &&
                read_time_second(is) &&
                (read_time_utc(is) || true);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       date-time  = date "T" time ;As specified in the DATE and TIME
//                                  ;value definitions
bool read_date_time(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_date(is) &&
                read_token(is, "T") &&
                read_time(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-week   = 1*DIGIT "W"
bool read_dur_week(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digits(is, 1) &&
                read_token(is, "W");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-second = 1*DIGIT "S"
bool read_dur_second(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digits(is, 1) &&
                read_token(is, "S");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-minute = 1*DIGIT "M" [dur-second]
bool read_dur_minute(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digits(is, 1) &&
                read_token(is, "M") &&
                (read_dur_second(is) || true);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-hour   = 1*DIGIT "H" [dur-minute]
bool read_dur_hour(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digits(is, 1) &&
                read_token(is, "H") &&
                (read_dur_minute(is) || true);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-day    = 1*DIGIT "D"
bool read_dur_day(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_digits(is, 1) &&
                read_token(is, "D");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-time   = "T" (dur-hour / dur-minute / dur-second)
bool read_dur_time(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "T") &&
                (
                        read_dur_hour(is) ||
                        read_dur_minute(is) ||
                        read_dur_second(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-date   = dur-day [dur-time]
bool read_dur_date(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_dur_day(is) &&
                (read_dur_time(is) || true);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)
bool read_dur_value(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                (read_token(is, "+") || read_token(is, "-") || true) &&
                read_token(is, "P") &&
                (read_dur_date(is) || read_dur_time(is) || read_dur_week(is));
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       period-start = date-time "/" dur-value
//       ; [ISO.8601.2004] complete representation basic format for a
//       ; period of time consisting of a start and positive duration
//       ; of time.
bool read_period_start(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_date_time(is) &&
                read_token(is, "/") &&
                read_dur_value(is) ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//       period-explicit = date-time "/" date-time
bool read_period_explicit(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_date_time(is) &&
                read_token(is, "/") &&
                read_date_time(is) ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//       ; [ISO.8601.2004] complete representation basic format for a
//       ; period of time consisting of a start and end.  The start MUST
//       ; be before the end.
//       period     = period-explicit / period-start
bool read_period(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_period_explicit(is) ||
                read_period_start(is) ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//     other-param   = (iana-param / x-param)
bool read_other_param(std::istream &is) {
        CALLSTACK;
        return read_iana_param(is) || read_x_param(is);
}

// stmparam   = *(";" other-param)
bool read_stmparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}

// dtstamp    = "DTSTAMP" stmparam ":" date-time CRLF
bool read_dtstamp(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTSTAMP") &&
                read_stmparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       uidparam   = *(";" other-param)
bool read_uidparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}
//       uid        = "UID" uidparam ":" text CRLF
bool read_uid(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "UID") &&
                read_uidparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       dtstval    = date-time / date
bool read_dtstval(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_date_time(is) || read_date(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       ;Value MUST match value type
//       dtstparam  = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" ("DATE-TIME" / "DATE")) /
//                  (";" tzidparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_dtstparam_single(std::istream &is) {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE"))
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        (read_token(is, "DATE-TIME") || read_token(is, "DATE"));
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" tzidparam)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_tzidparam(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_other_param(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool read_dtstparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_dtstparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       dtstart    = "DTSTART" dtstparam ":" dtstval CRLF
//
bool read_dtstart(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTSTART") &&
                read_dtstparam(is) &&
                read_token(is, ":") &&
                read_dtstval(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       classvalue = "PUBLIC" / "PRIVATE" / "CONFIDENTIAL" / iana-token
//                  / x-name
//       ;Default is PUBLIC
bool read_classvalue(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       classparam = *(";" other-param)
bool read_classparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       class      = "CLASS" classparam ":" classvalue CRLF
bool read_class(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "CLASS") &&
                read_classparam(is) &&
                read_token(is, ":") &&
                read_classvalue(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       creaparam  = *(";" other-param)
bool read_creaparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       created    = "CREATED" creaparam ":" date-time CRLF
bool read_created(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "CREATED") &&
                read_creaparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       descparam   = *(
//                   ;
//                   ; The following are OPTIONAL,
//                   ; but MUST NOT occur more than once.
//                   ;
//                   (";" altrepparam) /
//                   (";" languageparam) /
//                   ;
//                   ; The following is OPTIONAL,
//                   ; and MAY occur more than once.
//                   ;
//                   (";" other-param)
//                   ;
//                   )
bool read_descparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!(read_altrepparam(is) ||
                      read_languageparam(is) ||
                      read_other_param(is)))
                        return false;
        }
        ptran.commit();
        return true;
}
//       description = "DESCRIPTION" descparam ":" text CRLF
bool read_description(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DESCRIPTION") &&
                read_descparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       geovalue   = float ";" float
//       ;Latitude and Longitude components
bool read_geovalue(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_float(is) &&
                read_token(is, ";") &&
                read_float(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       geoparam   = *(";" other-param)
bool read_geoparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}
//       geo        = "GEO" geoparam ":" geovalue CRLF
bool read_geo(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "GEO") &&
                read_geoparam(is) &&
                read_token(is, ":") &&
                read_geovalue(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       lstparam   = *(";" other-param)
bool read_lstparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       last-mod   = "LAST-MODIFIED" lstparam ":" date-time CRLF
bool read_last_mod(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LAST-MODIFIED") &&
                read_lstparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       locparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" altrepparam) / (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_locparam_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, ";") && (
                        read_altrepparam(is) ||
                        read_languageparam(is) ||
                        read_other_param(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}
bool read_locparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_locparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       location   = "LOCATION"  locparam ":" text CRLF
bool read_location(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LOCATION") &&
                read_locparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       orgparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" cnparam) / (";" dirparam) / (";" sentbyparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_orgparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
bool read_cal_address(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       organizer  = "ORGANIZER" orgparam ":" cal-address CRLF
bool read_organizer(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ORGANIZER") &&
                read_orgparam(is) &&
                read_token(is, ":") &&
                read_cal_address(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       priovalue   = integer       ;Must be in the range [0..9]
//          ; All other values are reserved for future use.
bool read_priovalue(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       prioparam  = *(";" other-param)
bool read_prioparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       priority   = "PRIORITY" prioparam ":" priovalue CRLF
//       ;Default is zero (i.e., undefined).
bool read_priority(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "PRIORITY") &&
                read_prioparam(is) &&
                read_token(is, ":") &&
                read_priovalue(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       seqparam   = *(";" other-param)
bool read_seqparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}
//       seq = "SEQUENCE" seqparam ":" integer CRLF     ; Default is "0"
bool read_seq(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "SEQUENCE") &&
                read_seqparam(is) &&
                read_token(is, ":") &&
                read_integer(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       statvalue-jour  = "DRAFT"        ;Indicates journal is draft.
//                       / "FINAL"        ;Indicates journal is final.
//                       / "CANCELLED"    ;Indicates journal is removed.
//      ;Status values for "VJOURNAL".
bool read_statvalue_jour(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       statvalue-todo  = "NEEDS-ACTION" ;Indicates to-do needs action.
//                       / "COMPLETED"    ;Indicates to-do completed.
//                       / "IN-PROCESS"   ;Indicates to-do in process of.
//                       / "CANCELLED"    ;Indicates to-do was cancelled.
//       ;Status values for "VTODO".
bool read_statvalue_todo(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}


//       statvalue       = (statvalue-event
//                       /  statvalue-todo
//                       /  statvalue-jour)
bool read_statvalue(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       statvalue-event = "TENTATIVE"    ;Indicates event is tentative.
//                       / "CONFIRMED"    ;Indicates event is definite.
//                       / "CANCELLED"    ;Indicates event was cancelled.
//       ;Status values for a "VEVENT"
bool read_statvalue_event(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       statparam       = *(";" other-param)
bool read_statparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       status          = "STATUS" statparam ":" statvalue CRLF
bool read_status(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "STATUS") &&
                read_statparam(is) &&
                read_token(is, ":") &&
                read_statvalue(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       summparam  = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" altrepparam) / (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_summparam_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, ";") && (
                        read_altrepparam(is) ||
                        read_languageparam(is) ||
                        read_other_param(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}
bool read_summparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_summparam_single(is)) {
        }
        ptran.commit();
        return true;
}

//       summary    = "SUMMARY" summparam ":" text CRLF
bool read_summary(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "SUMMARY") &&
                read_summparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       transparam = *(";" other-param)
bool read_transparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                if (!read_other_param(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//       transvalue = "OPAQUE"
//                   ;Blocks or opaque on busy time searches.
//                   / "TRANSPARENT"
//                   ;Transparent on busy time searches.
//       ;Default value is OPAQUE
bool read_transvalue(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "OPAQUE") ||
                read_token(is, "TRANSPARENT");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       transp     = "TRANSP" transparam ":" transvalue CRLF
bool read_transp(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "TRANSP") &&
                read_transparam(is) &&
                read_token(is, ":") &&
                read_transvalue(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//      uri = <As defined in Section 3 of [RFC3986]>
bool read_uri(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = rfc3986::read_URI(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       urlparam   = *(";" other-param)
bool read_urlparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       url        = "URL" urlparam ":" uri CRLF
bool read_url(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "URL") &&
                read_urlparam(is) &&
                read_token(is, ":") &&
                read_uri(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       ridval     = date-time / date
//       ;Value MUST match value type
bool read_ridval(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_date_time(is) || read_date(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}


//       ridparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" ("DATE-TIME" / "DATE")) /
//                  (";" tzidparam) / (";" rangeparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_ridparam_single(std::istream &is) {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE"))
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        (read_token(is, "DATE-TIME") || read_token(is, "DATE"));
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" tzidparam) / (";" rangeparam)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        (read_tzidparam(is) || read_rangeparam(is));
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_other_param(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool read_ridparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_ridparam_single(is)) {
        }
        ptran.commit();
        return true;
}

//       setposday   = yeardaynum
bool read_setposday(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       bysplist    = ( setposday *("," setposday) )
bool read_bysplist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       monthnum    = 1*2DIGIT       ;1 to 12
bool read_monthnum(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       bymolist    = ( monthnum *("," monthnum) )
bool read_bymolist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       weeknum     = [plus / minus] ordwk
bool read_weeknum(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       bywknolist  = ( weeknum *("," weeknum) )
bool read_bywknolist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       ordyrday    = 1*3DIGIT      ;1 to 366
bool read_ordyrday(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       yeardaynum  = [plus / minus] ordyrday
bool read_yeardaynum(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       byyrdaylist = ( yeardaynum *("," yeardaynum) )
bool read_byyrdaylist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       ordmoday    = 1*2DIGIT       ;1 to 31
bool read_ordmoday(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       monthdaynum = [plus / minus] ordmoday
bool read_monthdaynum(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       bymodaylist = ( monthdaynum *("," monthdaynum) )
bool read_bymodaylist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       weekday     = "SU" / "MO" / "TU" / "WE" / "TH" / "FR" / "SA"
//       ;Corresponding to SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY,
//       ;FRIDAY, and SATURDAY days of the week.
bool read_weekday(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       ordwk       = 1*2DIGIT       ;1 to 53
bool read_ordwk(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       minus       = "-"
bool readminus(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       plus        = "+"
bool read_plus(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       weekdaynum  = [[plus / minus] ordwk] weekday
bool read_weekdaynum(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       bywdaylist  = ( weekdaynum *("," weekdaynum) )
bool read_bywdaylist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       hour        = 1*2DIGIT       ;0 to 23
bool read_hour(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       byhrlist    = ( hour *("," hour) )
bool read_byhrlist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       minutes     = 1*2DIGIT       ;0 to 59
bool read_minutes(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       byminlist   = ( minutes *("," minutes) )
bool read_byminlist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       seconds     = 1*2DIGIT       ;0 to 60
bool read_seconds(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       byseclist   = ( seconds *("," seconds) )
bool read_byseclist(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       enddate     = date / date-time
bool read_enddate(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       freq        = "SECONDLY" / "MINUTELY" / "HOURLY" / "DAILY"
//                   / "WEEKLY" / "MONTHLY" / "YEARLY"
bool read_freq(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       recur-rule-part = ( "FREQ" "=" freq )
//                       / ( "UNTIL" "=" enddate )
//                       / ( "COUNT" "=" 1*DIGIT )
//                       / ( "INTERVAL" "=" 1*DIGIT )
//                       / ( "BYSECOND" "=" byseclist )
//                       / ( "BYMINUTE" "=" byminlist )
//                       / ( "BYHOUR" "=" byhrlist )
//                       / ( "BYDAY" "=" bywdaylist )
//                       / ( "BYMONTHDAY" "=" bymodaylist )
//                       / ( "BYYEARDAY" "=" byyrdaylist )
//                       / ( "BYWEEKNO" "=" bywknolist )
//                       / ( "BYMONTH" "=" bymolist )
//                       / ( "BYSETPOS" "=" bysplist )
//                       / ( "WKST" "=" weekday )
bool read_recur_rule_part(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       recur           = recur-rule-part *( ";" recur-rule-part )
//                       ;
//                       ; The rule parts are not ordered in any
//                       ; particular sequence.
//                       ;
//                       ; The FREQ rule part is REQUIRED,
//                       ; but MUST NOT occur more than once.
//                       ;
//                       ; The UNTIL or COUNT rule parts are OPTIONAL,
//                       ; but they MUST NOT occur in the same 'recur'.
//                       ;
//                       ; The other rule parts are OPTIONAL,
//                       ; but MUST NOT occur more than once.
bool read_recur(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       recurid    = "RECURRENCE-ID" ridparam ":" ridval CRLF
bool read_recurid(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RECURRENCE-ID") &&
                read_ridparam(is) &&
                read_token(is, ":") &&
                read_ridval(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       rrulparam  = *(";" other-param)
bool read_rrulparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       rrule      = "RRULE" rrulparam ":" recur CRLF
bool read_rrule(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RRULE") &&
                read_rrulparam(is) &&
                read_token(is, ":") &&
                read_recur(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       dtendval   = date-time / date
//       ;Value MUST match value type
bool read_dtendval(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_date_time(is) ||read_date(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//        = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" ("DATE-TIME" / "DATE")) /
//                  (";" tzidparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//           )
bool read_dtendparam(std::istream &is) {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE"))
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        (read_token(is, "DATE-TIME") || read_token(is, "DATE"));
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" tzidparam)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_tzidparam(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto success =
                        read_token(is, ";") &&
                        read_other_param(is);
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
//       dtend      = "DTEND" dtendparam ":" dtendval CRLF
bool read_dtend(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTEND") &&
                read_dtendparam(is) &&
                read_token(is, ":") &&
                read_dtendval(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       durparam   = *(";" other-param)
bool read_durparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       duration   = "DURATION" durparam ":" dur-value CRLF
//                    ;consisting of a positive duration of time.
bool read_duration(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DURATION") &&
                read_durparam(is) &&
                read_token(is, ":") &&
                read_dur_value(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       attachparam = *(
//                   ;
//                   ; The following is OPTIONAL for a URI value,
//                   ; RECOMMENDED for a BINARY value,
//                   ; and MUST NOT occur more than once.
//                   ;
//                   (";" fmttypeparam) /
//                   ;
//                   ; The following is OPTIONAL,
//                   ; and MAY occur more than once.
//                   ;
//                   (";" other-param)
//                   ;
//                   )
bool read_attachparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, ";") &&
                (read_fmttypeparam(is) || read_other_param(is));
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       attach     = "ATTACH" attachparam
//                    (
//                        ":" uri
//                    )
//                    /
//                    (
//                      ";" "ENCODING" "=" "BASE64"
//                        ";" "VALUE" "=" "BINARY"
//                        ":" binary
//                    )
//                    CRLF
bool read_attach(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto match_head =
                read_token(is, "ATTACH") && read_attachparam(is);
        if (!match_head)
                return false;

        // ":" uri
        {
                save_input_pos ptran_uri(is);
                const auto match_uri =
                        read_token(is, ":") &&
                        read_uri(is) &&
                        read_newline(is) ;
                if (match_uri) {
                        ptran_uri.commit();
                        ptran.commit();
                        return true;
                }
        }

        // ";" "ENCODING" "=" "BASE64" ";" "VALUE" "=" "BINARY" ":" binary
        {
                save_input_pos ptran_enc(is);
                const auto match_enc =
                        read_token(is, ";") &&
                        read_token(is, "ENCODING") &&
                        read_token(is, "=") &&
                        read_token(is, "BASE64") &&
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        read_token(is, "BINARY") &&
                        read_token(is, ":") &&
                        read_binary(is) &&
                        read_newline(is) ;
                if (match_enc) {
                        ptran_enc.commit();
                        ptran.commit();
                        return true;
                }
        }

        return false;
}
//       attparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" cutypeparam) / (";" memberparam) /
//                  (";" roleparam) / (";" partstatparam) /
//                  (";" rsvpparam) / (";" deltoparam) /
//                  (";" delfromparam) / (";" sentbyparam) /
//                  (";" cnparam) / (";" dirparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_attparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       attendee   = "ATTENDEE" attparam ":" cal-address CRLF
bool read_attendee(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "ATTENDEE") &&
                read_attparam(is) &&
                read_token(is, ":") &&
                read_cal_address(is) &&
                read_newline(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       catparam   = *(
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" languageparam ) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_catparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                const auto match =
                        read_languageparam(is) ||
                        read_other_param(is);
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       categories = "CATEGORIES" catparam ":" text *("," text)
//                    CRLF
bool read_categories (std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "CATEGORIES") &&
                read_catparam(is) &&
                read_token(is, ":") &&
                read_text(is);
        if (!match)
                return false;
        while (read_token(is, ",")) {
                if (!read_text(is))
                        return false;
        }
        if (!read_newline(is))
                return false;

        ptran.commit();
        return true;
}
//       commparam  = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" altrepparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_commparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                const auto match =
                        read_altrepparam(is) ||
                        read_languageparam(is) ||
                        read_other_param(is);
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       comment    = "COMMENT" commparam ":" text CRLF
bool read_comment(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "COMMENT") &&
                read_commparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       contparam  = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" altrepparam)/
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_contparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                const auto match =
                        read_altrepparam(is) ||
                        read_languageparam(is) ||
                        read_other_param(is);
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       contact    = "CONTACT" contparam ":" text CRLF
bool read_contact(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "CONTACT") &&
                read_contparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       exdtval    = date-time / date
//       ;Value MUST match value type
bool read_exdtval(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       exdtparam  = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" ("DATE-TIME" / "DATE")) /
//                  ;
//                  (";" tzidparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_exdtparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       exdate     = "EXDATE" exdtparam ":" exdtval *("," exdtval) CRLF
bool read_exdate(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "EXDATE") &&
                read_exdtparam(is) &&
                read_token(is, ":") &&
                read_exdtval(is);
        if (!match)
                return false;
        while (read_token(is, ",")) {
                if (!read_exdtval(is))
                        return false;
        }
        if (!read_newline(is))
                return false;
        ptran.commit();
        return true;
}

//       extdata    = text
//       ;Textual exception data.  For example, the offending property
//       ;name and value or complete property line.
bool read_extdata(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_text(is))
                return false;
        ptran.commit();
        return true;
}

//       statdesc   = text
//       ;Textual status description
bool read_statdesc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_text(is))
                return false;
        ptran.commit();
        return true;
}

//       statcode   = 1*DIGIT 1*2("." 1*DIGIT)
//       ;Hierarchical, numeric return status code
bool read_statcode(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       rstatparam = *(
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_rstatparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                const auto match =
                        read_languageparam(is) ||
                        read_other_param(is);
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}

//       rstatus    = "REQUEST-STATUS" rstatparam ":"
//                    statcode ";" statdesc [";" extdata]
bool read_rstatus(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "REQUEST-STATUS") &&
                read_rstatparam(is) &&
                read_token(is, ":") &&
                read_statcode(is) &&
                read_token(is, ";") &&
                read_statdesc(is) &&
                (read_token(is, ";") ? read_extdata(is) : true);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       reltypeparam       = "RELTYPE" "="
//                           ("PARENT"    ; Parent relationship - Default
//                          / "CHILD"     ; Child relationship
//                          / "SIBLING"   ; Sibling relationship
//                          / iana-token  ; Some other IANA-registered
//                                        ; iCalendar relationship type
//                          / x-name)     ; A non-standard, experimental
//                                        ; relationship type
bool read_reltypeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "RELTYPE") &&
                read_token(is, "=") &&
                (
                        read_token(is, "PARENT") ||
                        read_token(is, "CHILD") ||
                        read_token(is, "SIBLING") ||
                        read_iana_token(is) ||
                        read_x_name(is)
                );
        if (!match)
                return false;
        ptran.commit();
        return true;
}
//       relparam   = *(
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" reltypeparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_relparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";")) {
                const auto match =
                        read_reltypeparam(is) ||
                        read_other_param(is);
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       related    = "RELATED-TO" relparam ":" text CRLF
bool read_related(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RELATED-TO") &&
                read_relparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       resrcparam = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" altrepparam) / (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_resrcparam(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       resources  = "RESOURCES" resrcparam ":" text *("," text) CRLF
bool read_resources(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RESOURCES") &&
                read_resrcparam(is) &&
                read_token(is, ":") &&
                read_text(is);
        if (!success)
                return false;
        while (read_token(is, ",")) {
                if (!read_text(is))
                        return false;
        }
        if (!read_newline(is))
                return false;
        ptran.commit();
        return true;
}
//       rdtval     = date-time / date / period
//       ;Value MUST match value type
bool read_rdtval(std::istream &is) {
        CALLSTACK;
        return read_date_time(is) || read_date(is) || read_period(is);
}
//       rdtparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" "VALUE" "=" ("DATE-TIME" / "DATE" / "PERIOD")) /
//                  (";" tzidparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
bool read_rdtparam_single(std::istream &is) {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE" / "PERIOD"))
        {
                save_input_pos ptran(is);
                const auto match =
                        read_token(is, ";") &&
                        read_token(is, "VALUE") &&
                        read_token(is, "=") &&
                        (
                                read_date_time(is) ||
                                read_date(is) ||
                                read_period(is)
                        );
                if (match) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" tzidparam)
        {
                save_input_pos ptran(is);
                const auto match =
                        read_token(is, ";") &&
                        read_tzidparam(is);
                if (match) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto match =
                        read_token(is, ";") &&
                        read_other_param(is);
                if (match) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool read_rdtparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_rdtparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       rdate      = "RDATE" rdtparam ":" rdtval *("," rdtval) CRLF
bool read_rdate(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RDATE") &&
                read_rdtparam(is) &&
                read_token(is, ":") &&
                read_rdtval(is);
        if (!success)
                return false;
        while (read_token(is, ",")) {
                if (!read_rdtval(is))
                        return false;
        }
        if (!read_newline(is))
                return false;
        ptran.commit();
        return true;
}

//       eventprop  = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  dtstamp / uid /
//                  ;
//                  ; The following is REQUIRED if the component
//                  ; appears in an iCalendar object that doesn't
//                  ; specify the "METHOD" property; otherwise, it
//                  ; is OPTIONAL; in any case, it MUST NOT occur
//                  ; more than once.
//                  ;
//                  dtstart /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  class / created / description / geo /
//                  last-mod / location / organizer / priority /
//                  seq / status / summary / transp /
//                  url / recurid /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but SHOULD NOT occur more than once.
//                  ;
//                  rrule /
//                  ;
//                  ; Either 'dtend' or 'duration' MAY appear in
//                  ; a 'eventprop', but 'dtend' and 'duration'
//                  ; MUST NOT occur in the same 'eventprop'.
//                  ;
//                  dtend / duration /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  attach / attendee / categories / comment /
//                  contact / exdate / rstatus / related /
//                  resources / rdate / x-prop / iana-prop
//                  ;
//                  )
// [condensed-version]
//  eventprop = *( dtstamp / uid /
//
//                 dtstart /
//
//                 class / created / description / geo /
//                  last-mod / location / organizer / priority /
//                  seq / status / summary / transp /
//                  url / recurid /
//
//                 rrule /
//
//                 dtend / duration /
//
//                 attach / attendee / categories / comment /
//                  contact / exdate / rstatus / related /
//                  resources / rdate / x-prop / iana-prop
//               )
bool read_eventprop_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_dtstamp(is) || read_uid(is) ||

                read_dtstart(is) ||

                read_class(is) || read_created(is) || read_description(is) ||
                read_geo(is) || read_last_mod(is) || read_location(is) ||
                read_organizer(is) || read_priority(is) || read_seq(is) ||
                read_rstatus(is) || read_summary(is) || read_transp(is) ||
                read_url(is) || read_recurid(is) ||

                read_rrule(is) ||

                read_dtend(is) || read_duration(is) ||

                read_attach(is) || read_attendee(is) || read_categories (is) ||
                read_comment(is) || read_contact(is) || read_exdate(is) ||
                read_rstatus(is) || read_related(is) || read_resources(is) ||
                read_rdate(is) || read_x_prop(is) || read_iana_prop(is)
        ;
        if (!success) {
                return false;
        }

        ptran.commit();
        return true;
}
bool read_eventprop(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_eventprop_single(is)) {
        }
        ptran.commit();
        return true;
}

//       eventc     = "BEGIN" ":" "VEVENT" CRLF
//                    eventprop *alarmc
//                    "END" ":" "VEVENT" CRLF
bool read_eventc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_key_value(is, "BEGIN", "VEVENT") &&
                read_eventprop(is);
        if (!success_pro) {
                return false;
        }

        while(read_alarmc(is)) {
        }

        const auto success_epi =
                read_key_value(is, "END", "VEVENT") ;
        if (!success_epi)
                return false;

        ptran.commit();
        return true;
}


//       todoprop   = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  dtstamp / uid /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  class / completed / created / description /
//                  dtstart / geo / last-mod / location / organizer /
//                  percent / priority / recurid / seq / status /
//                  summary / url /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but SHOULD NOT occur more than once.
//                  ;
//                  rrule /
//                  ;
//                  ; Either 'due' or 'duration' MAY appear in
//                  ; a 'todoprop', but 'due' and 'duration'
//                  ; MUST NOT occur in the same 'todoprop'.
//                  ; If 'duration' appear in a 'todoprop',
//                  ; then 'dtstart' MUST also appear in
//                  ; the same 'todoprop'.
//                  ;
//                  due / duration /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  attach / attendee / categories / comment / contact /
//                  exdate / rstatus / related / resources /
//                  rdate / x-prop / iana-prop
//                  ;
//                  )
bool read_todoprop(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       todoc      = "BEGIN" ":" "VTODO" CRLF
//                    todoprop *alarmc
//                    "END" ":" "VTODO" CRLF
bool read_todoc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_key_value(is, "BEGIN", "VTODO") &&
                read_todoprop(is);
        if (!success_pro)
                return false;

        while(read_alarmc(is)) {
        }

        const auto success_epi =
                read_key_value(is, "END", "VTODO") ;
        if (!success_epi)
                return false;

        ptran.commit();

        return true;
}

//       jourprop   = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  dtstamp / uid /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  class / created / dtstart /
//                  last-mod / organizer / recurid / seq /
//                  status / summary / url /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; but SHOULD NOT occur more than once.
//                  ;
//                  rrule /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  attach / attendee / categories / comment /
//                  contact / description / exdate / related / rdate /
//                  rstatus / x-prop / iana-prop
//                  ;
//                  )
bool read_jourprop(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       journalc   = "BEGIN" ":" "VJOURNAL" CRLF
//                    jourprop
//                    "END" ":" "VJOURNAL" CRLF
bool read_journalc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VJOURNAL") &&
                read_jourprop(is) &&
                read_key_value(is, "END", "VJOURNAL") ;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       fbprop     = *(
//                  ;
//                  ; The following are REQUIRED,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  dtstamp / uid /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  contact / dtstart / dtend /
//                  organizer / url /
//                  ;
//                  ; The following are OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  attendee / comment / freebusy / rstatus / x-prop /
//                  iana-prop
//                  ;
//                  )
bool read_fbprop(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       freebusyc  = "BEGIN" ":" "VFREEBUSY" CRLF
//                    fbprop
//                    "END" ":" "VFREEBUSY" CRLF
bool read_freebusyc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VFREEBUSY") &&
                read_fbprop(is) &&
                read_key_value(is, "END", "VFREEBUSY") ;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

bool read_tzid(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

bool read_tzurl(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       tzprop     = *(
//                    ;
//                    ; The following are REQUIRED,
//                    ; but MUST NOT occur more than once.
//                    ;
//                    dtstart / tzoffsetto / tzoffsetfrom /
//                    ;
//                    ; The following is OPTIONAL,
//                    ; but SHOULD NOT occur more than once.
//                    ;
//                    rrule /
//                    ;
//                    ; The following are OPTIONAL,
//                    ; and MAY occur more than once.
//                    ;
//                    comment / rdate / tzname / x-prop / iana-prop
//                    ;
//                    )
bool read_tzprop(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       daylightc  = "BEGIN" ":" "DAYLIGHT" CRLF
//                    tzprop
//                    "END" ":" "DAYLIGHT" CRLF
bool read_daylightc(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       standardc  = "BEGIN" ":" "STANDARD" CRLF
//                    tzprop
//                    "END" ":" "STANDARD" CRLF
bool read_standardc(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       timezonec  = "BEGIN" ":" "VTIMEZONE" CRLF
//                    *(
//                    ;
//                    ; 'tzid' is REQUIRED, but MUST NOT occur more
//                    ; than once.
//                    ;
//                    tzid /
//                    ;
//                    ; 'last-mod' and 'tzurl' are OPTIONAL,
//                    ; but MUST NOT occur more than once.
//                    ;
//                    last-mod / tzurl /
//                    ;
//                    ; One of 'standardc' or 'daylightc' MUST occur
//                    ; and each MAY occur more than once.
//                    ;
//                    standardc / daylightc /
//                    ;
//                    ; The following are OPTIONAL,
//                    ; and MAY occur more than once.
//                    ;
//                    x-prop / iana-prop
//                    ;
//                    )
//                    "END" ":" "VTIMEZONE" CRLF
bool read_timezonec(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_key_value(is, "BEGIN", "VTIMEZONE") &&
                read_todoprop(is);
        if (!success_pro)
                return false;

        while(read_tzid(is) ||
              read_last_mod(is) ||
              read_tzurl(is) ||
              read_standardc(is) ||
              read_daylightc(is) ||
              read_x_prop(is) ||
              read_iana_prop(is)) {
        }

        const auto success_epi =
                read_key_value(is, "END", "VTIMEZONE") ;
        if (!success_epi)
                return false;

        ptran.commit();
        return true;
}

//       iana-comp  = "BEGIN" ":" iana-token CRLF
//                    1*contentline
//                    "END" ":" iana-token CRLF
bool read_iana_comp(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_token(is, "BEGIN") &&
                read_token(is, ":") &&
                read_iana_token(is) &&
                read_newline(is);
        if (!success_pro)
                return false;

        if (!read_contentline(is))
                return false;
        while(read_contentline(is)) {
        }

        const auto success_epi =
                read_token(is, "END") &&
                read_token(is, ":") &&
                read_iana_token(is) &&
                read_newline(is);
        if (!success_epi)
                return false;

        ptran.commit();
        return true;
}

//       x-comp     = "BEGIN" ":" x-name CRLF
//                    1*contentline
//                    "END" ":" x-name CRLF
bool read_x_comp(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_token(is, "BEGIN") &&
                read_token(is, ":") &&
                read_x_name(is) &&
                read_newline(is);
        if (!success_pro)
                return false;

        if (!read_contentline(is))
                return false;
        while(read_contentline(is)) {
        }

        const auto success_epi =
                read_token(is, "END") &&
                read_token(is, ":") &&
                read_x_name(is) &&
                read_newline(is);
        if (!success_epi)
                return false;

        ptran.commit();
        return true;
}

//       component  = 1*(eventc / todoc / journalc / freebusyc /
//                    timezonec / iana-comp / x-comp)
//
bool read_component_single(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_eventc(is)
                             || read_todoc(is)
                             || read_journalc(is)
                             || read_freebusyc(is)
                             || read_timezonec(is)
                             || read_iana_comp(is)
                             || read_x_comp(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
void expect_component_single(std::istream &is) {
        CALLSTACK;
        if (!read_component_single(is))
                throw syntax_error(is.tellg());
}
void expect_component(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_component_single(is);
        while(read_component_single(is)) {
        }
        ptran.commit();
}

bool read_dquoted_value(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        if (!read_dquote(is))
                return false;

        while (!read_dquote(is)) {
        }

        if (!read_dquote(is))
                return false;

        ptran.commit();
        return true;
}

// altrepparam = "ALTREP" "=" DQUOTE uri DQUOTE
bool read_altrepparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ALTREP") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

// cnparam    = "CN" "=" param-value
bool read_cnparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ALTREP") &&
                read_token(is, "=") &&
                read_param_value(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      cutypeparam        = "CUTYPE" "="
//                          ("INDIVIDUAL"   ; An individual
//                         / "GROUP"        ; A group of individuals
//                         / "RESOURCE"     ; A physical resource
//                         / "ROOM"         ; A room resource
//                         / "UNKNOWN"      ; Otherwise not known
//                         / x-name         ; Experimental type
//                         / iana-token)    ; Other IANA-registered
//                                          ; type
//       ; Default is INDIVIDUAL
bool read_cutypeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "CUTYPE") &&
                read_token(is, "=") &&
                read_param_value(is) &&
                (
                        read_token(is, "INDIVIDUAL") ||
                        read_token(is, "GROUP") ||
                        read_token(is, "RESOURCE") ||
                        read_token(is, "ROOM") ||
                        read_token(is, "UNKNOWN") ||
                        read_x_name(is) ||
                        read_iana_token(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//      delfromparam       = "DELEGATED-FROM" "=" DQUOTE cal-address DQUOTE
//                           *("," DQUOTE cal-address DQUOTE)
bool read_delfromparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DELEGATED-FROM") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        while (read_token(is, ",")) {
                if (!read_dquoted_value(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//      deltoparam = "DELEGATED-TO" "=" DQUOTE cal-address DQUOTE
//                    *("," DQUOTE cal-address DQUOTE)
bool read_deltoparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DELEGATED-TO") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        while (read_token(is, ",")) {
                if (!read_dquoted_value(is))
                        return false;
        }
        ptran.commit();
        return true;
}

//      dirparam   = "DIR" "=" DQUOTE uri DQUOTE
bool read_dirparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DIR") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

// encodingparam =
//          "ENCODING" "="
//          ( "8BIT"   ; "8bit" text encoding is defined in [RFC2045]
//          / "BASE64" ; "BASE64" binary encoding format is defined in [RFC4648]
//          )
bool read_encodingparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ENCODING") &&
                read_token(is, "=") &&
                (
                        read_token(is, "8BIT") ||
                        read_token(is, "BASE64")
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       fmttypeparam = "FMTTYPE" "=" type-name "/" subtype-name
//                      ; Where "type-name" and "subtype-name" are
//                      ; defined in Section 4.2 of [RFC4288].
bool read_fmttypeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "FMTTYPE") &&
                read_token(is, "=") &&
                (
                        read_type_name(is) ||
                        read_subtype_name(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       fbtypeparam        = "FBTYPE" "="
//                          ( "FREE"
//                          / "BUSY"
//                          / "BUSY-UNAVAILABLE"
//                          / "BUSY-TENTATIVE"
//                          / x-name
//                ; Some experimental iCalendar free/busy type.
//                          / iana-token
//                          )
//                ; Some other IANA-registered iCalendar free/busy type.
bool read_fbtypeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "FBTYPE") &&
                read_token(is, "=") &&
                (
                        read_token(is, "FREE") ||
                        read_token(is, "BUSY") ||
                        read_token(is, "BUSY-UNAVAILABLE") ||
                        read_token(is, "BUSY-TENTATIVE") ||
                        read_x_name(is) ||
                        read_iana_token(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       languageparam = "LANGUAGE" "=" language
//
//       language = Language-Tag
//                  ; As defined in [RFC5646].
bool read_languageparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LANGUAGE") &&
                read_token(is, "=") &&
                read_language_tag(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       memberparam        = "MEMBER" "=" DQUOTE cal-address DQUOTE
//                            *("," DQUOTE cal-address DQUOTE)
bool read_memberparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LANGUAGE") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        while (read_token(is, ",")) {
                if (!read_dquoted_value(is))
                        return false;
        }
        ptran.commit();
        return true;
}


//       partstat-jour    = ("NEEDS-ACTION"    ; Journal needs action
//                        / "ACCEPTED"         ; Journal accepted
//                        / "DECLINED"         ; Journal declined
//                        / x-name             ; Experimental status
//                        / iana-token)        ; Other IANA-registered
//                                             ; status
//       ; These are the participation statuses for a "VJOURNAL".
//       ; Default is NEEDS-ACTION.
bool read_partstat_jour(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//
//       partstat-todo    = ("NEEDS-ACTION"    ; To-do needs action
//                        / "ACCEPTED"         ; To-do accepted
//                        / "DECLINED"         ; To-do declined
//                        / "TENTATIVE"        ; To-do tentatively
//                                             ; accepted
//                        / "DELEGATED"        ; To-do delegated
//                        / "COMPLETED"        ; To-do completed
//                                             ; COMPLETED property has
//                                             ; DATE-TIME completed
//                        / "IN-PROCESS"       ; To-do in process of
//                                             ; being completed
//                        / x-name             ; Experimental status
//                        / iana-token)        ; Other IANA-registered
//                                             ; status
//       ; These are the participation statuses for a "VTODO".
//       ; Default is NEEDS-ACTION.
bool read_partstat_todo(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       partstat-event   = ("NEEDS-ACTION"    ; Event needs action
//                        / "ACCEPTED"         ; Event accepted
//                        / "DECLINED"         ; Event declined
//                        / "TENTATIVE"        ; Event tentatively
//                                             ; accepted
//                        / "DELEGATED"        ; Event delegated
//                        / x-name             ; Experimental status
//                        / iana-token)        ; Other IANA-registered
//                                             ; status
//       ; These are the participation statuses for a "VEVENT".
//       ; Default is NEEDS-ACTION.
bool read_partstat_event(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       partstatparam    = "PARTSTAT" "="
//                         (partstat-event
//                        / partstat-todo
//                        / partstat-jour)
bool read_partstatparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "PARTSTAT") &&
                read_token(is, "=") &&
                (
                        read_partstat_event(is) ||
                        read_partstat_todo(is) ||
                        read_partstat_jour(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       rangeparam = "RANGE" "=" "THISANDFUTURE"
//       ; To specify the instance specified by the recurrence identifier
//       ; and all subsequent recurrence instances.
bool read_rangeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RANGE") &&
                read_token(is, "=") &&
                read_token(is, "THISANDFUTURE");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       trigrelparam       = "RELATED" "="
//                          ( "START"       ; Trigger off of start
//                          / "END")        ; Trigger off of end
bool read_trigrelparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RELATED") &&
                read_token(is, "=") &&
                (
                        read_token(is, "START") ||
                        read_token(is, "END")
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       roleparam  = "ROLE" "="
//                   ("CHAIR"             ; Indicates chair of the
//                                        ; calendar entity
//                  / "REQ-PARTICIPANT"   ; Indicates a participant whose
//                                        ; participation is required
//                  / "OPT-PARTICIPANT"   ; Indicates a participant whose
//                                        ; participation is optional
//                  / "NON-PARTICIPANT"   ; Indicates a participant who
//                                        ; is copied for information
//                                        ; purposes only
//                  / x-name              ; Experimental role
//                  / iana-token)         ; Other IANA role
//       ; Default is REQ-PARTICIPANT
bool read_roleparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ROLE") &&
                read_token(is, "=") &&
                (
                        read_token(is, "CHAIR") ||
                        read_token(is, "REQ-PARTICIPANT") ||
                        read_token(is, "OPT-PARTICIPANT") ||
                        read_token(is, "NON-PARTICIPANT") ||
                        read_x_name(is) ||
                        read_iana_token(is)
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       rsvpparam = "RSVP" "=" ("TRUE" / "FALSE")
//       ; Default is FALSE
bool read_rsvpparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RSVP") &&
                read_token(is, "=") &&
                (
                        read_token(is, "TRUE") ||
                        read_token(is, "FALSE")
                );
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       sentbyparam        = "SENT-BY" "=" DQUOTE cal-address DQUOTE
bool read_sentbyparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "SENT-BY") &&
                read_token(is, "=") &&
                read_dquoted_value(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       tzidprefix = "/"
bool read_tzidprefix(std::istream &is) {
        CALLSTACK;
        return read_token(is, "/");
}
bool read_optional_tzidprefix(std::istream &is) {
        CALLSTACK;
        read_tzidprefix(is);
        return true;
}

//       tzidparam  = "TZID" "=" [tzidprefix] paramtext
bool read_tzidparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "TZID") &&
                read_token(is, "=") &&
                read_optional_tzidprefix(is) &&
                read_paramtext(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       valuetype  =("BINARY"
//                  / "BOOLEAN"
//                  / "CAL-ADDRESS"
//                  / "DATE"
//                  / "DATE-TIME"
//                  / "DURATION"
//                  / "FLOAT"
//                  / "INTEGER"
//                  / "PERIOD"
//                  / "RECUR"
//                  / "TEXT"
//                  / "TIME"
//                  / "URI"
//                  / "UTC-OFFSET"
//                  / x-name
//                  ; Some experimental iCalendar value type.
//                  / iana-token)
//                  ; Some other IANA-registered iCalendar value type.
bool read_valuetype(std::istream &is) {
        CALLSTACK;
        return read_token(is, "BINARY") ||
               read_token(is, "BOOLEAN") ||
               read_token(is, "CAL-ADDRESS") ||
               read_token(is, "DATE") ||
               read_token(is, "DATE-TIME") ||
               read_token(is, "DURATION") ||
               read_token(is, "FLOAT") ||
               read_token(is, "INTEGER") ||
               read_token(is, "PERIOD") ||
               read_token(is, "RECUR") ||
               read_token(is, "TEXT") ||
               read_token(is, "TIME") ||
               read_token(is, "URI") ||
               read_token(is, "UTC-OFFSET") ||
               read_x_name(is) ||
               read_iana_token(is);
}

//       valuetypeparam = "VALUE" "=" valuetype
bool read_valuetypeparam(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "VALUE") &&
                read_token(is, "=") &&
                read_valuetype(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
bool read_iana_param(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
bool read_x_param(std::istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}


//     icalparameter = altrepparam       ; Alternate text representation
//                   / cnparam           ; Common name
//                   / cutypeparam       ; Calendar user type
//                   / delfromparam      ; Delegator
//                   / deltoparam        ; Delegatee
//                   / dirparam          ; Directory entry
//                   / encodingparam     ; Inline encoding
//                   / fmttypeparam      ; Format type
//                   / fbtypeparam       ; Free/busy time type
//                   / languageparam     ; Language for text
//                   / memberparam       ; Group or list membership
//                   / partstatparam     ; Participation status
//                   / rangeparam        ; Recurrence identifier range
//                   / trigrelparam      ; Alarm trigger relationship
//                   / reltypeparam      ; Relationship type
//                   / roleparam         ; Participation role
//                   / rsvpparam         ; RSVP expectation
//                   / sentbyparam       ; Sent by
//                   / tzidparam         ; Reference to time zone object
//                   / valuetypeparam    ; Property value data type
//                   / other-param
void expect_icalparameter(std::istream &is) {
        CALLSTACK;
        if (!read_icalparameter(is))
                throw syntax_error(is.tellg());
}
bool read_icalparameter(std::istream &is) {
        CALLSTACK;
        return read_altrepparam(is)
               || read_cnparam(is)
               || read_cutypeparam(is)
               || read_delfromparam(is)
               || read_deltoparam(is)
               || read_dirparam(is)
               || read_encodingparam(is)
               || read_fmttypeparam(is)
               || read_fbtypeparam(is)
               || read_languageparam(is)
               || read_memberparam(is)
               || read_partstatparam(is)
               || read_rangeparam(is)
               || read_trigrelparam(is)
               || read_reltypeparam(is)
               || read_roleparam(is)
               || read_rsvpparam(is)
               || read_sentbyparam(is)
               || read_tzidparam(is)
               || read_valuetypeparam(is)
               || read_other_param(is);
}
