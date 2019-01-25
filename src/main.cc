// RFCs:
// - ABNF:
//   - [RFC 5234](https://tools.ietf.org/html/rfc5234)
// - ICalendar:
//   - [RFC 5545](https://tools.ietf.org/html/rfc5545)
//   - [RFC 5546](https://tools.ietf.org/html/rfc5546)
//   - [RFC 6868](https://tools.ietf.org/html/rfc6868)
//   - [RFC 7529](https://tools.ietf.org/html/rfc7529)
//   - [RFC 7986](https://tools.ietf.org/html/rfc7986)

#include <istream>
#include <iostream> // TODO: Remove iostream include.
#include <string>

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
        syntax_error() : std::runtime_error("Syntax error.") {}
        syntax_error(std::string const &msg) : std::runtime_error(msg) {}
};

class unexpected_token : public syntax_error {
public:
        unexpected_token() : syntax_error() {}
        unexpected_token(std::istream const &is, string const &tok)
                : syntax_error("Expected token '" + tok + "' not found")
        {}
};

class key_value_pair_expected : public syntax_error {
public:
        key_value_pair_expected()
                : syntax_error("Expected key-value-pair") {}

        key_value_pair_expected(std::istream const &is,
                                string const &k,
                                string const &v)
                : syntax_error("Expected key-value-pair '" + k + ":" + v + "'")
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
                throw unexpected_token();
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
                throw syntax_error();
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
                throw syntax_error("expected digit");
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
                throw syntax_error("expected alpha or digit");
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
void expect_content_line(std::istream &);
void expect_name(std::istream &);

bool read_iana_token(std::istream &);
void expect_iana_token(std::istream &);

bool read_vendorid(std::istream &);
void expect_param(std::istream &);
void expect_param_name(std::istream &);
void expect_param_value(std::istream &);
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
void expect_content_line(std::istream &is) {
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

//     name          = iana-token / x-name
void expect_name(std::istream &is) {
        const auto success =
                read_iana_token(is) ||
                read_x_name(is);
        if (!success) {
                throw syntax_error("expected iana-token or x-name");
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
        return true;
}
void expect_iana_token(std::istream &is) {
        if (!read_iana_token(is))
                throw unexpected_token();
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
                throw syntax_error("expected param-name");
        }
}


//     param-value   = paramtext / quoted-string
void expect_param_value(std::istream &is) {
        const auto success =
                read_paramtext(is) ||
                read_quoted_string(is);
        if (!success) {
                throw syntax_error("expected param-value");
        }
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
        std::cerr << "todo: non-us-ascii" << std::endl;
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
        save_input_pos ptran(is);
        while (read_calprop(is))
                ;
        std::cerr << "expect_calprops() does not test semantics\n";
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
                throw syntax_error();

        // [vendorid "-"]
        if (read_vendorid(is)) {
                expect_token(is, "-");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (!read_alnum(is) && !read_token(is, "-"))
                throw syntax_error();
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


bool read_iana_prop(std::istream &is) {
        NOT_IMPLEMENTED;
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
        std::cerr << "todo: non-us-ascii" << std::endl;
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

//       component  = 1*(eventc / todoc / journalc / freebusyc /
//                    timezonec / iana-comp / x-comp)
//
//       iana-comp  = "BEGIN" ":" iana-token CRLF
//                    1*contentline
//                    "END" ":" iana-token CRLF
//
//       x-comp     = "BEGIN" ":" x-name CRLF
//                    1*contentline
//                    "END" ":" x-name CRLF
void expect_component(std::istream &is) {
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
        NOT_IMPLEMENTED;
}
bool read_icalparameter(std::istream &is) {
        try {
                expect_icalparameter(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

#include <fstream>
int main() {
        std::ifstream f("dev-assets/f1calendar.com/2019_full.ics");
        try {
                expect_ical(f);
        } catch (std::exception &e) {
                std::cerr << e.what();
        }
        return 0;
}
