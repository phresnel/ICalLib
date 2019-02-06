#include <iostream>
#include "rfc3986.hh"
#include "rfc4288.hh"
#include "rfc5234.hh"
#include "rfc5646.hh"
#include "parser.hh"
#include "parser_helpers.hh"


// -- Parser helpers specific to ICal, but generic within ICal. ----------------
tuple<string, string> expect_key_value(
        istream &is,
        string const &k,
        string const &v
) {
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
        return {k, v};
}

optional<tuple<string, string>> read_key_value(
        istream &is, string const &k, string const &v
) {
        CALLSTACK;
        try {
                return expect_key_value(is, k, v);
        } catch (syntax_error &) {
                return nullopt;
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

//    contentline   = name *(";" param ) ":" value CRLF
ContentLine expect_contentline(istream &is) {
        CALLSTACK;
        save_input_pos ts(is);
        ContentLine ret;
        ret.name = expect_name(is);
        while (read_token(is, ";")) {
                ret.params.push_back(expect_param(is));
        }
        expect_token(is, ":");
        ret.value = expect_value(is);
        expect_newline(is);
        ts.commit();

        std::cout << ret << std::endl;
        return ret;
}
optional<ContentLine> read_contentline(istream &is) {
        CALLSTACK;
        try {
                return expect_contentline(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//     name          = iana-token / x-name
string expect_name(istream &is) {
        CALLSTACK;
        if (auto v = read_iana_token(is))
                return *v;
        if (auto v = read_x_name(is))
                return *v;
        throw syntax_error(is.tellg(), "expected iana-token or x-name");
}

//     iana-token    = 1*(ALPHA / DIGIT / "-")
//     ; iCalendar identifier registered with IANA
optional<string> read_iana_token_char(istream &is) {
        CALLSTACK;
        if (auto v = read_alnum(is)) return *v;
        if (auto v = read_token(is, "-")) return *v;
        return nullopt;
}
optional<string> read_iana_token(istream &is) {
        CALLSTACK;
        string ret;
        if (auto c = read_iana_token_char(is)) {
                ret += *c;
        } else {
                return nullopt;
        }
        while (auto c = read_iana_token_char(is))
                ret += *c;
        // TODO: IANA iCalendar identifiers
        return ret;
}
string expect_iana_token(istream &is) {
        CALLSTACK;
        if (auto v = read_iana_token(is))
                return *v;
        throw unexpected_token(is.tellg());
}

//     vendorid      = 3*(ALPHA / DIGIT)
//     ; Vendor identification
string expect_vendorid(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        string ret;
        ret += expect_alnum(is);
        ret += expect_alnum(is);
        ret += expect_alnum(is);

        while (auto v = read_alnum(is)) {
                ret += *v;
        }

        ptran.commit();
        return ret;
}
optional<string> read_vendorid(istream &is) {
        CALLSTACK;
        try {
                return expect_vendorid(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//     SAFE-CHAR     = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-7E / NON-US-ASCII
//     ; Any character except CONTROL, DQUOTE, ";", ":", ","
optional<string> read_safe_char(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (auto v = read_wsp(is)) {
                ptran.commit();
                return *v;
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
                return string() + char(i);
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return nullopt;
}

//     VALUE-CHAR    = WSP / %x21-7E / NON-US-ASCII
//     ; Any textual character
optional<string> read_value_char(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (auto v = read_wsp(is)) {
                ptran.commit();
                return v;
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
                return string() + char(i);
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return nullopt;
}

//     QSAFE-CHAR    = WSP / %x21 / %x23-7E / NON-US-ASCII
//     ; Any character except CONTROL and DQUOTE
optional<string> read_qsafe_char(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     NON-US-ASCII  = UTF8-2 / UTF8-3 / UTF8-4
//     ; UTF8-2, UTF8-3, and UTF8-4 are defined in [RFC3629]
optional<string> read_non_us_ascii(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     CONTROL       = %x00-08 / %x0A-1F / %x7F
//     ; All the controls except HTAB
optional<string> read_control(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     value         = *VALUE-CHAR
string expect_value(istream &is) {
        CALLSTACK;
        string ret;
        while (auto c = read_value_char(is)) {
                ret += *c;
        }
        return ret;
}


//     param         = param-name "=" param-value *("," param-value)
//     ; Each property defines the specific ABNF for the parameters
//     ; allowed on the property.  Refer to specific properties for
//     ; precise parameter ABNF.
//
Param expect_param(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Param ret;
        ret.name = expect_param_name(is);
        expect_token(is, "=");
        ret.values.push_back(expect_param_value(is));
        while (read_token(is, ","))
                ret.values.push_back(expect_param_value(is));
        ptran.commit();
        return ret;
}

//     param-name    = iana-token / x-name
string expect_param_name(istream &is) {
        CALLSTACK;
        if (auto v = read_iana_token(is))
                return *v;
        if (auto v = read_x_name(is))
                return *v;
        throw syntax_error(is.tellg(), "expected param-name");
}


//     param-value   = paramtext / quoted-string
optional<string> read_param_value(istream &is) {
        CALLSTACK;
        if (auto v = read_paramtext(is)) return *v;
        if (auto v = read_quoted_string(is)) return *v;
        return nullopt;
}
string expect_param_value(istream &is) {
        CALLSTACK;
        if (auto v = read_param_value(is))
                return *v;
        throw syntax_error(is.tellg());
}

//     paramtext     = *SAFE-CHAR
optional<string> read_paramtext(istream &is) {
        CALLSTACK;
        string ret;
        while (auto v = read_safe_char(is))
                ret += *v;
        return ret;
}

//     quoted-string = DQUOTE *QSAFE-CHAR DQUOTE
optional<string> read_quoted_string(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        if (!read_dquote(is))
                return nullopt;

        string ret;
        while (auto v = read_qsafe_char(is)) {
                ret += *v;
        }

        if (!read_dquote(is))
                return nullopt;

        ptran.commit();
        return ret;
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
Calendar expect_ical(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Calendar ret;
        expect_key_value(is, "BEGIN", "VCALENDAR");
        ret = expect_icalbody(is);
        expect_key_value(is, "END", "VCALENDAR");
        ptran.commit();
        return ret;
}

optional<Calendar> read_ical(istream &is) {
        CALLSTACK;
        try {
                return expect_ical(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       icalbody   = calprops component
Calendar expect_icalbody(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Calendar ret;
        ret.properties = expect_calprops(is);
        ret.components = expect_component(is);
        ptran.commit();
        return ret;
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
CalProps expect_calprops(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        CalProps ret;
        int prodidc = 0,
            versionc = 0,
            calscalec = 0,
            methodc = 0;
        while (true) {
                if (auto val = read_prodid(is)) {
                        ret.prodId = *val;
                        ++prodidc;
                } else if (read_version(is)) {
                        // ret.version = ...
                        ++versionc;
                } else if (read_calscale(is)) {
                        ++calscalec;
                } else if (read_method(is)) {
                        ++methodc;
                } else if (read_x_prop(is)) {
                } else if (read_iana_prop(is)) {
                } else {
                        break;
                }
        }
        ptran.commit();
        return ret;
}

//       prodid     = "PRODID" pidparam ":" pidvalue CRLF
ProdId expect_prodid(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        ProdId ret;
        expect_token(is, "PRODID");
        ret.params = expect_pidparam(is);
        expect_token(is, ":");
        ret.value = expect_pidvalue(is);
        expect_newline(is);
        ptran.commit();
        return ret;
}
optional<ProdId> read_prodid(istream &is) {
        CALLSTACK;
        try {
                return expect_prodid(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       version    = "VERSION" verparam ":" vervalue CRLF
Version expect_version(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Version ret;
        expect_token(is, "VERSION");
        ret.params = expect_verparam(is);
        expect_token(is, ":");
        ret.value = expect_vervalue(is);
        expect_newline(is);
        ptran.commit();

        std::cout << "Version read: " << ret << std::endl;
        return ret;
}
optional<Version> read_version(istream &is) {
        CALLSTACK;
        try {
                return expect_version(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       verparam   = *(";" other-param)
std::vector<OtherParam> expect_verparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        std::vector<OtherParam> ret;
        while(read_token(is, ";")) {
                ret.push_back(expect_other_param(is));
        }
        ptran.commit();
        return ret;
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
string expect_vervalue(istream &is) {
        CALLSTACK;
        return expect_text(is);
}

//       calscale   = "CALSCALE" calparam ":" calvalue CRLF
void expect_calscale(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_token(is, "CALSCALE");
        expect_calparam(is);
        expect_token(is, ":");
        expect_calvalue(is);
        expect_newline(is);
        ptran.commit();
}
bool read_calscale(istream &is) {
        CALLSTACK;
        try {
                expect_calscale(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}

//       calparam   = *(";" other-param)
void expect_calparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       calvalue   = "GREGORIAN"
void expect_calvalue(istream &is) {
        CALLSTACK;
        expect_token(is, "GREGORIAN");
}

//       method     = "METHOD" metparam ":" metvalue CRLF
bool read_method(istream &is) {
        CALLSTACK;
        try {
                expect_method(is);
                return true;
        } catch (syntax_error &) {
                return false;
        }
}
void expect_method(istream &is) {
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
void expect_metparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_token(is, ";"))
                expect_other_param(is);
        ptran.commit();
}

//       metvalue   = iana-token
void expect_metvalue(istream &is) {
        CALLSTACK;
        expect_iana_token(is);
}

//       x-prop = x-name *(";" icalparameter) ":" value CRLF
XProp expect_x_prop(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        XProp ret;
        ret.name = expect_x_name(is);
        while (read_token(is, ";")) {
                ret.parameters.push_back(expect_icalparameter(is));
        }
        expect_token(is, ":");
        ret.value = expect_value(is);
        expect_newline(is);
        ptran.commit();
        return ret;
}
optional<XProp> read_x_prop(istream &is) {
        CALLSTACK;
        try {
                return expect_x_prop(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}


//     x-name        = "X-" [vendorid "-"] 1*(ALPHA / DIGIT / "-")
//     ; Reserved for experimental use.
string expect_x_name(istream &is) {
        CALLSTACK;
        save_input_pos ts(is);

        string ret;

        // "X-"
        if (auto val = read_token(is, "X-")) {
                ret += *val;
        } else {
                throw syntax_error(is.tellg());
        }

        // [vendorid "-"]
        if (auto val = read_vendorid(is)) {
                ret += *val;
                ret += expect_token(is, "-");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (auto v = read_alnum(is)) {
                ret += *v;
        } else if (auto v = read_token(is, "-")) {
                ret += *v;
        } else {
                throw syntax_error(is.tellg());
        }
        while (true) {
                if (auto v = read_alnum(is)) {
                        ret += *v;
                } else if (auto v = read_token(is, "-")) {
                        ret += *v;
                } else {
                        break;
                }
        }

        ts.commit();
        return ret;
}
optional<string> read_x_name(istream &is) {
        CALLSTACK;
        try {
                return expect_x_name(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       iana-prop = iana-token *(";" icalparameter) ":" value CRLF
void expect_iana_prop(istream &is) {
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
optional<IanaProp> read_iana_prop(istream &is) {
        CALLSTACK;
        return nullopt; // TODO
        NOT_IMPLEMENTED;
        try {
                expect_iana_prop(is);
                //return true;
        } catch (syntax_error &) {
                //return false;
        }
}

//       pidparam   = *(";" other-param)
std::vector<OtherParam> expect_pidparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        std::vector<OtherParam> ret;
        while (read_token(is, ";"))
                ret.push_back(expect_other_param(is));
        ptran.commit();
        return ret;
}

//     other-param   = (iana-param / x-param)
//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
OtherParam expect_other_param(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       pidvalue   = text
//       ;Any text that describes the product and version
//       ;and that is generally assured of being unique.
string expect_pidvalue(istream &is) {
        CALLSTACK;
        return expect_text(is);
}


//       ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
//          ; \\ encodes \, \N or \n encodes newline
//          ; \; encodes ;, \, encodes ,
optional<string> read_escaped_char(istream &is) {
        CALLSTACK;
        if (auto val = read_token(is, "\\\\"))
                return val;
        if (auto val = read_token(is, "\\;"))
                return val;
        if (auto val = read_token(is, "\\,"))
                return val;
        if (auto val = read_token(is, "\\N"))
                return val;
        if (auto val = read_token(is, "\\n"))
                return val;
        return nullopt;
}

//       TSAFE-CHAR = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-5B /
//                    %x5D-7E / NON-US-ASCII
//          ; Any character except CONTROLs not needed by the current
//          ; character set, DQUOTE, ";", ":", "\", ","
optional<string> read_tsafe_char(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (auto c = read_wsp(is)) {
                ptran.commit();
                return c;
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
                return string() + (char)i;
        }

        // NON-US-ASCII
        //std::cerr << "todo: non-us-ascii" << std::endl;
        return nullopt;
}


//       text       = *(TSAFE-CHAR / ":" / DQUOTE / ESCAPED-CHAR)
//          ; Folded according to description above
//
optional<string> read_text_char(istream &is) {
        CALLSTACK;
        if (auto val = read_tsafe_char(is)) return val;
        if (auto val = read_token(is, ":")) return val;
        if (auto val = read_dquote(is)) return val;
        if (auto val = read_escaped_char(is)) return val;
        return nullopt;
}
string expect_text(istream &is) {
        CALLSTACK;
        string ret;
        while(auto c = read_text_char(is)) {
                ret += *c;
        }
        return ret;
}

// DQUOTE: ASCII 22
optional<string> read_dquote(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != 22)
                return nullopt;
        ptran.commit();
        return string() + (char)22;
}

optional<string> read_text(istream &is) {
        CALLSTACK;
        try {
                return expect_text(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

bool read_binary(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       float      = (["+"] / "-") 1*DIGIT ["." 1*DIGIT]
bool read_float(istream &is) {
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
bool read_integer(istream &is) {
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
bool read_actionvalue(istream &is) {
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
bool read_actionparam(istream &is) {
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
bool read_action(istream &is) {
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
bool read_trigabs(istream &is) {
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
bool read_trigrel_single(istream &is) {
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
bool read_trigrel(istream &is) {
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
bool read_trigger(istream &is) {
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
bool read_repparam(istream &is) {
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
bool read_repeat(istream &is) {
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
bool read_audioprop_single(istream &is) {
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
bool read_dispprop_single(istream &is) {
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
bool read_emailprop_single(istream &is) {
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
bool read_alarmc_prop(istream &is) {
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
optional<Alarm> read_alarmc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VALARM") &&
                read_alarmc_prop(is) &&
                read_key_value(is, "END", "VALARM");
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       date-fullyear      = 4DIGIT
bool read_date_fullyear(istream &is) {
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
bool read_date_month(istream &is) {
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
bool read_date_mday(istream &is) {
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
bool read_date_value(istream &is) {
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
bool read_date(istream &is) {
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
bool read_time_hour(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_digit(is) && read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       time-minute  = 2DIGIT        ;00-59
bool read_time_minute(istream &is) {
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
bool read_time_second(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_digit(is) && read_digit(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       time-utc     = "Z"
bool read_time_utc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = read_token(is, "Z");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       time         = time-hour time-minute time-second [time-utc]
bool read_time(istream &is) {
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
bool read_date_time(istream &is) {
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
bool read_dur_week(istream &is) {
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
bool read_dur_second(istream &is) {
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
bool read_dur_minute(istream &is) {
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
bool read_dur_hour(istream &is) {
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
bool read_dur_day(istream &is) {
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
bool read_dur_time(istream &is) {
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
bool read_dur_date(istream &is) {
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
bool read_dur_value(istream &is) {
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
bool read_period_start(istream &is) {
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
bool read_period_explicit(istream &is) {
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
bool read_period(istream &is) {
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
optional<OtherParam> read_other_param(istream &is) {
        CALLSTACK;
        if (auto v = read_iana_param(is)) return v;
        if (auto v = read_x_param(is)) return v;
        return nullopt;
}

// stmparam   = *(";" other-param)
bool read_stmparam(istream &is) {
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
optional<DtStamp> read_dtstamp(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTSTAMP") &&
                read_stmparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}
//       uidparam   = *(";" other-param)
bool read_uidparam(istream &is) {
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
optional<Uid> read_uid(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "UID") &&
                read_uidparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}
//       dtstval    = date-time / date
bool read_dtstval(istream &is) {
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
bool read_dtstparam_single(istream &is) {
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
bool read_dtstparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_dtstparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       dtstart    = "DTSTART" dtstparam ":" dtstval CRLF
//
optional<DtStart> read_dtstart(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTSTART") &&
                read_dtstparam(is) &&
                read_token(is, ":") &&
                read_dtstval(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       classvalue = "PUBLIC" / "PRIVATE" / "CONFIDENTIAL" / iana-token
//                  / x-name
//       ;Default is PUBLIC
bool read_classvalue(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       classparam = *(";" other-param)
bool read_classparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       class      = "CLASS" classparam ":" classvalue CRLF
optional<Class> read_class(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "CLASS") &&
                read_classparam(is) &&
                read_token(is, ":") &&
                read_classvalue(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       creaparam  = *(";" other-param)
bool read_creaparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       created    = "CREATED" creaparam ":" date-time CRLF
optional<Created> read_created(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "CREATED") &&
                read_creaparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_descparam(istream &is) {
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
optional<Description> read_description(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DESCRIPTION") &&
                read_descparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       geovalue   = float ";" float
//       ;Latitude and Longitude components
bool read_geovalue(istream &is) {
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
bool read_geoparam(istream &is) {
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
optional<Geo> read_geo(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "GEO") &&
                read_geoparam(is) &&
                read_token(is, ":") &&
                read_geovalue(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       lstparam   = *(";" other-param)
bool read_lstparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       last-mod   = "LAST-MODIFIED" lstparam ":" date-time CRLF
optional<LastMod> read_last_mod(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LAST-MODIFIED") &&
                read_lstparam(is) &&
                read_token(is, ":") &&
                read_date_time(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_locparam_single(istream &is) {
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
bool read_locparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_locparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       location   = "LOCATION"  locparam ":" text CRLF
optional<Location> read_location(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "LOCATION") &&
                read_locparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_orgparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}
optional<string> read_cal_address(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}
//       organizer  = "ORGANIZER" orgparam ":" cal-address CRLF
optional<Organizer> read_organizer(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "ORGANIZER") &&
                read_orgparam(is) &&
                read_token(is, ":") &&
                read_cal_address(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       priovalue   = integer       ;Must be in the range [0..9]
//          ; All other values are reserved for future use.
bool read_priovalue(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       prioparam  = *(";" other-param)
bool read_prioparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       priority   = "PRIORITY" prioparam ":" priovalue CRLF
//       ;Default is zero (i.e., undefined).
optional<Priority> read_priority(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "PRIORITY") &&
                read_prioparam(is) &&
                read_token(is, ":") &&
                read_priovalue(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       seqparam   = *(";" other-param)
bool read_seqparam(istream &is) {
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
optional<Seq> read_seq(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "SEQUENCE") &&
                read_seqparam(is) &&
                read_token(is, ":") &&
                read_integer(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       statvalue-jour  = "DRAFT"        ;Indicates journal is draft.
//                       / "FINAL"        ;Indicates journal is final.
//                       / "CANCELLED"    ;Indicates journal is removed.
//      ;Status values for "VJOURNAL".
bool read_statvalue_jour(istream &is) {
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
bool read_statvalue_todo(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}


//       statvalue       = (statvalue-event
//                       /  statvalue-todo
//                       /  statvalue-jour)
bool read_statvalue(istream &is) {
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
bool read_statvalue_event(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       statparam       = *(";" other-param)
bool read_statparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       status          = "STATUS" statparam ":" statvalue CRLF
optional<Status> read_status(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "STATUS") &&
                read_statparam(is) &&
                read_token(is, ":") &&
                read_statvalue(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_summparam_single(istream &is) {
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
bool read_summparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_summparam_single(is)) {
        }
        ptran.commit();
        return true;
}

//       summary    = "SUMMARY" summparam ":" text CRLF
optional<Summary> read_summary(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "SUMMARY") &&
                read_summparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       transparam = *(";" other-param)
bool read_transparam(istream &is) {
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
bool read_transvalue(istream &is) {
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
optional<Transp> read_transp(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "TRANSP") &&
                read_transparam(is) &&
                read_token(is, ":") &&
                read_transvalue(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//      uri = <As defined in Section 3 of [RFC3986]>
bool read_uri(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = rfc3986::read_URI(is);
        if (!success)
                return false;
        ptran.commit();
        return true;
}
//       urlparam   = *(";" other-param)
bool read_urlparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       url        = "URL" urlparam ":" uri CRLF
optional<Url> read_url(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "URL") &&
                read_urlparam(is) &&
                read_token(is, ":") &&
                read_uri(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       ridval     = date-time / date
//       ;Value MUST match value type
bool read_ridval(istream &is) {
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
bool read_ridparam_single(istream &is) {
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
bool read_ridparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_ridparam_single(is)) {
        }
        ptran.commit();
        return true;
}

//       setposday   = yeardaynum
bool read_setposday(istream &is) {
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
bool read_bysplist(istream &is) {
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
bool read_monthnum(istream &is) {
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
bool read_bymolist(istream &is) {
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
bool read_weeknum(istream &is) {
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
bool read_bywknolist(istream &is) {
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
bool read_ordyrday(istream &is) {
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
bool read_yeardaynum(istream &is) {
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
bool read_byyrdaylist(istream &is) {
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
bool read_ordmoday(istream &is) {
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
bool read_monthdaynum(istream &is) {
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
bool read_bymodaylist(istream &is) {
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
bool read_weekday(istream &is) {
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
bool read_ordwk(istream &is) {
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
bool readminus(istream &is) {
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
bool read_plus(istream &is) {
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
bool read_weekdaynum(istream &is) {
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
bool read_bywdaylist(istream &is) {
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
bool read_hour(istream &is) {
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
bool read_byhrlist(istream &is) {
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
bool read_minutes(istream &is) {
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
bool read_byminlist(istream &is) {
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
bool read_seconds(istream &is) {
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
bool read_byseclist(istream &is) {
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
bool read_enddate(istream &is) {
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
bool read_freq(istream &is) {
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
bool read_recur_rule_part(istream &is) {
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
bool read_recur(istream &is) {
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
optional<RecurId> read_recurid(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RECURRENCE-ID") &&
                read_ridparam(is) &&
                read_token(is, ":") &&
                read_ridval(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       rrulparam  = *(";" other-param)
bool read_rrulparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       rrule      = "RRULE" rrulparam ":" recur CRLF
optional<RRule> read_rrule(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RRULE") &&
                read_rrulparam(is) &&
                read_token(is, ":") &&
                read_recur(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       dtendval   = date-time / date
//       ;Value MUST match value type
bool read_dtendval(istream &is) {
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
bool read_dtendparam(istream &is) {
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
optional<DtEnd> read_dtend(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DTEND") &&
                read_dtendparam(is) &&
                read_token(is, ":") &&
                read_dtendval(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       durparam   = *(";" other-param)
bool read_durparam(istream &is) {
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
optional<Duration> read_duration(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "DURATION") &&
                read_durparam(is) &&
                read_token(is, ":") &&
                read_dur_value(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_attachparam(istream &is) {
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
optional<Attach> read_attach(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto match_head =
                read_token(is, "ATTACH") && read_attachparam(is);
        if (!match_head)
                return nullopt;

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
                        NOT_IMPLEMENTED;
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
                        NOT_IMPLEMENTED;
                }
        }

        return nullopt;
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
bool read_attparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       attendee   = "ATTENDEE" attparam ":" cal-address CRLF
optional<Attendee> read_attendee(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "ATTENDEE") &&
                read_attparam(is) &&
                read_token(is, ":") &&
                read_cal_address(is) &&
                read_newline(is);
        if (!match)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_catparam(istream &is) {
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
optional<Categories> read_categories (istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "CATEGORIES") &&
                read_catparam(is) &&
                read_token(is, ":") &&
                read_text(is);
        if (!match)
                return nullopt;
        while (read_token(is, ",")) {
                if (!read_text(is))
                        return nullopt;
        }
        if (!read_newline(is))
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_commparam(istream &is) {
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
optional<Comment> read_comment(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "COMMENT") &&
                read_commparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!match)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
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
bool read_contparam(istream &is) {
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
optional<Contact> read_contact(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "CONTACT") &&
                read_contparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!match)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       exdtval    = date-time / date
//       ;Value MUST match value type
bool read_exdtval(istream &is) {
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
bool read_exdtparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       exdate     = "EXDATE" exdtparam ":" exdtval *("," exdtval) CRLF
optional<ExDate> read_exdate(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                read_token(is, "EXDATE") &&
                read_exdtparam(is) &&
                read_token(is, ":") &&
                read_exdtval(is);
        if (!match)
                return nullopt;
        while (read_token(is, ",")) {
                if (!read_exdtval(is))
                        return nullopt;
        }
        if (!read_newline(is))
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       extdata    = text
//       ;Textual exception data.  For example, the offending property
//       ;name and value or complete property line.
bool read_extdata(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_text(is))
                return false;
        ptran.commit();
        return true;
}

//       statdesc   = text
//       ;Textual status description
bool read_statdesc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_text(is))
                return false;
        ptran.commit();
        return true;
}

//       statcode   = 1*DIGIT 1*2("." 1*DIGIT)
//       ;Hierarchical, numeric return status code
bool read_statcode(istream &is) {
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
bool read_rstatparam(istream &is) {
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
optional<RStatus> read_rstatus(istream &is) {
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
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       reltypeparam       = "RELTYPE" "="
//                           ("PARENT"    ; Parent relationship - Default
//                          / "CHILD"     ; Child relationship
//                          / "SIBLING"   ; Sibling relationship
//                          / iana-token  ; Some other IANA-registered
//                                        ; iCalendar relationship type
//                          / x-name)     ; A non-standard, experimental
//                                        ; relationship type
optional<RelTypeParam> read_reltypeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        RelTypeParam ret;

        if (!read_token(is, "RELTYPE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "PARENT")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "CHILD")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "SIBLING")) {
                ret.value = *v;
        } else if (auto v = read_iana_token(is)) {
                ret.value = *v;
        } else if (auto v = read_x_name(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
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
bool read_relparam(istream &is) {
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
optional<Related> read_related(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RELATED-TO") &&
                read_relparam(is) &&
                read_token(is, ":") &&
                read_text(is) &&
                read_newline(is);
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       resrcparam = *(
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
bool read_resrcparam(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       resources  = "RESOURCES" resrcparam ":" text *("," text) CRLF
optional<Resources> read_resources(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RESOURCES") &&
                read_resrcparam(is) &&
                read_token(is, ":") &&
                read_text(is);
        if (!success)
                return nullopt;
        while (read_token(is, ",")) {
                if (!read_text(is))
                        return nullopt;
        }
        if (!read_newline(is))
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       rdtval     = date-time / date / period
//       ;Value MUST match value type
bool read_rdtval(istream &is) {
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
bool read_rdtparam_single(istream &is) {
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
bool read_rdtparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        while (read_rdtparam_single(is)) {
        }
        ptran.commit();
        return true;
}
//       rdate      = "RDATE" rdtparam ":" rdtval *("," rdtval) CRLF
optional<RDate> read_rdate(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_token(is, "RDATE") &&
                read_rdtparam(is) &&
                read_token(is, ":") &&
                read_rdtval(is);
        if (!success)
                return nullopt;
        while (read_token(is, ",")) {
                if (!read_rdtval(is))
                        return nullopt;
        }
        if (!read_newline(is))
                return nullopt;
        ptran.commit();
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
optional<EventProp> read_eventprop_single(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        EventProp ret;
        if (auto v = read_dtstamp(is)) ret = *v;
        else if (auto v = read_uid(is)) ret = *v;

        else if (auto v = read_dtstart(is)) ret = *v;

        else if (auto v = read_class(is)) ret = *v;
        else if (auto v = read_created(is)) ret = *v;
        else if (auto v = read_description(is)) ret = *v;
        else if (auto v = read_geo(is)) ret = *v;
        else if (auto v = read_last_mod(is)) ret = *v;
        else if (auto v = read_location(is)) ret = *v;
        else if (auto v = read_organizer(is)) ret = *v;
        else if (auto v = read_priority(is)) ret = *v;
        else if (auto v = read_seq(is)) ret = *v;
        else if (auto v = read_status(is)) ret = *v;
        else if (auto v = read_summary(is)) ret = *v;
        else if (auto v = read_transp(is)) ret = *v;
        else if (auto v = read_url(is)) ret = *v;
        else if (auto v = read_recurid(is)) ret = *v;

        else if (auto v = read_rrule(is)) ret = *v;

        else if (auto v = read_dtend(is)) ret = *v;
        else if (auto v = read_duration(is)) ret = *v;

        else if (auto v = read_attach(is)) ret = *v;
        else if (auto v = read_attendee(is)) ret = *v;
        else if (auto v = read_categories (is)) ret = *v;
        else if (auto v = read_comment(is)) ret = *v;
        else if (auto v = read_contact(is)) ret = *v;
        else if (auto v = read_exdate(is)) ret = *v;
        else if (auto v = read_rstatus(is)) ret = *v;
        else if (auto v = read_related(is)) ret = *v;
        else if (auto v = read_resources(is)) ret = *v;
        else if (auto v = read_rdate(is)) ret = *v;
        else if (auto v = read_x_prop(is)) ret = *v;
        else if (auto v = read_iana_prop(is)) ret = *v;

        else return nullopt;

        ptran.commit();
        return ret;
}
optional<vector<EventProp>> read_eventprop(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<EventProp> ret;
        while (auto v = read_eventprop_single(is)) {
                ret.push_back(*v);
        }
        ptran.commit();
        return ret;
}

//       eventc     = "BEGIN" ":" "VEVENT" CRLF
//                    eventprop *alarmc
//                    "END" ":" "VEVENT" CRLF
optional<EventComp> read_eventc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        EventComp ret;
        if (read_key_value(is, "BEGIN", "VEVENT"))
                return nullopt;
        if (auto v = read_eventprop(is)) {
                ret.properties = *v;
        } else {
                return nullopt;
        }

        while(auto v = read_alarmc(is)) {
                ret.alarms.push_back(*v);
        }

        if (!read_key_value(is, "END", "VEVENT"))
                return nullopt;

        ptran.commit();
        return ret;
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
bool read_todoprop(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       todoc      = "BEGIN" ":" "VTODO" CRLF
//                    todoprop *alarmc
//                    "END" ":" "VTODO" CRLF
optional<TodoComp> read_todoc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_key_value(is, "BEGIN", "VTODO") &&
                read_todoprop(is);
        if (!success_pro)
                return nullopt;

        while(read_alarmc(is)) {
        }

        const auto success_epi =
                read_key_value(is, "END", "VTODO") ;
        if (!success_epi)
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
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
bool read_jourprop(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       journalc   = "BEGIN" ":" "VJOURNAL" CRLF
//                    jourprop
//                    "END" ":" "VJOURNAL" CRLF
optional<JournalComp> read_journalc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VJOURNAL") &&
                read_jourprop(is) &&
                read_key_value(is, "END", "VJOURNAL") ;
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
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
bool read_fbprop(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       freebusyc  = "BEGIN" ":" "VFREEBUSY" CRLF
//                    fbprop
//                    "END" ":" "VFREEBUSY" CRLF
optional<FreeBusyComp> read_freebusyc(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                read_key_value(is, "BEGIN", "VFREEBUSY") &&
                read_fbprop(is) &&
                read_key_value(is, "END", "VFREEBUSY") ;
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

bool read_tzid(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

bool read_tzurl(istream &is) {
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
bool read_tzprop(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       daylightc  = "BEGIN" ":" "DAYLIGHT" CRLF
//                    tzprop
//                    "END" ":" "DAYLIGHT" CRLF
bool read_daylightc(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       standardc  = "BEGIN" ":" "STANDARD" CRLF
//                    tzprop
//                    "END" ":" "STANDARD" CRLF
bool read_standardc(istream &is) {
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
optional<TimezoneComp> read_timezonec(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_key_value(is, "BEGIN", "VTIMEZONE") &&
                read_todoprop(is);
        if (!success_pro)
                return nullopt;

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
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       iana-comp  = "BEGIN" ":" iana-token CRLF
//                    1*contentline
//                    "END" ":" iana-token CRLF
optional<IanaComp> read_iana_comp(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_token(is, "BEGIN") &&
                read_token(is, ":") &&
                read_iana_token(is) &&
                read_newline(is);
        if (!success_pro)
                return nullopt;

        if (!read_contentline(is))
                return nullopt;
        while(read_contentline(is)) {
        }

        const auto success_epi =
                read_token(is, "END") &&
                read_token(is, ":") &&
                read_iana_token(is) &&
                read_newline(is);
        if (!success_epi)
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       x-comp     = "BEGIN" ":" x-name CRLF
//                    1*contentline
//                    "END" ":" x-name CRLF
optional<XComp> read_x_comp(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                read_token(is, "BEGIN") &&
                read_token(is, ":") &&
                read_x_name(is) &&
                read_newline(is);
        if (!success_pro)
                return nullopt;

        if (!read_contentline(is))
                return nullopt;
        while(read_contentline(is)) {
        }

        const auto success_epi =
                read_token(is, "END") &&
                read_token(is, ":") &&
                read_x_name(is) &&
                read_newline(is);
        if (!success_epi)
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       component  = 1*(eventc / todoc / journalc / freebusyc /
//                    timezonec / iana-comp / x-comp)
//
optional<Component> read_component_single(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        Component ret;
        if (auto v = read_eventc(is)) ret = *v;
        else if (auto v = read_todoc(is)) ret = *v;
        else if (auto v = read_journalc(is)) ret = *v;
        else if (auto v = read_freebusyc(is)) ret = *v;
        else if (auto v = read_timezonec(is)) ret = *v;
        else if (auto v = read_iana_comp(is)) ret = *v;
        else if (auto v = read_x_comp(is)) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}
Component expect_component_single(istream &is) {
        CALLSTACK;
        if (auto v = read_component_single(is))
                return *v;
        throw syntax_error(is.tellg());
}
vector<Component> expect_component(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<Component> ret;
        ret.push_back(expect_component_single(is));
        while(auto v = read_component_single(is)) {
                ret.push_back(*v);
        }
        ptran.commit();
        return ret;
}

// altrepparam = "ALTREP" "=" DQUOTE uri DQUOTE
optional<AltRepParam> read_altrepparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_token(is, "ALTREP")) return nullopt;
        if (!read_token(is, "=")) return nullopt;
        const auto v = read_quoted_string(is);
        if (!v) return nullopt;
        ptran.commit();
        return {{*v}};
}

// cnparam    = "CN" "=" param-value
optional<CnParam> read_cnparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_token(is, "CN")) return nullopt;
        if (!read_token(is, "=")) return nullopt;
        const auto v = read_param_value(is);
        if (!v) return nullopt;
        ptran.commit();
        return {{*v}};
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
optional<CuTypeParam> read_cutypeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!read_token(is, "CUTYPE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        string val;
        if (auto v = read_token(is, "INDIVIDUAL")) val = *v;
        else if (read_token(is, "GROUP")) val = *v;
        else if (read_token(is, "RESOURCE")) val = *v;
        else if (read_token(is, "ROOM")) val = *v;
        else if (read_token(is, "UNKNOWN")) val = *v;
        else if (read_x_name(is)) val = *v;
        else if (read_iana_token(is)) val = *v;
        else return nullopt;

        ptran.commit();
        return {{val}};
}

//      delfromparam       = "DELEGATED-FROM" "=" DQUOTE cal-address DQUOTE
//                           *("," DQUOTE cal-address DQUOTE)
optional<DelFromParam> read_delfromparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        DelFromParam ret;

        if (!read_token(is, "DELEGATED-FROM")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_quoted_string(is)) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (read_token(is, ",")) {
                if (auto v = read_quoted_string(is)) {
                        ret.values.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      deltoparam = "DELEGATED-TO" "=" DQUOTE cal-address DQUOTE
//                    *("," DQUOTE cal-address DQUOTE)
optional<DelToParam> read_deltoparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        DelToParam ret;

        if (!read_token(is, "DELEGATED-TO")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_quoted_string(is)) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (read_token(is, ",")) {
                if (auto v = read_quoted_string(is)) {
                        ret.values.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      dirparam   = "DIR" "=" DQUOTE uri DQUOTE
optional<DirParam> read_dirparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        DirParam ret;

        if (!read_token(is, "DIR")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_quoted_string(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

// encodingparam =
//          "ENCODING" "="
//          ( "8BIT"   ; "8bit" text encoding is defined in [RFC2045]
//          / "BASE64" ; "BASE64" binary encoding format is defined in [RFC4648]
//          )
optional<EncodingParam> read_encodingparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        EncodingParam ret;
        if (!read_token(is, "ENCODING")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "8BIT")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "BASE64")) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       fmttypeparam = "FMTTYPE" "=" type-name "/" subtype-name
//                      ; Where "type-name" and "subtype-name" are
//                      ; defined in Section 4.2 of [RFC4288].
optional<FmtTypeParam> read_fmttypeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        FmtTypeParam ret;

        if (!read_token(is, "FMTTYPE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_type_name(is)) {
                ret.value = *v;
        } else if (auto v = read_subtype_name(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
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
optional<FbTypeParam> read_fbtypeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        FbTypeParam ret;
        if (!read_token(is, "FBTYPE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "FREE")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "BUSY")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "BUSY-UNAVAILABLE")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "BUSY-TENTATIVE")) {
                ret.value = *v;
        } else if (auto v = read_x_name(is)) {
                ret.value = *v;
        } else if (auto v = read_iana_token(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       languageparam = "LANGUAGE" "=" language
//
//       language = Language-Tag
//                  ; As defined in [RFC5646].
optional<LanguageParam> read_languageparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        LanguageParam ret;
        if (!read_token(is, "LANGUAGE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_language_tag(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       memberparam        = "MEMBER" "=" DQUOTE cal-address DQUOTE
//                            *("," DQUOTE cal-address DQUOTE)
optional<MemberParam> read_memberparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        MemberParam ret;

        if (!read_token(is, "MEMBER")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_cal_address(is)) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (read_token(is, ",")) {
                if (auto v = read_cal_address(is)) {
                        ret.values.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}


//       partstat-jour    = ("NEEDS-ACTION"    ; Journal needs action
//                        / "ACCEPTED"         ; Journal accepted
//                        / "DECLINED"         ; Journal declined
//                        / x-name             ; Experimental status
//                        / iana-token)        ; Other IANA-registered
//                                             ; status
//       ; These are the participation statuses for a "VJOURNAL".
//       ; Default is NEEDS-ACTION.
optional<PartStatJour> read_partstat_jour(istream &is) {
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
optional<PartStatTodo> read_partstat_todo(istream &is) {
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
optional<PartStatEvent> read_partstat_event(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       partstatparam    = "PARTSTAT" "="
//                         (partstat-event
//                        / partstat-todo
//                        / partstat-jour)
optional<PartStatParam> read_partstatparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        PartStatParam ret;

        if (!read_token(is, "PARTSTAT")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_partstat_event(is)) {
                ret = *v;
        } else if (auto v = read_partstat_todo(is)) {
                ret = *v;
        } else if (auto v = read_partstat_jour(is)) {
                ret = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       rangeparam = "RANGE" "=" "THISANDFUTURE"
//       ; To specify the instance specified by the recurrence identifier
//       ; and all subsequent recurrence instances.
optional<RangeParam> read_rangeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        RangeParam ret;

        if (!read_token(is, "RANGE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "THISANDFUTURE")) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       trigrelparam       = "RELATED" "="
//                          ( "START"       ; Trigger off of start
//                          / "END")        ; Trigger off of end
optional<TrigRelParam> read_trigrelparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        TrigRelParam ret;

        if (!read_token(is, "RELATED")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "START")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "END")) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
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
optional<RoleParam> read_roleparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        RoleParam ret;

        if (!read_token(is, "ROLE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "CHAIR")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "REQ-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "OPT-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "NON-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = read_x_name(is)) {
                ret.value = *v;
        } else if (auto v = read_iana_token(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       rsvpparam = "RSVP" "=" ("TRUE" / "FALSE")
//       ; Default is FALSE
optional<RsvpParam> read_rsvpparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        RsvpParam ret;

        if (!read_token(is, "RSVP")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_token(is, "TRUE")) {
                ret.value = *v;
        } else if (auto v = read_token(is, "FALSE")) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       sentbyparam        = "SENT-BY" "=" DQUOTE cal-address DQUOTE
optional<SentByParam> read_sentbyparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        SentByParam ret;

        if (!read_token(is, "SENT-BY")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_quoted_string(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       tzidprefix = "/"
optional<string> read_tzidprefix(istream &is) {
        CALLSTACK;
        return read_token(is, "/");
}

//       tzidparam  = "TZID" "=" [tzidprefix] paramtext
optional<TzIdParam> read_tzidparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        TzIdParam ret;

        if (!read_token(is, "TZID")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_tzidprefix(is)) {
                ret.value += *v;
        }

        if (auto v = read_paramtext(is)) {
                ret.value += *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
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
optional<ValueType> read_valuetype(istream &is) {
        CALLSTACK;
        if (auto v = read_token(is, "BINARY")) return {{*v}};
        if (auto v = read_token(is, "BOOLEAN")) return {{*v}};
        if (auto v = read_token(is, "CAL-ADDRESS")) return {{*v}};
        if (auto v = read_token(is, "DATE")) return {{*v}};
        if (auto v = read_token(is, "DATE-TIME")) return {{*v}};
        if (auto v = read_token(is, "DURATION")) return {{*v}};
        if (auto v = read_token(is, "FLOAT")) return {{*v}};
        if (auto v = read_token(is, "INTEGER")) return {{*v}};
        if (auto v = read_token(is, "PERIOD")) return {{*v}};
        if (auto v = read_token(is, "RECUR")) return {{*v}};
        if (auto v = read_token(is, "TEXT")) return {{*v}};
        if (auto v = read_token(is, "TIME")) return {{*v}};
        if (auto v = read_token(is, "URI")) return {{*v}};
        if (auto v = read_token(is, "UTC-OFFSET")) return {{*v}};
        if (auto v = read_x_name(is)) return {{*v}};
        if (auto v = read_iana_token(is)) return {{*v}};
        return nullopt;
}

//       valuetypeparam = "VALUE" "=" valuetype
optional<ValueTypeParam> read_valuetypeparam(istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        ValueTypeParam ret;

        if (!read_token(is, "VALUE")) return nullopt;
        if (!read_token(is, "=")) return nullopt;

        if (auto v = read_valuetype(is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
optional<IanaParam> read_iana_param(istream &is) {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
optional<XParam> read_x_param(istream &is) {
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
ICalParameter expect_icalparameter(istream &is) {
        CALLSTACK;
        if (auto v = read_icalparameter(is))
                return *v;
        throw syntax_error(is.tellg());
}
optional<ICalParameter> read_icalparameter(istream &is) {
        CALLSTACK;
        if (auto v = read_altrepparam(is)) return v;
        if (auto v = read_cnparam(is)) return v;
        if (auto v = read_cutypeparam(is)) return v;
        if (auto v = read_delfromparam(is)) return v;
        if (auto v = read_deltoparam(is)) return v;
        if (auto v = read_dirparam(is)) return v;
        if (auto v = read_encodingparam(is)) return v;
        if (auto v = read_fmttypeparam(is)) return v;
        if (auto v = read_fbtypeparam(is)) return v;
        if (auto v = read_languageparam(is)) return v;
        if (auto v = read_memberparam(is)) return v;
        if (auto v = read_partstatparam(is)) return v;
        if (auto v = read_rangeparam(is)) return v;
        if (auto v = read_trigrelparam(is)) return v;
        if (auto v = read_reltypeparam(is)) return v;
        if (auto v = read_roleparam(is)) return v;
        if (auto v = read_rsvpparam(is)) return v;
        if (auto v = read_sentbyparam(is)) return v;
        if (auto v = read_tzidparam(is)) return v;
        if (auto v = read_valuetypeparam(is)) return v;
        if (auto v = read_other_param(is)) return v;
        return nullopt;
}
