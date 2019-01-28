// RFCs:
// - Media Types:
//   - [RFC 4288](https://tools.ietf.org/html/rfc4288)
// - ABNF:
//   - [RFC 5234](https://tools.ietf.org/html/rfc5234)
// - ICalendar:
//   - [RFC 5545](https://tools.ietf.org/html/rfc5545)
//   - [RFC 5546](https://tools.ietf.org/html/rfc5546)
//   - [RFC 6868](https://tools.ietf.org/html/rfc6868)
//   - [RFC 7529](https://tools.ietf.org/html/rfc7529)
//   - [RFC 7986](https://tools.ietf.org/html/rfc7986)
// - Tags for Identifying Languages:
//   - [RFC 5646](https://tools.ietf.org/html/rfc5646)
#include <istream>
#include <iostream> // TODO: Remove iostream include.
#include <string>
#include <vector>

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

/*
CallStack callStack;
#define CALLSTACK CallStack::Entry call_stack_entry_##__LINE__ = callStack.push(__func__);
//*/
#define CALLSTACK

#define NOT_IMPLEMENTED do{throw not_implemented(__func__);}while(false)

// -- Typedefs. ----------------------------------------------------------------
using string = std::string;


// -- Exceptions. --------------------------------------------------------------
class invalid_ical : public std::runtime_error {
public:
        invalid_ical() : std::runtime_error("Invalid ICalendar file") {}
};

class not_implemented : public std::runtime_error {
public:
        not_implemented(std::string const &what) :
                std::runtime_error("Not implemented: " + what) {}
};

class syntax_error : public std::runtime_error {
public:
        syntax_error(std::istream &is) :
                std::runtime_error("Syntax error."),
                pos(is.tellg())
                {}

        syntax_error(std::istream &is, std::string const &msg) :
                std::runtime_error(msg),
                pos(is.tellg())
                {}

        std::istream::pos_type pos = 0;
};

class unexpected_token : public syntax_error {
public:
        unexpected_token(std::istream &is) :
                syntax_error(is)
                {}

        unexpected_token(std::istream &is, string const &tok) :
                syntax_error(is, "Expected token '" + tok + "' not found")
                {}
};

class key_value_pair_expected : public syntax_error {
public:
        key_value_pair_expected(std::istream &is) :
                syntax_error(is, "Expected key-value-pair")
                {}

        key_value_pair_expected(std::istream &is,
                                string const &k,
                                string const &v)
                : syntax_error(
                        is,
                        "Expected key-value-pair '" + k + ":" + v + "'")
                {}
};


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
                        throw unexpected_token(is, tok);
                }
                if(i != c) {
                        throw unexpected_token(is, tok);
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

void expect_newline(std::istream &is) {
        // Even though RFC 5545 says just "CRLF", we also handle "CR" and "LF".
        if (!read_token(is, "\r\n") &&
            !read_token(is, "\n") &&
            !read_token(is, "\r"))
        {
                throw unexpected_token(is);
        }
}

bool read_newline(std::istream &is) {
        try {
                expect_newline(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

void expect_alpha(std::istream &is) {
        save_input_pos ptran(is);
        const auto i = is.get();
        switch(i) {
        default:
        case EOF:
                throw syntax_error(is);
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
        try {
                expect_alpha(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}


void expect_digit(std::istream &is) {
        save_input_pos ptran(is);
        const auto i = is.get();
        if (i<'0' || i>'9')
                throw syntax_error(is, "expected digit");
        ptran.commit();
}

bool read_digit(std::istream &is) {
        try {
                expect_digit(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

void expect_alnum(std::istream &is) {
        save_input_pos ptran(is);
        const auto success = read_alpha(is) || read_digit(is);
        if (!success) {
                throw syntax_error(is, "expected alpha or digit");
        }
        ptran.commit();
}

bool read_alnum(std::istream &is) {
        try {
                expect_alnum(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

// -- Media Type (RFC 4288) Parser Helpers. ------------------------------------
//       type-name = reg-name
bool read_type_name(std::istream &) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       subtype-name = reg-name
bool read_subtype_name(std::istream &) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name = 1*127reg-name-chars
bool read_reg_name(std::istream &) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       reg-name-chars = ALPHA / DIGIT / "!" /
//                       "#" / "$" / "&" / "." /
//                       "+" / "-" / "^" / "_"
bool read_reg_name_char(std::istream &) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

// -- Tags for Identifying Languages (RFC 5646) Parser Helpers. ----------------

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
bool read_language_tag(std::istream &) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

// -- ABNF (RFC 5234) Parser Helpers. ------------------------------------------
bool read_htab(std::istream &is) {
        // HTAB = %x09
        return read_token(is, "\t");
}

bool read_sp(std::istream &is) {
        // SP = %x20
        return read_token(is, " ");
}

bool read_wsp(std::istream &is) {
        // WSP = SP / HTAB ; white space
        return read_sp(is) || read_htab(is);
}

// -- Parser helpers specific to ICal, but generic within ICal. ----------------
void expect_key_value(std::istream &is, string const &k, string const &v) {
        save_input_pos ptran(is);
        const auto success =
                read_token(is, k) &&
                read_token(is, ":") &&
                read_token(is, v) &&
                read_newline(is);
        if (!success)
                throw key_value_pair_expected(is, k, v);
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

void expect_icalbody(std::istream &);

void expect_text(std::istream &);
bool read_text(std::istream &);

void expect_calprops(std::istream &is);
bool read_prodid(std::istream &);
void expect_prodid(std::istream &);
void expect_pidparam(std::istream &);
void expect_other_param(std::istream &);

void expect_pidvalue(std::istream &);

bool read_version(std::istream &);
void expect_version(std::istream &);
void expect_vervalue(std::istream &);
void expect_verparam(std::istream &);

bool read_calscale(std::istream &);
void expect_calscale(std::istream &);
void expect_calparam(std::istream &);
void expect_calvalue(std::istream &);

bool read_method(std::istream &);
void expect_method(std::istream &);
void expect_metparam(std::istream &);
void expect_metvalue(std::istream &);

bool read_iana_prop(std::istream &);
void expect_iana_prop(std::istream &);

void expect_component(std::istream &is);
void expect_contentline(std::istream &is);
void expect_name(std::istream &);

bool read_iana_token(std::istream &);
void expect_iana_token(std::istream &);

bool read_vendorid(std::istream &);
void expect_param(std::istream &);
void expect_param_name(std::istream &);
void expect_param_value(std::istream &);
bool read_param_value(std::istream &);
bool read_paramtext(std::istream &);
bool read_quoted_string(std::istream &);
bool read_safe_char(std::istream &);
bool read_value_char(std::istream &);
bool read_qvalue_char(std::istream &);
bool read_non_us_ascii(std::istream &);
bool read_control(std::istream &);
bool read_dquote(std::istream &is);
bool read_wsp(std::istream &is);
void expect_value(std::istream &);

bool read_x_prop(std::istream &);
void expect_x_prop(std::istream &);
bool read_x_name(std::istream &);
void expect_x_name(std::istream &);

bool read_icalparameter(std::istream &);
void expect_icalparameter(std::istream &);


//    contentline   = name *(";" param ) ":" value CRLF
void expect_contentline(std::istream &is) {
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
        try {
                expect_contentline(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//     name          = iana-token / x-name
void expect_name(std::istream &is) {
        const auto success =
                read_iana_token(is) ||
                read_x_name(is);
        if (!success) {
                throw syntax_error(is, "expected iana-token or x-name");
        }
}

//     iana-token    = 1*(ALPHA / DIGIT / "-")
//     ; iCalendar identifier registered with IANA
bool read_iana_token_char(std::istream &is) {
        return read_alnum(is) || read_token(is, "-");
}
bool read_iana_token(std::istream &is) {
        if (!read_iana_token_char(is))
                return false;
        while (read_iana_token_char(is))
                ;
        // TODO: IANA iCalendar identifiers
        return true;
}
void expect_iana_token(std::istream &is) {
        if (!read_iana_token(is))
                throw unexpected_token(is);
}

//     vendorid      = 3*(ALPHA / DIGIT)
//     ; Vendor identification
void expect_vendorid(std::istream &is) {
        save_input_pos ptran(is);

        expect_alnum(is);
        expect_alnum(is);
        expect_alnum(is);

        while (read_alnum(is)) {
        }

        ptran.commit();
}
bool read_vendorid(std::istream &is) {
        try {
                expect_vendorid(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//     param         = param-name "=" param-value *("," param-value)
//     ; Each property defines the specific ABNF for the parameters
//     ; allowed on the property.  Refer to specific properties for
//     ; precise parameter ABNF.
//
void expect_param(std::istream &is) {
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
        const auto success =
                read_iana_token(is) ||
                read_x_name(is);
        if (!success) {
                throw syntax_error(is, "expected param-name");
        }
}


//     param-value   = paramtext / quoted-string
bool read_param_value(std::istream &is) {
        return read_paramtext(is) || read_quoted_string(is);
}
void expect_param_value(std::istream &is) {
        if (!read_param_value(is))
                throw syntax_error(is);
}

//     paramtext     = *SAFE-CHAR
bool read_paramtext(std::istream &is) {
        while (read_safe_char(is))
                ;
        return true;
}

//     quoted-string = DQUOTE *QSAFE-CHAR DQUOTE
bool read_quoted_string(std::istream &is) {
        NOT_IMPLEMENTED;
}

//     SAFE-CHAR     = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-7E
//                   / NON-US-ASCII
//     ; Any character except CONTROL, DQUOTE, ";", ":", ","
bool read_safe_char(std::istream &) {
        NOT_IMPLEMENTED;
}

//     VALUE-CHAR    = WSP / %x21-7E / NON-US-ASCII
//     ; Any textual character
bool read_value_char(std::istream &is) {
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
bool read_qsafe_char(std::istream &) {
        NOT_IMPLEMENTED;
}

//     NON-US-ASCII  = UTF8-2 / UTF8-3 / UTF8-4
//     ; UTF8-2, UTF8-3, and UTF8-4 are defined in [RFC3629]
bool read_non_us_ascii(std::istream &) {
        NOT_IMPLEMENTED;
}

//     CONTROL       = %x00-08 / %x0A-1F / %x7F
//     ; All the controls except HTAB
bool read_control(std::istream &) {
        NOT_IMPLEMENTED;
}

//     value         = *VALUE-CHAR
void expect_value(std::istream &is) {
        while (read_value_char(is)) {
        }
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
        std::cerr << "expect_calprops()...\n";
        expect_calprops(is);
        std::cerr << "expect_component()...\n";
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
        save_input_pos ptran(is);
        expect_token(is, "PRODID");
        expect_pidparam(is);
        expect_token(is, ":");
        expect_pidvalue(is);
        expect_newline(is);
        ptran.commit();
        std::cerr << " <-- PRODID" << std::endl;
}
bool read_prodid(std::istream &is) {
        try {
                expect_prodid(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       version    = "VERSION" verparam ":" vervalue CRLF
void expect_version(std::istream &is) {
        save_input_pos ptran(is);
        expect_token(is, "VERSION");
        expect_verparam(is);
        expect_token(is, ":");
        expect_vervalue(is);
        expect_newline(is);
        ptran.commit();

        std::cerr << " <-- VERSION" << std::endl;
}
bool read_version(std::istream &is) {
        try {
                expect_version(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       verparam   = *(";" other-param)
void expect_verparam(std::istream &is) {
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
        expect_text(is);
}

//       calscale   = "CALSCALE" calparam ":" calvalue CRLF
void expect_calscale(std::istream &is) {
        save_input_pos ptran(is);
        expect_token(is, "CALSCALE");
        expect_calparam(is);
        expect_token(is, ":");
        expect_calvalue(is);
        expect_newline(is);
        ptran.commit();
        std::cerr << " <-- CALSCALE" << std::endl;
}
bool read_calscale(std::istream &is) {
        try {
                expect_calscale(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       calparam   = *(";" other-param)
void expect_calparam(std::istream &is) {
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       calvalue   = "GREGORIAN"
void expect_calvalue(std::istream &is) {
        read_text(is);
}

//       method     = "METHOD" metparam ":" metvalue CRLF
bool read_method(std::istream &is) {
        try {
                expect_method(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}
void expect_method(std::istream &is) {
        save_input_pos ptran(is);
        expect_token(is, "METHOD");
        expect_metparam(is);
        expect_token(is, ":");
        expect_metvalue(is);
        expect_newline(is);
        ptran.commit();
        std::cerr << " <-- METHOD" << std::endl;
}

//       metparam   = *(";" other-param)
void expect_metparam(std::istream &is) {
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       metvalue   = iana-token
void expect_metvalue(std::istream &is) {
        expect_iana_token(is);
}

//       x-prop = x-name *(";" icalparameter) ":" value CRLF
void expect_x_prop(std::istream &is) {
        save_input_pos ptran(is);
        expect_x_name(is);
        while (read_token(is, ";"))
                expect_icalparameter(is);
        expect_token(is, ":");
        expect_value(is);
        expect_newline(is);
        ptran.commit();
        std::cerr << " <-- X-PROP" << std::endl;
}
bool read_x_prop(std::istream &is) {
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
        save_input_pos ts(is);

        // "X-"
        if (!read_token(is, "X-"))
                throw syntax_error(is);

        // [vendorid "-"]
        if (read_vendorid(is)) {
                expect_token(is, "-");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (!read_alnum(is) && !read_token(is, "-"))
                throw syntax_error(is);
        while (read_alnum(is) || read_token(is, "-")) {
        }

        ts.commit();
}
bool read_x_name(std::istream &is) {
        try {
                expect_x_name(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       iana-prop = iana-token *(";" icalparameter) ":" value CRLF
void expect_iana_prop(std::istream &is) {
        save_input_pos ptran(is);
        expect_iana_token(is);
        while (read_token(is, ";"))
                expect_icalparameter(is);
        expect_token(is, ":");
        expect_value(is);
        expect_newline(is);
        ptran.commit();
        std::cerr << " <-- IANA-PROP" << std::endl;
}
bool read_iana_prop(std::istream &is) {
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
        NOT_IMPLEMENTED;
}

//       pidvalue   = text
//       ;Any text that describes the product and version
//       ;and that is generally assured of being unique.
void expect_pidvalue(std::istream &is) {
        expect_text(is);
}


//       ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
//          ; \\ encodes \, \N or \n encodes newline
//          ; \; encodes ;, \, encodes ,
bool read_escaped_char(std::istream &is) {
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
        return read_tsafe_char(is)
            || read_token(is, ":")
            || read_dquote(is)
            || read_escaped_char(is);
}
void expect_text(std::istream &is) {
        while(read_text_char(is)) {
        }
}

// DQUOTE: ASCII 22
bool read_dquote(std::istream &is) {
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != 22)
                return false;
        ptran.commit();
        return true;
}

bool read_text(std::istream &is) {
        try {
                expect_text(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
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
//       alarmc     = "BEGIN" ":" "VALARM" CRLF
//                    (audioprop / dispprop / emailprop)
//                    "END" ":" "VALARM" CRLF
bool read_alarmc(std::istream &is) {
        NOT_IMPLEMENTED;
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
bool read_eventprop(std::istream &is) {
        std::cerr << " read_eventprop\n";
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       eventc     = "BEGIN" ":" "VEVENT" CRLF
//                    eventprop *alarmc
//                    "END" ":" "VEVENT" CRLF
bool read_eventc(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        std::cerr << " read_eventc\n";
        const auto success_pro =
                read_key_value(is, "BEGIN", "VEVENT") &&
                read_eventprop(is);
        if (!success_pro) {
                std::cerr << " read_eventc: success_pro=false\n";
                return false;
        }
        std::cerr << " read_eventc: success_pro=true\n";

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
        NOT_IMPLEMENTED;
}

//       todoc      = "BEGIN" ":" "VTODO" CRLF
//                    todoprop *alarmc
//                    "END" ":" "VTODO" CRLF
bool read_todoc(std::istream &is) {
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

        std::cerr << " <-- TODO" << std::endl;
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
        NOT_IMPLEMENTED;
}

//       freebusyc  = "BEGIN" ":" "VFREEBUSY" CRLF
//                    fbprop
//                    "END" ":" "VFREEBUSY" CRLF
bool read_freebusyc(std::istream &is) {
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
        NOT_IMPLEMENTED;
}

bool read_last_mod(std::istream &is) {
        NOT_IMPLEMENTED;
}

bool read_tzurl(std::istream &is) {
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
        NOT_IMPLEMENTED;
}

//       daylightc  = "BEGIN" ":" "DAYLIGHT" CRLF
//                    tzprop
//                    "END" ":" "DAYLIGHT" CRLF
bool read_daylightc(std::istream &is) {
        NOT_IMPLEMENTED;
}

//       standardc  = "BEGIN" ":" "STANDARD" CRLF
//                    tzprop
//                    "END" ":" "STANDARD" CRLF
bool read_standardc(std::istream &is) {
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
                throw syntax_error(is);
}
void expect_component(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        std::cerr << " expect_component: 1st\n";
        expect_component_single(is);
        std::cerr << " got 1st component\n";
        while(read_component_single(is)) {
                std::cerr << " expect_component: n'th\n";
        }
        ptran.commit();
}

bool read_dquoted_value(std::istream &is) {
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
        NOT_IMPLEMENTED;
}

//       partstatparam    = "PARTSTAT" "="
//                         (partstat-event
//                        / partstat-todo
//                        / partstat-jour)
bool read_partstatparam(std::istream &is) {
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

//       reltypeparam       = "RELTYPE" "="
//                           ("PARENT"    ; Parent relationship - Default
//                          / "CHILD"     ; Child relationship
//                          / "SIBLING"   ; Sibling relationship
//                          / iana-token  ; Some other IANA-registered
//                                        ; iCalendar relationship type
//                          / x-name)     ; A non-standard, experimental
//                                        ; relationship type
bool read_reltypeparam(std::istream &is) {
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RELTYPE") &&
                read_token(is, "=") &&
                (
                        read_token(is, "PARENT") ||
                        read_token(is, "CHILD") ||
                        read_token(is, "SIBLING") ||
                        read_iana_token(is) ||
                        read_x_name(is)
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
        return read_token(is, "/");
}
bool read_optional_tzidprefix(std::istream &is) {
        read_tzidprefix(is);
        return true;
}

//       tzidparam  = "TZID" "=" [tzidprefix] paramtext
bool read_tzidparam(std::istream &is) {
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
        NOT_IMPLEMENTED;
}

//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
bool read_x_param(std::istream &is) {
        NOT_IMPLEMENTED;
}


//     other-param   = (iana-param / x-param)
bool read_other_param(std::istream &is) {
        return read_iana_param(is) || read_x_param(is);
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
        if (!read_icalparameter(is))
                throw syntax_error(is);
}
bool read_icalparameter(std::istream &is) {
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

#include <fstream>
int main() {
        CALLSTACK;
        std::ifstream f("dev-assets/f1calendar.com/2019_full.ics");
        try {
                expect_ical(f);
        } catch (syntax_error &e) {
                std::cerr << "syntax-error:" << e.what();

                const auto where = e.pos;
                f.seekg(0);
                int line = 1, col = 1;
                while(f.tellg() < where) {
                        if(read_newline(f)) {
                                col = 1;
                                ++line;
                        } else {
                                auto g = f.get();
                                //std::cout << "[" << (char)g << "]";
                        }
                }
                std::cerr << " (in line " << line << ":" << col << ")\n";
        } catch (std::exception &e) {
                std::cerr << "unknown error:" << e.what();
        }
        return 0;
}
