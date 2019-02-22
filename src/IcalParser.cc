#include <iostream>
#include "rfc3986.hh"
#include "rfc4288.hh"
#include "rfc5234.hh"
#include "rfc5646.hh"
#include "IcalParser.hh"
#include "parser_helpers.hh"
#include "parser_exceptions.hh"

string IcalParser::expect_token(string const &tok) {
        CALLSTACK;
        save_input_pos ptran(is);
        for (auto &c : tok) {
                const auto i = is.get();
                if(i == EOF) {
                        throw unexpected_token(is.tellg(), tok);
                }
                if(i != c) {
                        throw syntax_error(is.tellg(), tok);
                }
        }
        ptran.commit();
        return tok;
}

optional<string> IcalParser::token(string const &tok) {
        CALLSTACK;
        try {
                return expect_token(tok);
        } catch(syntax_error &) {
                return nullopt;
        }
}

optional<string> IcalParser::eof() {
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != EOF)
                return nullopt;
        ptran.commit();
        return string() + (char)c;
}

string IcalParser::expect_newline() {
        CALLSTACK;
        // Even though RFC 5545 says just "CRLF", we also handle "CR" and "LF".

        std::istream::sentry se(is, true);
        std::streambuf* sb = is.rdbuf();

        switch (sb->sgetc()) {
        case '\n':
                sb->sbumpc();
                return "\n";
        case '\r':
                sb->sbumpc();
                if (sb->sgetc() == '\n') {
                        sb->sbumpc();
                        return "\r\n";
                }
                return "\r";
        case std::streambuf::traits_type::eof():
                is.setstate(std::ios::eofbit);
                return "";
        };
        throw unexpected_token(is.tellg());
}

optional<string> IcalParser::newline() {
        CALLSTACK;
        try {
                return expect_newline();
        } catch (syntax_error &) {
                return nullopt;
        }
}

optional<string> IcalParser::hex() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        switch(i) {
        default:
        case EOF:
                return nullopt;
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
        return string() + (char)i;
}

string IcalParser::expect_alpha() {
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
        return string() + (char)i;
}

optional<string> IcalParser::alpha() {
        CALLSTACK;
        try {
                return expect_alpha();
        } catch (syntax_error &) {
                return nullopt;
        }
}


string IcalParser::expect_digit() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        if (i<'0' || i>'9')
                throw syntax_error(is.tellg(), "expected digit");
        ptran.commit();
        return string() + (char)i;
}

string IcalParser::expect_digit(int min, int max) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        if (i<'0'+min || i>'0'+max)
                throw syntax_error(is.tellg(),
                                   "expected digit in range [" +
                                   std::to_string(min) + ".." +
                                   std::to_string(max) + "]");
        ptran.commit();
        return string() + (char)i;
}

optional<string> IcalParser::digit() {
        CALLSTACK;
        try {
                return expect_digit();
        } catch (syntax_error &) {
                return nullopt;
        }
}

optional<string> IcalParser::digit(int min, int max) {
        CALLSTACK;
        try {
                return expect_digit(min, max);
        } catch (syntax_error &) {
                return nullopt;
        }
}

optional<string> IcalParser::digits(int at_least, int at_most) {
        CALLSTACK;
        save_input_pos ptran(is);
        int c = 0;
        string ret;
        optional<string> tmp;
        while ((at_most<0 || c<at_most) && (tmp=digit())) {
                ++c;
                ret += *tmp;
        }

        if (at_least >= 0 && c<at_least)
                return nullopt;
        if (at_most >= 0 && c>at_most)
                return nullopt;

        ptran.commit();
        return ret;
}

optional<string> IcalParser::digits(int num) {
        CALLSTACK;
        return digits(num, num);
}

string IcalParser::expect_alnum() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (auto v = alpha()) {
                ptran.commit();
                return *v;
        } else if (auto v = digit()) {
                ptran.commit();
                return *v;
        }
        throw syntax_error(is.tellg(), "expected alpha or digit");
}

optional<string> IcalParser::alnum() {
        CALLSTACK;
        try {
                return expect_alnum();
        } catch (syntax_error &) {
                return nullopt;
        }
}


tuple<string, string> IcalParser::expect_key_value(
        string const &k,
        string const &v
) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token(k) &&
                token(":") &&
                token(v) &&
                newline();
        if (!success) {
                throw key_value_pair_expected(is.tellg(), k, v);
        }
        ptran.commit();
        return {k, v};
}

optional<tuple<string, string>> IcalParser::key_value(
        string const &k,
        string const &v
) {
        CALLSTACK;
        try {
                return expect_key_value(k, v);
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
ContentLine IcalParser::expect_contentline() {
        CALLSTACK;
        save_input_pos ptran(is);
        ContentLine ret;
        ret.name = expect_name();
        while (token(";")) {
                ret.params.push_back(expect_param());
        }
        expect_token(":");
        ret.value = expect_value();
        expect_newline();
        ptran.commit();
        return ret;
}
optional<ContentLine> IcalParser::contentline() {
        CALLSTACK;
        try {
                return expect_contentline();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//     name          = iana-token / x-name
string IcalParser::expect_name() {
        CALLSTACK;
        if (auto v = iana_token())
                return *v;
        if (auto v = x_name())
                return *v;
        throw syntax_error(is.tellg(), "expected iana-token or x-name");
}

//     iana-token    = 1*(ALPHA / DIGIT / "-")
//     ; iCalendar identifier registered with IANA
optional<string> IcalParser::iana_token_char() {
        CALLSTACK;
        if (auto v = alnum()) return *v;
        if (auto v = token("-")) return *v;
        return nullopt;
}
optional<string> IcalParser::iana_token() {
        CALLSTACK;
        string ret;
        if (auto c = iana_token_char()) {
                ret += *c;
        } else {
                return nullopt;
        }
        while (auto c = iana_token_char())
                ret += *c;
        // TODO: IANA iCalendar identifiers
        return ret;
}
string IcalParser::expect_iana_token() {
        CALLSTACK;
        if (auto v = iana_token())
                return *v;
        throw unexpected_token(is.tellg());
}

//     vendorid      = 3*(ALPHA / DIGIT)
//     ; Vendor identification
string IcalParser::expect_vendorid() {
        CALLSTACK;
        save_input_pos ptran(is);

        string ret;
        ret += expect_alnum();
        ret += expect_alnum();
        ret += expect_alnum();

        while (auto v = alnum()) {
                ret += *v;
        }

        ptran.commit();
        return ret;
}
optional<string> IcalParser::vendorid() {
        CALLSTACK;
        try {
                return expect_vendorid();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//     SAFE-CHAR     = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-7E / NON-US-ASCII
//     ; Any character except CONTROL, DQUOTE, ";", ":", ","
optional<string> IcalParser::safe_char() {
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
optional<string> IcalParser::value_char() {
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
optional<string> IcalParser::qsafe_char() {
        CALLSTACK;
        save_input_pos ptran(is);
        // WSP
        if (auto v = read_wsp(is)) {
                ptran.commit();
                return v;
        }
        const auto i = is.get();
        switch(i) {
                // %x21
        case '!':
                // %x23-7E
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

//     NON-US-ASCII  = UTF8-2 / UTF8-3 / UTF8-4
//     ; UTF8-2, UTF8-3, and UTF8-4 are defined in [RFC3629]
optional<string> IcalParser::non_us_ascii() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     CONTROL       = %x00-08 / %x0A-1F / %x7F
//     ; All the controls except HTAB
optional<string> IcalParser::control() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//     value         = *VALUE-CHAR
string IcalParser::expect_value() {
        CALLSTACK;
        string ret;
        while (auto c = value_char()) {
                ret += *c;
        }
        return ret;
}


//     param         = param-name "=" param-value *("," param-value)
//     ; Each property defines the specific ABNF for the parameters
//     ; allowed on the property.  Refer to specific properties for
//     ; precise parameter ABNF.
//
Param IcalParser::expect_param() {
        CALLSTACK;
        save_input_pos ptran(is);
        Param ret;
        ret.name = expect_param_name();
        expect_token("=");
        ret.values.push_back(expect_param_value());
        while (token(","))
                ret.values.push_back(expect_param_value());
        ptran.commit();
        return ret;
}

//     param-name    = iana-token / x-name
string IcalParser::expect_param_name() {
        CALLSTACK;
        if (auto v = iana_token())
                return *v;
        if (auto v = x_name())
                return *v;
        throw syntax_error(is.tellg(), "expected param-name");
}


//     param-value   = paramtext / quoted-string
optional<string> IcalParser::param_value() {
        CALLSTACK;
        // NOTE: the testing order of paramtext and quoted-string
        //       as per the RFC is errorful, as paramtext can be
        //       validly empty, meaning that when in fact we have a
        //       quoted string, it will never be matched, because paramtext
        //       will successfully return an empty string.
        //       Therefore, we switched it here.
        if (auto v = quoted_string())  {
                return *v;
        }
        if (auto v = paramtext()) {
                return *v;
        }
        return nullopt;
}
string IcalParser::expect_param_value() {
        CALLSTACK;
        if (auto v = param_value())
                return *v;
        throw syntax_error(is.tellg());
}

//     paramtext     = *SAFE-CHAR
optional<string> IcalParser::paramtext() {
        CALLSTACK;
        string ret;
        while (auto v = safe_char())
                ret += *v;
        return ret;
}

//     quoted-string = DQUOTE *QSAFE-CHAR DQUOTE
optional<string> IcalParser::quoted_string() {
        CALLSTACK;
        save_input_pos ptran(is);

        if (!dquote())
                return nullopt;

        string ret;
        while (auto v = qsafe_char()) {
                ret += *v;
        }

        if (!dquote())
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
Calendar IcalParser::expect_ical() {
        CALLSTACK;
        save_input_pos ptran(is);
        Calendar ret;
        expect_key_value("BEGIN", "VCALENDAR");
        ret = expect_icalbody();
        expect_key_value("END", "VCALENDAR");
        ptran.commit();
        // TODO: The grammar says that there can be more than 1 icalobject.
        return ret;
}

optional<Calendar> IcalParser::ical() {
        CALLSTACK;
        try {
                return expect_ical();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       icalbody   = calprops component
Calendar IcalParser::expect_icalbody() {
        CALLSTACK;
        save_input_pos ptran(is);
        Calendar ret;
        ret.properties = expect_calprops();
        ret.components = expect_component();
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
CalProps IcalParser::expect_calprops() {
        CALLSTACK;
        save_input_pos ptran(is);
        CalProps ret;
        int prodidc = 0,
            versionc = 0,
            calscalec = 0,
            methodc = 0;

        while (true) {
                if (auto val = prodid()) {
                        ret.prodId = *val;
                        ++prodidc;
                } else if (auto val = version()) {
                        // ret.version = ...
                        ++versionc;
                } else if (auto val = calscale()) {
                        ++calscalec;
                } else if (auto val = method()) {
                        ++methodc;
                } else if (auto val = x_prop()) {
                } else if (auto val = iana_prop()) {
                } else {
                        break;
                }
        }
        ptran.commit();
        return ret;
}

//       prodid     = "PRODID" pidparam ":" pidvalue CRLF
ProdId IcalParser::expect_prodid() {
        CALLSTACK;
        save_input_pos ptran(is);
        ProdId ret;
        expect_token("PRODID");
        ret.params = expect_pidparam();
        expect_token(":");
        ret.value = expect_pidvalue();
        expect_newline();
        ptran.commit();
        return ret;
}
optional<ProdId> IcalParser::prodid() {
        CALLSTACK;
        try {
                return expect_prodid();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       version    = "VERSION" verparam ":" vervalue CRLF
Version IcalParser::expect_version() {
        CALLSTACK;
        save_input_pos ptran(is);
        Version ret;
        expect_token("VERSION");
        ret.params = expect_verparam();
        expect_token(":");
        ret.value = expect_vervalue();
        expect_newline();
        ptran.commit();
        return ret;
}
optional<Version> IcalParser::version() {
        CALLSTACK;
        try {
                return expect_version();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       verparam   = *(";" other-param)
std::vector<OtherParam> IcalParser::expect_verparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        std::vector<OtherParam> ret;
        while(token(";")) {
                ret.push_back(expect_other_param());
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
string IcalParser::expect_vervalue() {
        CALLSTACK;
        return expect_text();
}

//       calscale   = "CALSCALE" calparam ":" calvalue CRLF
CalScale IcalParser::expect_calscale() {
        CALLSTACK;
        save_input_pos ptran(is);
        CalScale ret;
        expect_token("CALSCALE");
        ret.params = expect_calparam();
        expect_token(":");
        ret.value = expect_calvalue();
        expect_newline();
        ptran.commit();
        return ret;
}
optional<CalScale> IcalParser::calscale() {
        CALLSTACK;
        try {
                return expect_calscale();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       calparam   = *(";" other-param)
vector<OtherParam> IcalParser::expect_calparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<OtherParam> ret;
        while (token(";"))
                ret.push_back(expect_other_param());
        ptran.commit();
        return ret;
}

//       calvalue   = "GREGORIAN"
string IcalParser::expect_calvalue() {
        CALLSTACK;
        return expect_token("GREGORIAN");
}

//       method     = "METHOD" metparam ":" metvalue CRLF
optional<Method> IcalParser::method() {
        CALLSTACK;
        try {
                return expect_method();
        } catch (syntax_error &) {
                return nullopt;
        }
}
Method IcalParser::expect_method() {
        CALLSTACK;
        save_input_pos ptran(is);
        Method ret;
        expect_token("METHOD");
        ret.params = expect_metparam();
        expect_token(":");
        ret.value = expect_metvalue();
        expect_newline();
        ptran.commit();
        return ret;
}

//       metparam   = *(";" other-param)
std::vector<OtherParam> IcalParser::expect_metparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        std::vector<OtherParam> ret;
        while (token(";"))
                ret.push_back(expect_other_param());
        ptran.commit();
        return ret;
}

//       metvalue   = iana-token
string IcalParser::expect_metvalue() {
        CALLSTACK;
        return expect_iana_token();
}

//       x-prop = x-name *(";" icalparameter) ":" value CRLF
XProp IcalParser::expect_x_prop() {
        CALLSTACK;
        save_input_pos ptran(is);
        XProp ret;
        ret.name = expect_x_name();
        while (token(";")) {
                ret.params.push_back(expect_icalparameter());
        }
        expect_token(":");
        ret.value = expect_value();
        expect_newline();
        ptran.commit();
        return ret;
}
optional<XProp> IcalParser::x_prop() {
        CALLSTACK;
        try {
                return expect_x_prop();
        } catch (syntax_error &) {
                return nullopt;
        }
}


//     x-name        = "X-" [vendorid "-"] 1*(ALPHA / DIGIT / "-")
//     ; Reserved for experimental use.
string IcalParser::expect_x_name() {
        CALLSTACK;
        save_input_pos ptran(is);

        string ret;

        // "X-"
        if (auto val = token("X-")) {
                ret += *val;
        } else {
                throw syntax_error(is.tellg());
        }

        // [vendorid "-"]
        if (auto val = vendorid()) {
                ret += *val;
                ret += expect_token("-");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (auto v = alnum()) {
                ret += *v;
        } else if (auto v = token("-")) {
                ret += *v;
        } else {
                throw syntax_error(is.tellg());
        }
        while (true) {
                if (auto v = alnum()) {
                        ret += *v;
                } else if (auto v = token("-")) {
                        ret += *v;
                } else {
                        break;
                }
        }

        ptran.commit();
        return ret;
}
optional<string> IcalParser::x_name() {
        CALLSTACK;
        try {
                return expect_x_name();
        } catch (syntax_error &) {
                return nullopt;
        }
}

//       iana-prop = iana-token *(";" icalparameter) ":" value CRLF
void IcalParser::expect_iana_prop() {
        CALLSTACK;
        save_input_pos ptran(is);
        expect_iana_token();
        while (token(";"))
                expect_icalparameter();
        expect_token(":");
        expect_value();
        expect_newline();
        ptran.commit();
}
optional<IanaProp> IcalParser::iana_prop() {
        CALLSTACK;
        return nullopt; // TODO
        NOT_IMPLEMENTED;
        try {
                expect_iana_prop();
                //return true;
        } catch (syntax_error &) {
                //return false;
        }
}

//       pidparam   = *(";" other-param)
std::vector<OtherParam> IcalParser::expect_pidparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        std::vector<OtherParam> ret;
        while (token(";"))
                ret.push_back(expect_other_param());
        ptran.commit();
        return ret;
}

//     other-param   = (iana-param / x-param)
//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
OtherParam IcalParser::expect_other_param() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       pidvalue   = text
//       ;Any text that describes the product and version
//       ;and that is generally assured of being unique.
string IcalParser::expect_pidvalue() {
        CALLSTACK;
        return expect_text();
}


//       ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
//          ; \\ encodes \, \N or \n encodes newline
//          ; \; encodes ;, \, encodes ,
optional<string> IcalParser::escaped_char() {
        CALLSTACK;
        if (auto val = token("\\\\"))
                return val;
        if (auto val = token("\\;"))
                return val;
        if (auto val = token("\\,"))
                return val;
        if (auto val = token("\\N"))
                return val;
        if (auto val = token("\\n"))
                return val;
        return nullopt;
}

//       TSAFE-CHAR = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-5B /
//                    %x5D-7E / NON-US-ASCII
//          ; Any character except CONTROLs not needed by the current
//          ; character set, DQUOTE, ";", ":", "\", ","
optional<string> IcalParser::tsafe_char() {
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
optional<string> IcalParser::text_char() {
        CALLSTACK;
        if (auto val = tsafe_char()) return val;
        if (auto val = token(":")) return val;
        if (auto val = dquote()) return val;
        if (auto val = escaped_char()) return val;
        return nullopt;
}
string IcalParser::expect_text() {
        CALLSTACK;
        string ret;
        while(auto c = text_char()) {
                ret += *c;
        }
        return ret;
}

// DQUOTE: ASCII 22 == '"'
optional<string> IcalParser::dquote() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != '"')
                return nullopt;
        ptran.commit();
        return "\"";
}

optional<string> IcalParser::text() {
        CALLSTACK;
        try {
                return expect_text();
        } catch (syntax_error &) {
                return nullopt;
        }
}

bool IcalParser::binary() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       float      = (["+"] / "-") 1*DIGIT ["." 1*DIGIT]
optional<string> IcalParser::float_() {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;

        // (["+"] / "-")
        if (auto v = token("+")) ret += *v;
        else if (auto v = token("-")) ret += *v;

        // 1*DIGIT
        if (auto v = digit()) ret += *v;
        else return nullopt;

        while (auto v = digit()) {
                ret += *v;
        }

        // ["." 1*DIGIT]
        if (auto dot = token(".")) {
                ret += *dot;

                if (auto v = digit()) ret += *v;
                else return nullopt;

                while (auto v = digit()) {
                        ret += *v;
                }
        }

        ptran.commit();
        return ret;
}

//       integer    = (["+"] / "-") 1*DIGIT
optional<int> IcalParser::integer() {
        CALLSTACK;
        save_input_pos ptran(is);

        string raw;
        // (["+"] / "-")
        if (auto v = token("+")) raw += *v;
        else if (auto v = token("-")) raw += *v;

        // 1*DIGIT
        if (auto v = digit()) raw += *v;
        else return nullopt;

        while (auto v = digit()) {
                raw += *v;
        }
        const auto ret = std::stoi(raw);
        ptran.commit();
        return ret;
}

//       actionvalue = "AUDIO" / "DISPLAY" / "EMAIL" / iana-token / x-name
optional<string> IcalParser::actionvalue() {
        CALLSTACK;
        save_input_pos ptran(is);
        string ret;

        if (auto v = token("AUDIO")) ret = *v;
        else if (auto v = token("DISPLAY")) ret = *v;
        else if (auto v = token("EMAIL")) ret = *v;
        else if (auto v = iana_token()) ret = *v;
        else if (auto v = x_name()) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}
//       actionparam = *(";" other-param)
optional<ActionParam> IcalParser::actionparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        ActionParam ret;

        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt;
        }
        ptran.commit();
        return ret;
}
//       action      = "ACTION" actionparam ":" actionvalue CRLF
optional<Action> IcalParser::action() {
        CALLSTACK;
        save_input_pos ptran(is);
        Action ret;
        if (!token("ACTION")) return nullopt;

        if (auto v = actionparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = actionvalue()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
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
optional<TrigAbs> IcalParser::trigabs() {
        CALLSTACK;
        save_input_pos ptran(is);
        TrigAbs ret;

        while (token(";")) {
                {
                        save_input_pos ptran(is);
                        auto match = token("VALUE") &&
                                     token("=") &&
                                     token("DATE-TIME");
                        if (match) {
                                ret.value = "DATE-TIME";
                                ptran.commit();
                                continue;
                        }
                }

                if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;// error
                }
        }

        if (!token(":")) return nullopt;

        if (auto v = date_time()) ret.dateTime = *v;
        else return nullopt;

        ptran.commit();
        return ret;
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

optional<TrigRel> IcalParser::trigrel() {
        CALLSTACK;
        save_input_pos ptran(is);
        TrigRel ret;

        while (token(";")) {
                {
                        save_input_pos ptran(is);
                        auto match = token("VALUE") &&
                                     token("=") &&
                                     token("DURATION");
                        if (match) {
                                ret.value = "DURATION";
                                ptran.commit();
                                continue;
                        }
                }

                if (auto v = trigrelparam()) {
                        ret.trigRelParam = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;// error
                }
        }

        if (!token(":")) return nullopt;

        if (auto v = dur_value()) ret.durValue = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}
//       trigger    = "TRIGGER" (trigrel / trigabs) CRLF
optional<Trigger> IcalParser::trigger() {
        CALLSTACK;
        save_input_pos ptran(is);
        Trigger ret;

        if (!token("TRIGGER")) return nullopt;

        if (auto v = trigrel()) ret = *v;
        else if (auto v = trigabs()) ret = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}
//       repparam   = *(";" other-param)
optional<RepParam> IcalParser::repparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        RepParam ret;
        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt; // error
        }
        ptran.commit();
        return ret;
}
//       repeat  = "REPEAT" repparam ":" integer CRLF  ;Default is "0", zero.
optional<Repeat> IcalParser::repeat() {
        CALLSTACK;
        save_input_pos ptran(is);
        Repeat ret;

        if (!token("REPEAT")) return nullopt;

        if (auto v = repparam()) ret.params = *v;
        else return nullopt; // error

        if (!token(":")) return nullopt;

        if (auto v = integer()) ret.value = *v;
        else return nullopt; // error

        if (!newline()) return nullopt; // error

        ptran.commit();
        return ret;
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
optional<AudioProp> IcalParser::audioprop() {
        CALLSTACK;
        save_input_pos ptran(is);
        AudioProp ret;

        bool req_act = false, req_trig = false;
        while(true) {
                if (auto v = action()) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = trigger()) {
                        req_trig = true;
                        ret.trigger = *v;
                }
                else if (auto v = duration())
                        ret.duration = *v;
                else if (auto v = repeat())
                        ret.repeat = *v;
                else if (auto v = attach())
                        ret.attach = *v;
                else if (auto v = x_prop())
                        ret.xProps.push_back(*v);
                else if (auto v = iana_prop())
                        ret.ianaProps.push_back(*v);
                else break;
        }
        // std::cerr << "audioprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        if (!(req_act && req_trig)) {
                return nullopt;
        }
        ptran.commit();
        return ret;
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
optional<DispProp> IcalParser::dispprop() {
        CALLSTACK;
        save_input_pos ptran(is);
        DispProp ret;

        bool req_act = false, req_desc = false, req_trig = false;
        while(true) {
                if (auto v = action()) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = description()) {
                        req_desc = true;
                        ret.description = *v;
                }
                else if (auto v = trigger()) {
                        req_trig = true;
                        ret.trigger = *v;
                }

                else if (auto v = duration()) ret.duration = *v;
                else if (auto v = repeat()) ret.repeat = *v;

                else if (auto v = x_prop()) ret.xProps.push_back(*v);
                else if (auto v = iana_prop()) ret.ianaProps.push_back(*v);

                else break;
        }

        // std::cerr << "dispprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has desc   : " << (req_desc?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        if (!(req_act && req_desc && req_trig)) {
                 return nullopt;
        }

        ptran.commit();
        return ret;
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
optional<EmailProp> IcalParser::emailprop() {
        CALLSTACK;
        save_input_pos ptran(is);
        EmailProp ret;

        bool req_act = false,
             req_desc = false,
             req_trig = false,
             req_summ = false;
        while(true) {
                if (auto v = action()) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = description()) {
                        req_desc = true;
                        ret.description = *v;
                }
                else if (auto v = trigger()) {
                        req_trig = true;
                        ret.trigger = *v;
                }
                else if (auto v = summary()) {
                        req_summ = true;
                        ret.summary = *v;
                }

                else if (auto v = attendee()) ret.attendee = *v;

                else if (auto v = duration()) ret.duration = *v;
                else if (auto v = repeat()) ret.repeat = *v;

                else if (auto v = attach()) ret.attach.push_back(*v);
                else if (auto v = x_prop()) ret.xProps.push_back(*v);
                else if (auto v = iana_prop()) ret.ianaProps.push_back(*v);

                else break;
        }

        // std::cerr << "emailprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has desc   : " << (req_desc?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        // std::cerr << "  has summ   : " << (req_summ?"yes":"no") << std::endl;
        if (!(req_act && req_desc && req_trig && req_summ)) {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       alarmc     = "BEGIN" ":" "VALARM" CRLF
//                    (audioprop / dispprop / emailprop)
//                    "END" ":" "VALARM" CRLF
optional<Alarm> IcalParser::alarmc() {
        CALLSTACK;
        save_input_pos ptran(is);
        Alarm ret;

        if (!key_value("BEGIN", "VALARM")) return nullopt;

        // Compared to the grammar, we invert the checking-order
        // because the required fields overlap, so we check for most
        // specialized first.

        if (auto v = emailprop()) ret = *v;
        else if (auto v = dispprop()) ret = *v;
        else if (auto v = audioprop()) ret = *v;
        else return nullopt; // TODO: Error, not empty

        if (!key_value("END", "VALARM"))
                return nullopt; // TODO: Error, not empty

        ptran.commit();
        return ret;
}

//       date-fullyear      = 4DIGIT
optional<string> IcalParser::date_fullyear() {
        CALLSTACK;
        return digits(4);
}

//       date-month         = 2DIGIT        ;01-12
optional<string> IcalParser::date_month() {
        CALLSTACK;
        return digits(2);
}

//       date-mday          = 2DIGIT        ;01-28, 01-29, 01-30, 01-31
//                                          ;based on month/year
optional<string> IcalParser::date_mday() {
        CALLSTACK;
        return digits(2);
}

//       date-value         = date-fullyear date-month date-mday
optional<Date> IcalParser::date_value() {
        CALLSTACK;
        save_input_pos ptran(is);
        Date ret;

        if (auto v = date_fullyear()) ret.year = *v;
        else return nullopt;

        if (auto v = date_month()) ret.month = *v;
        else return nullopt;

        if (auto v = date_mday()) ret.day = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       date               = date-value
optional<Date> IcalParser::date() {
        CALLSTACK;
        return date_value();
}

//       time-hour    = 2DIGIT        ;00-23
optional<TimeHour> IcalParser::time_hour() {
        CALLSTACK;
        save_input_pos ptran(is);
        TimeHour ret;
        if (auto v = digits(2)) ret.value = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//       time-minute  = 2DIGIT        ;00-59
optional<TimeMinute> IcalParser::time_minute() {
        CALLSTACK;
        save_input_pos ptran(is);
        TimeMinute ret;
        if (auto v = digits(2)) ret.value = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//       time-second  = 2DIGIT        ;00-60
//       ;The "60" value is used to account for positive "leap" seconds.
optional<TimeSecond> IcalParser::time_second() {
        CALLSTACK;
        save_input_pos ptran(is);
        TimeSecond ret;
        if (auto v = digits(2)) ret.value = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//       time-utc     = "Z"
optional<string> IcalParser::time_utc() {
        CALLSTACK;
        return token("Z");
}

//       time         = time-hour time-minute time-second [time-utc]
optional<Time> IcalParser::time() {
        CALLSTACK;
        save_input_pos ptran(is);
        Time ret;

        if (auto v = time_hour()) ret.hour = *v;
        else return nullopt;

        if (auto v = time_minute()) ret.minute = *v;
        else return nullopt;

        if (auto v = time_second()) ret.second = *v;
        else return nullopt;

        if (auto v = time_utc()) ret.utc = v;

        ptran.commit();
        return ret;
}

//       date-time  = date "T" time ;As specified in the DATE and TIME
//                                  ;value definitions
optional<DateTime> IcalParser::date_time() {
        CALLSTACK;
        save_input_pos ptran(is);
        DateTime ret;

        if (auto v = date()) ret.date = *v;
        else return nullopt;

        if (!token("T")) return nullopt;

        if (auto v = time()) ret.time = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       dur-week   = 1*DIGIT "W"
optional<DurWeek> IcalParser::dur_week() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurWeek ret;

        if (auto v = digits(1, -1)) ret.value = *v;
        else return nullopt;

        if (!token("W")) return nullopt;

        ptran.commit();
        return ret;
}

//       dur-second = 1*DIGIT "S"
optional<DurSecond> IcalParser::dur_second() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurSecond ret;

        if (auto v = digits(1, -1)) ret.second = *v;
        else return nullopt;

        if (!token("S")) return nullopt;

        ptran.commit();
        return ret;
}

//       dur-minute = 1*DIGIT "M" [dur-second]
optional<DurMinute> IcalParser::dur_minute() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurMinute ret;

        if (auto v = digits(1, -1)) ret.minute = *v;
        else return nullopt;

        if (!token("M")) return nullopt;

        if (auto v = dur_second()) ret.second = *v;

        ptran.commit();
        return ret;
}

//       dur-hour   = 1*DIGIT "H" [dur-minute]
optional<DurHour> IcalParser::dur_hour() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurHour ret;

        if (auto v = digits(1, -1)) ret.hour = *v;
        else return nullopt;

        if (!token("H")) return nullopt;

        if (auto v = dur_minute()) ret.minute = *v;

        ptran.commit();
        return ret;
}

//       dur-day    = 1*DIGIT "D"
optional<DurDay> IcalParser::dur_day() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurDay ret;

        if (auto v = digits(1, -1)) ret.value += *v;
        else return nullopt;

        if (!token("D")) return nullopt;

        ptran.commit();
        return ret;
}

//       dur-time   = "T" (dur-hour / dur-minute / dur-second)
optional<DurTime> IcalParser::dur_time() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurTime ret;

        if (!token("T")) return nullopt;

        if (auto v = dur_hour()) {
                ret = *v;
        }
        else if (auto v = dur_minute()) {
                ret = *v;
        }
        else if (auto v = dur_second()) {
                ret = *v;
        }
        else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       dur-date   = dur-day [dur-time]
optional<DurDate> IcalParser::dur_date() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurDate ret;

        if (auto v = dur_day()) ret.day = *v;
        else return nullopt;

        if (auto v = dur_time()) ret.time = *v;

        ptran.commit();
        return ret;
}

//       dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)
optional<DurValue> IcalParser::dur_value() {
        CALLSTACK;
        save_input_pos ptran(is);
        DurValue ret;

        if(token("+")) ret.positive = true;
        else if (token("-")) ret.positive = false;

        if (!token("P")) return nullopt; // error

        if (auto v = dur_date()) {
                ret.value = *v;
        }
        else if (auto v = dur_time()) {
                ret.value = *v;
        }
        else if (auto v = dur_week()) {
                ret.value = *v;
        }
        else return nullopt; // error

        ptran.commit();
        return ret;
}

//       period-start = date-time "/" dur-value
//       ; [ISO.8601.2004] complete representation basic format for a
//       ; period of time consisting of a start and positive duration
//       ; of time.
bool IcalParser::period_start() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                date_time() &&
                token("/") &&
                dur_value() ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//       period-explicit = date-time "/" date-time
bool IcalParser::period_explicit() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                date_time() &&
                token("/") &&
                date_time() ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//       ; [ISO.8601.2004] complete representation basic format for a
//       ; period of time consisting of a start and end.  The start MUST
//       ; be before the end.
//       period     = period-explicit / period-start
bool IcalParser::period() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                period_explicit() ||
                period_start() ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//     other-param   = (iana-param / x-param)
optional<OtherParam> IcalParser::other_param() {
        CALLSTACK;
        if (auto v = iana_param()) return OtherParam{*v};
        if (auto v = x_param()) return OtherParam{*v};
        return optional<OtherParam>();
}

// stmparam   = *(";" other-param)
optional<DtStampParams> IcalParser::stmparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtStampParams ret;

        while (token(";")) {
                if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

// dtstamp    = "DTSTAMP" stmparam ":" date-time CRLF
optional<DtStamp> IcalParser::dtstamp() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtStamp ret;

        if (!token("DTSTAMP")) return nullopt;

        if (auto v = stmparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = date_time()) ret.date_time = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}
//       uidparam   = *(";" other-param)
optional<vector<OtherParam>> IcalParser::uidparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<OtherParam> ret;
        while (token(";")) {
                if (auto v = other_param()) {
                        ret.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}
//       uid        = "UID" uidparam ":" text CRLF
optional<Uid> IcalParser::uid() {
        CALLSTACK;
        save_input_pos ptran(is);
        Uid ret;

        if (!token("UID")) return nullopt;

        if (auto v = uidparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = text()) ret.value = *v;
        else return nullopt;

        if (!newline())  return nullopt;

        ptran.commit();
        return ret;
}
//       dtstval    = date-time / date
optional<DtStartVal> IcalParser::dtstval() {
        CALLSTACK;
        if (auto v = date_time()) return DtStartVal{*v};
        if (auto v = date()) return DtStartVal{*v};
        return optional<DtStartVal>();
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
optional<DtStartParams> IcalParser::dtstparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtStartParams ret;

        while (token(";")) {
                if (token("VALUE")) {
                        if (!token("="))
                                return nullopt;

                        if (auto v = token("DATE-TIME")) {
                                ret.value = *v;
                        } else if (auto v = token("DATE")) {
                                ret.value = *v;
                        } else {
                                return nullopt;
                        }
                } else if (auto v = tzidparam()) {
                        ret.tz_id = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}
//       dtstart    = "DTSTART" dtstparam ":" dtstval CRLF
//
optional<DtStart> IcalParser::dtstart() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtStart ret;

        if (!token("DTSTART")) return nullopt;

        if (auto v = dtstparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = dtstval()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       classvalue = "PUBLIC" / "PRIVATE" / "CONFIDENTIAL" / iana-token
//                  / x-name
//       ;Default is PUBLIC
optional<string> IcalParser::classvalue() {
        CALLSTACK;
        if (auto v = token("PUBLIC")) return v;
        if (auto v = token("PRIVATE")) return v;
        if (auto v = token("CONFIDENTIAL")) return v;
        if (auto v = iana_token()) return v;
        if (auto v = x_name()) return v;
        else return nullopt;
}

//       classparam = *(";" other-param)
optional<ClassParams> IcalParser::classparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        ClassParams ret;

        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt;
        }
        ptran.commit();
        return ret;
}

//       class      = "CLASS" classparam ":" classvalue CRLF
optional<Class> IcalParser::class_() {
        CALLSTACK;
        save_input_pos ptran(is);
        Class ret;

        if (!token("CLASS")) return nullopt;

        if (auto v = classparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = classvalue()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}
//       creaparam  = *(";" other-param)
bool IcalParser::creaparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       created    = "CREATED" creaparam ":" date-time CRLF
optional<Created> IcalParser::created() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("CREATED") &&
                creaparam() &&
                token(":") &&
                date_time() &&
                newline();
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
optional<DescParams> IcalParser::descparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DescParams ret;
        while(token(";")) {
                if (auto v = altrepparam()) {
                        ret.alt_rep = *v;
                } else if (auto v = languageparam()) {
                        ret.language = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        std::cerr << " unknown" << std::endl;
                        // TODO: warn
                }
        }
        ptran.commit();
        return ret;
}
//       description = "DESCRIPTION" descparam ":" text CRLF
optional<Description> IcalParser::description() {
        CALLSTACK;
        save_input_pos ptran(is);
        Description ret;

        if (!token("DESCRIPTION")) return nullopt;

        if (auto v = descparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = text()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       geovalue   = float ";" float
//       ;Latitude and Longitude components
optional<GeoValue> IcalParser::geovalue() {
        CALLSTACK;
        save_input_pos ptran(is);
        GeoValue ret;
        if (auto v = float_()) ret.latitude = *v;
        else return nullopt;

        if (!token(";")) return nullopt;

        if (auto v = float_()) ret.longitude = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}
//       geoparam   = *(";" other-param)
optional<GeoParams> IcalParser::geoparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        GeoParams ret;
        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt;
        }
        ptran.commit();
        return ret;
}
//       geo        = "GEO" geoparam ":" geovalue CRLF
optional<Geo> IcalParser::geo() {
        CALLSTACK;
        save_input_pos ptran(is);
        Geo ret;

        if (!token("GEO")) return nullopt;

        if (auto v = geoparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = geovalue()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}
//       lstparam   = *(";" other-param)
bool IcalParser::lstparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       last-mod   = "LAST-MODIFIED" lstparam ":" date-time CRLF
optional<LastMod> IcalParser::last_mod() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("LAST-MODIFIED") &&
                lstparam() &&
                token(":") &&
                date_time() &&
                newline();
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
//                  (";" altrepparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
optional<LocParams> IcalParser::locparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        LocParams ret;
        while(token(";")) {
                if (auto v = altrepparam()) {
                        ret.alt_rep = *v;
                } else if (auto v = languageparam()) {
                        ret.language = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        std::cerr << " unknown" << std::endl;
                        // TODO: warn
                }
        }
        ptran.commit();
        return ret;
}
//       location   = "LOCATION"  locparam ":" text CRLF
optional<Location> IcalParser::location() {
        CALLSTACK;
        save_input_pos ptran(is);
        Location ret;
        if (!token("LOCATION"))
                return nullopt;

        if (auto v = locparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = text()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();

        return ret;
}
//       orgparam   = *(
//                  ;
//                  ; The following are OPTIONAL,
//                  ; but MUST NOT occur more than once.
//                  ;
//                  (";" cnparam) /
//                  (";" dirparam) /
//                  (";" sentbyparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
optional<OrgParams> IcalParser::orgparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        OrgParams ret;
        while(token(";")) {
                if (auto v = cnparam()) {
                        ret.cn = *v;
                } else if (auto v = dirparam()) {
                        ret.dir = *v;
                } else if (auto v = sentbyparam()) {
                        ret.sentBy = *v;
                } else if (auto v = languageparam()) {
                        ret.language = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        std::cerr << " unknown" << std::endl;
                        // TODO: warn
                }
        }
        ptran.commit();
        return ret;
}
//       cal-address        = uri
optional<Uri> IcalParser::cal_address() {
        CALLSTACK;
        return uri();
}
//       organizer  = "ORGANIZER" orgparam ":" cal-address CRLF
optional<Organizer> IcalParser::organizer() {
        CALLSTACK;
        save_input_pos ptran(is);
        Organizer ret;

        if (!token("ORGANIZER"))
                return nullopt;

        if (auto v = orgparam()) {
                ret.params = *v;
        } else {
                return nullopt;
        }

        if (!token(":"))
                return nullopt;

        if (auto v = cal_address()) {
                ret.address = *v;
        } else {
                return nullopt;
        }

        if (!newline())
                return nullopt;

        ptran.commit();
        return ret;
}

//       priovalue   = integer       ;Must be in the range [0..9]
//          ; All other values are reserved for future use.
bool IcalParser::priovalue() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       prioparam  = *(";" other-param)
bool IcalParser::prioparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       priority   = "PRIORITY" prioparam ":" priovalue CRLF
//       ;Default is zero (i.e., undefined).
optional<Priority> IcalParser::priority() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("PRIORITY") &&
                prioparam() &&
                token(":") &&
                priovalue() &&
                newline();
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       seqparam   = *(";" other-param)
optional<SeqParams> IcalParser::seqparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        SeqParams ret;
        while (auto v = other_param())
                ret.params.push_back(*v);
        ptran.commit();
        return ret;
}
//       seq = "SEQUENCE" seqparam ":" integer CRLF     ; Default is "0"
optional<Seq> IcalParser::seq() {
        CALLSTACK;
        save_input_pos ptran(is);
        Seq ret;

        if (!token("SEQUENCE")) return nullopt;

        if (auto v = seqparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = integer()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}
//       statvalue-jour  = "DRAFT"        ;Indicates journal is draft.
//                       / "FINAL"        ;Indicates journal is final.
//                       / "CANCELLED"    ;Indicates journal is removed.
//      ;Status values for "VJOURNAL".
bool IcalParser::statvalue_jour() {
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
bool IcalParser::statvalue_todo() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}


//       statvalue       = (statvalue-event
//                       /  statvalue-todo
//                       /  statvalue-jour)
bool IcalParser::statvalue() {
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
bool IcalParser::statvalue_event() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       statparam       = *(";" other-param)
bool IcalParser::statparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       status          = "STATUS" statparam ":" statvalue CRLF
optional<Status> IcalParser::status() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("STATUS") &&
                statparam() &&
                token(":") &&
                statvalue() &&
                newline();
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
//                  (";" altrepparam) /
//                  (";" languageparam) /
//                  ;
//                  ; The following is OPTIONAL,
//                  ; and MAY occur more than once.
//                  ;
//                  (";" other-param)
//                  ;
//                  )
optional<SummParams> IcalParser::summparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        SummParams ret;
        while(token(";")) {
                if (auto v = altrepparam()) {
                        ret.alt_rep = *v;
                } else if (auto v = languageparam()) {
                        ret.language = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        std::cerr << " unknown" << std::endl;
                        // TODO: warn
                }
        }
        ptran.commit();
        return ret;
}

//       summary    = "SUMMARY" summparam ":" text CRLF
optional<Summary> IcalParser::summary() {
        CALLSTACK;
        save_input_pos ptran(is);
        Summary ret;
        if (!token("SUMMARY"))
                return nullopt;

        if (auto v = summparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = text()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       transparam = *(";" other-param)
bool IcalParser::transparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (token(";")) {
                if (!other_param())
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
bool IcalParser::transvalue() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("OPAQUE") ||
                token("TRANSPARENT");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       transp     = "TRANSP" transparam ":" transvalue CRLF
optional<Transp> IcalParser::transp() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("TRANSP") &&
                transparam() &&
                token(":") &&
                transvalue() &&
                newline();
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//      uri = <As defined in Section 3 of [RFC3986]>
optional<Uri> IcalParser::uri() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto uri = rfc3986::read_URI(is);
        if (!uri)
                return nullopt;
        ptran.commit();
        return uri;
}
//       urlparam   = *(";" other-param)
bool IcalParser::urlparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       url        = "URL" urlparam ":" uri CRLF
optional<Url> IcalParser::url() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("URL") &&
                urlparam() &&
                token(":") &&
                uri() &&
                newline();
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       ridval     = date-time / date
//       ;Value MUST match value type
bool IcalParser::ridval() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success = date_time() || date();
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
bool IcalParser::ridparam_single() {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE"))
        {
                save_input_pos ptran(is);
                const auto success =
                        token(";") &&
                        token("VALUE") &&
                        token("=") &&
                        (token("DATE-TIME") || token("DATE"));
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" tzidparam) / (";" rangeparam)
        {
                save_input_pos ptran(is);
                const auto success =
                        token(";") &&
                        (tzidparam() || rangeparam());
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto success =
                        token(";") &&
                        other_param();
                if (success) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool IcalParser::ridparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (ridparam_single()) {
        }
        ptran.commit();
        return true;
}

//       setposday   = yeardaynum
bool IcalParser::setposday() {
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
bool IcalParser::bysplist() {
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
bool IcalParser::monthnum() {
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
bool IcalParser::bymolist() {
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
bool IcalParser::weeknum() {
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
bool IcalParser::bywknolist() {
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
bool IcalParser::ordyrday() {
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
bool IcalParser::yeardaynum() {
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
bool IcalParser::byyrdaylist() {
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
bool IcalParser::ordmoday() {
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
bool IcalParser::monthdaynum() {
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
bool IcalParser::bymodaylist() {
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
bool IcalParser::weekday() {
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
bool IcalParser::ordwk() {
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
bool IcalParser::minus() {
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
bool IcalParser::plus() {
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
bool IcalParser::weekdaynum() {
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
bool IcalParser::bywdaylist() {
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
bool IcalParser::hour() {
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
bool IcalParser::byhrlist() {
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
bool IcalParser::minutes() {
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
bool IcalParser::byminlist() {
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
bool IcalParser::seconds() {
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
bool IcalParser::byseclist() {
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
bool IcalParser::enddate() {
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
bool IcalParser::freq() {
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
bool IcalParser::recur_rule_part() {
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
bool IcalParser::recur() {
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
optional<RecurId> IcalParser::recurid() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("RECURRENCE-ID") &&
                ridparam() &&
                token(":") &&
                ridval() &&
                newline();
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       rrulparam  = *(";" other-param)
bool IcalParser::rrulparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}

//       rrule      = "RRULE" rrulparam ":" recur CRLF
optional<RRule> IcalParser::rrule() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("RRULE") &&
                rrulparam() &&
                token(":") &&
                recur() &&
                newline();
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       dtendval   = date-time / date
//       ;Value MUST match value type
optional<DtEndVal> IcalParser::dtendval() {
        CALLSTACK;
        if (auto v = date_time()) return DtEndVal{*v};
        if (auto v = date()) return DtEndVal{*v};
        return optional<DtStartVal>();
}

//       dtendparam = *(
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
optional<DtEndParams> IcalParser::dtendparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtEndParams ret;

        while (token(";")) {
                if (token("VALUE")) {
                        if (!token("="))
                                return nullopt;

                        if (auto v = token("DATE-TIME")) {
                                ret.value = *v;
                        } else if (auto v = token("DATE")) {
                                ret.value = *v;
                        } else {
                                return nullopt;
                        }
                } else if (auto v = tzidparam()) {
                        ret.tz_id = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//       dtend      = "DTEND" dtendparam ":" dtendval CRLF
optional<DtEnd> IcalParser::dtend() {
        CALLSTACK;
        save_input_pos ptran(is);
        DtEnd ret;

        if (!token("DTEND")) return nullopt;

        if (auto v = dtendparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = dtendval()) ret.value = *v;
        else return nullopt;

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       durparam   = *(";" other-param)
bool IcalParser::durparam() {
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
optional<Duration> IcalParser::duration() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("DURATION") &&
                durparam() &&
                token(":") &&
                dur_value() &&
                newline();
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
bool IcalParser::attachparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token(";") &&
                (fmttypeparam() || other_param());
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
optional<Attach> IcalParser::attach() {
        CALLSTACK;
        save_input_pos ptran(is);

        const auto match_head =
                token("ATTACH") && attachparam();
        if (!match_head)
                return nullopt;

        // ":" uri
        {
                save_input_pos ptran_uri(is);
                const auto match_uri =
                        token(":") &&
                        uri() &&
                        newline() ;
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
                        token(";") &&
                        token("ENCODING") &&
                        token("=") &&
                        token("BASE64") &&
                        token(";") &&
                        token("VALUE") &&
                        token("=") &&
                        token("BINARY") &&
                        token(":") &&
                        binary() &&
                        newline() ;
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
bool IcalParser::attparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       attendee   = "ATTENDEE" attparam ":" cal-address CRLF
optional<Attendee> IcalParser::attendee() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                token("ATTENDEE") &&
                attparam() &&
                token(":") &&
                cal_address() &&
                newline();
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
optional<CatParams> IcalParser::catparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        CatParams ret;
        while (token(";")) {
                if (auto v = languageparam()) {
                        ret.language = *v;
                } else if (auto v = other_param()) {
                        ret.params.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}
//       categories = "CATEGORIES" catparam ":" text *("," text)
//                    CRLF
optional<Categories> IcalParser::categories () {
        CALLSTACK;
        save_input_pos ptran(is);
        Categories ret;

        if (!token("CATEGORIES")) return nullopt;

        if (auto v = catparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt;

        if (auto v = text()) ret.values.push_back(*v);
        else return nullopt;

        while (token(",")) {
                if (auto v = text()) ret.values.push_back(*v);
                else return nullopt;
        }

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
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
bool IcalParser::commparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (token(";")) {
                const auto match =
                        altrepparam() ||
                        languageparam() ||
                        other_param();
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       comment    = "COMMENT" commparam ":" text CRLF
optional<Comment> IcalParser::comment() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                token("COMMENT") &&
                commparam() &&
                token(":") &&
                text() &&
                newline();
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
bool IcalParser::contparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (token(";")) {
                const auto match =
                        altrepparam() ||
                        languageparam() ||
                        other_param();
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       contact    = "CONTACT" contparam ":" text CRLF
optional<Contact> IcalParser::contact() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                token("CONTACT") &&
                contparam() &&
                token(":") &&
                text() &&
                newline();
        if (!match)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       exdtval    = date-time / date
//       ;Value MUST match value type
bool IcalParser::exdtval() {
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
bool IcalParser::exdtparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       exdate     = "EXDATE" exdtparam ":" exdtval *("," exdtval) CRLF
optional<ExDate> IcalParser::exdate() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto match =
                token("EXDATE") &&
                exdtparam() &&
                token(":") &&
                exdtval();
        if (!match)
                return nullopt;
        while (token(",")) {
                if (!exdtval())
                        return nullopt;
        }
        if (!newline())
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}

//       extdata    = text
//       ;Textual exception data.  For example, the offending property
//       ;name and value or complete property line.
bool IcalParser::extdata() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!text())
                return false;
        ptran.commit();
        return true;
}

//       statdesc   = text
//       ;Textual status description
bool IcalParser::statdesc() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!text())
                return false;
        ptran.commit();
        return true;
}

//       statcode   = 1*DIGIT 1*2("." 1*DIGIT)
//       ;Hierarchical, numeric return status code
bool IcalParser::statcode() {
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
bool IcalParser::rstatparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (token(";")) {
                const auto match =
                        languageparam() ||
                        other_param();
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}

//       rstatus    = "REQUEST-STATUS" rstatparam ":"
//                    statcode ";" statdesc [";" extdata]
optional<RStatus> IcalParser::rstatus() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("REQUEST-STATUS") &&
                rstatparam() &&
                token(":") &&
                statcode() &&
                token(";") &&
                statdesc() &&
                (token(";") ? extdata() : true);
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
optional<RelTypeParam> IcalParser::reltypeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        RelTypeParam ret;

        if (!token("RELTYPE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("PARENT")) {
                ret.value = *v;
        } else if (auto v = token("CHILD")) {
                ret.value = *v;
        } else if (auto v = token("SIBLING")) {
                ret.value = *v;
        } else if (auto v = iana_token()) {
                ret.value = *v;
        } else if (auto v = x_name()) {
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
bool IcalParser::relparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (token(";")) {
                const auto match =
                        reltypeparam() ||
                        other_param();
                if (!match)
                        return false;
        }
        ptran.commit();
        return true;
}
//       related    = "RELATED-TO" relparam ":" text CRLF
optional<Related> IcalParser::related() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("RELATED-TO") &&
                relparam() &&
                token(":") &&
                text() &&
                newline();
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
bool IcalParser::resrcparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(is);
        ptran.commit();
        return true;
}
//       resources  = "RESOURCES" resrcparam ":" text *("," text) CRLF
optional<Resources> IcalParser::resources() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("RESOURCES") &&
                resrcparam() &&
                token(":") &&
                text();
        if (!success)
                return nullopt;
        while (token(",")) {
                if (!text())
                        return nullopt;
        }
        if (!newline())
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
}
//       rdtval     = date-time / date / period
//       ;Value MUST match value type
bool IcalParser::rdtval() {
        CALLSTACK;
        return date_time() || date() || period();
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
bool IcalParser::rdtparam_single() {
        CALLSTACK;
        // (";" "VALUE" "=" ("DATE-TIME" / "DATE" / "PERIOD"))
        {
                save_input_pos ptran(is);
                const auto match =
                        token(";") &&
                        token("VALUE") &&
                        token("=") &&
                        (
                                date_time() ||
                                date() ||
                                period()
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
                        token(";") &&
                        tzidparam();
                if (match) {
                        ptran.commit();
                        return true;
                }
        }
        // (";" other-param)
        {
                save_input_pos ptran(is);
                const auto match =
                        token(";") &&
                        other_param();
                if (match) {
                        ptran.commit();
                        return true;
                }
        }
        return false;
}
bool IcalParser::rdtparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        while (rdtparam_single()) {
        }
        ptran.commit();
        return true;
}
//       rdate      = "RDATE" rdtparam ":" rdtval *("," rdtval) CRLF
optional<RDate> IcalParser::rdate() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                token("RDATE") &&
                rdtparam() &&
                token(":") &&
                rdtval();
        if (!success)
                return nullopt;
        while (token(",")) {
                if (!rdtval())
                        return nullopt;
        }
        if (!newline())
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
optional<EventProp> IcalParser::eventprop_single() {
        CALLSTACK;
        save_input_pos ptran(is);
        EventProp ret;
        if (auto v = dtstamp()) ret = *v;
        else if (auto v = uid()) ret = *v;

        else if (auto v = dtstart()) ret = *v;

        else if (auto v = class_()) ret = *v;
        else if (auto v = created()) ret = *v;
        else if (auto v = description()) ret = *v;
        else if (auto v = geo()) ret = *v;
        else if (auto v = last_mod()) ret = *v;
        else if (auto v = location()) ret = *v;
        else if (auto v = organizer()) ret = *v;
        else if (auto v = priority()) ret = *v;
        else if (auto v = seq()) ret = *v;
        else if (auto v = status()) ret = *v;
        else if (auto v = summary()) ret = *v;
        else if (auto v = transp()) ret = *v;
        else if (auto v = url()) ret = *v;
        else if (auto v = recurid()) ret = *v;

        else if (auto v = rrule()) ret = *v;

        else if (auto v = dtend()) ret = *v;
        else if (auto v = duration()) ret = *v;

        else if (auto v = attach()) ret = *v;
        else if (auto v = attendee()) ret = *v;
        else if (auto v = categories ()) ret = *v;
        else if (auto v = comment()) ret = *v;
        else if (auto v = contact()) ret = *v;
        else if (auto v = exdate()) ret = *v;
        else if (auto v = rstatus()) ret = *v;
        else if (auto v = related()) ret = *v;
        else if (auto v = resources()) ret = *v;
        else if (auto v = rdate()) ret = *v;
        else if (auto v = x_prop()) ret = *v;
        else if (auto v = iana_prop()) ret = *v;

        else return nullopt;

        ptran.commit();
        return ret;
}
optional<vector<EventProp>> IcalParser::eventprop() {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<EventProp> ret;
        while (auto v = eventprop_single()) {
                ret.push_back(*v);
        }

        ptran.commit();
        return ret;
}

//       eventc     = "BEGIN" ":" "VEVENT" CRLF
//                    eventprop *alarmc
//                    "END" ":" "VEVENT" CRLF
optional<EventComp> IcalParser::eventc() {
        CALLSTACK;
        save_input_pos ptran(is);
        EventComp ret;

        if (!key_value("BEGIN", "VEVENT")) return nullopt;

        if (auto v = eventprop()) ret.properties = *v;
        else return nullopt;

        while(auto v = alarmc()) {
                ret.alarms.push_back(*v);
        }

        if (!key_value("END", "VEVENT")) return nullopt;

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
bool IcalParser::todoprop() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       todoc      = "BEGIN" ":" "VTODO" CRLF
//                    todoprop *alarmc
//                    "END" ":" "VTODO" CRLF
optional<TodoComp> IcalParser::todoc() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                key_value("BEGIN", "VTODO") &&
                todoprop();
        if (!success_pro)
                return nullopt;

        while(alarmc()) {
        }

        const auto success_epi =
                key_value("END", "VTODO") ;
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
bool IcalParser::jourprop() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       journalc   = "BEGIN" ":" "VJOURNAL" CRLF
//                    jourprop
//                    "END" ":" "VJOURNAL" CRLF
optional<JournalComp> IcalParser::journalc() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                key_value("BEGIN", "VJOURNAL") &&
                jourprop() &&
                key_value("END", "VJOURNAL") ;
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
bool IcalParser::fbprop() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       freebusyc  = "BEGIN" ":" "VFREEBUSY" CRLF
//                    fbprop
//                    "END" ":" "VFREEBUSY" CRLF
optional<FreeBusyComp> IcalParser::freebusyc() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success =
                key_value("BEGIN", "VFREEBUSY") &&
                fbprop() &&
                key_value("END", "VFREEBUSY") ;
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       tzidpropparam      = *(";" other-param)
optional<TzIdPropParam> IcalParser::tzidpropparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzIdPropParam ret;
        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt; // error
        }
        ptran.commit();
        return ret;
}

//       tzid       = "TZID" tzidpropparam ":" [tzidprefix] text CRLF
optional<TzId> IcalParser::tzid() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzId ret;

        if (!token("TZID")) return nullopt;

        if (auto v = tzidpropparam()) ret.propParams = *v;
        else return nullopt; // error

        if (!token(":")) return nullopt; // error

        if (auto v = tzidprefix()) ret.prefix = *v;

        if (auto v = text()) ret.text = *v;
        else return nullopt; // error

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       tzurlparam = *(";" other-param)
optional<TzUrlParam> IcalParser::tzurlparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzUrlParam ret;
        while (token(";")) {
                if (auto v = other_param()) ret.params.push_back(*v);
                else return nullopt; // error
        }
        ptran.commit();
        return ret;
}

//       tzurl      = "TZURL" tzurlparam ":" uri CRLF
optional<TzUrl> IcalParser::tzurl() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzUrl ret;

        if (!token("TZURL")) return nullopt;

        if (auto v = tzurlparam()) ret.params = *v;
        else return nullopt;

        if (!token(":")) return nullopt; // error

        if (auto v = uri()) ret.uri = *v;
        else return nullopt; // error

        if (!newline()) return nullopt;

        ptran.commit();
        return ret;
}

//       frmparam   = *(";" other-param)
optional<FrmParam> IcalParser::frmparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        FrmParam ret;

        while (token(";")) {
                if (auto v = other_param()) ret.otherParams.push_back(*v);
                else return nullopt; // error
        }

        ptran.commit();
        return ret;
}

//       toparam    = *(";" other-param)
optional<ToParam> IcalParser::toparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        ToParam ret;

        while (token(";")) {
                if (auto v = other_param()) ret.otherParams.push_back(*v);
                else return nullopt; // error
        }

        ptran.commit();
        return ret;
}

//       time-numzone = ("+" / "-") time-hour time-minute [time-second]
optional<TimeNumZone> IcalParser::time_numzone() {
        CALLSTACK;
        save_input_pos ptran(is);
        TimeNumZone ret;

        if (token("+")) ret.sign = +1;
        else if (token("-")) ret.sign = -1;
        else return nullopt;

        if (auto v = time_hour()) ret.hour = *v;
        else return nullopt; // error

        if (auto v = time_minute()) ret.minute = *v;
        else return nullopt; // error

        if (auto v = time_second()) ret.second = *v;

        ptran.commit();
        return ret;
}

//       utc-offset = time-numzone
optional<UtcOffset> IcalParser::utc_offset() {
        CALLSTACK;
        save_input_pos ptran(is);
        UtcOffset ret;

        if (auto v = time_numzone()) ret.numZone = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       tzoffsetto = "TZOFFSETTO" toparam ":" utc-offset CRLF
optional<TzOffsetTo> IcalParser::tzoffsetto() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzOffsetTo ret;

        if (!token("TZOFFSETTO")) return nullopt;

        if (auto v = toparam()) ret.param = *v;
        else return nullopt; // error

        if (!token(":")) return nullopt; // error

        if (auto v = utc_offset()) ret.utcOffset = *v;
        else return nullopt; // error

        if (!newline()) return nullopt; // error

        ptran.commit();
        return ret;
}

//       tzoffsetfrom       = "TZOFFSETFROM" frmparam ":" utc-offset CRLF
optional<TzOffsetFrom> IcalParser::tzoffsetfrom() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzOffsetFrom ret;

        if (!token("TZOFFSETFROM")) return nullopt;

        if (auto v = frmparam()) ret.param = *v;
        else return nullopt; // error

        if (!token(":")) return nullopt; // error

        if (auto v = utc_offset()) ret.utcOffset = *v;
        else return nullopt; // error

        if (!newline()) return nullopt; // error

        ptran.commit();
        return ret;
}

//       tznparam   = *(
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
optional<TzNParam> IcalParser::tznparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzNParam ret;

        while (true) {
                if (auto v = languageparam()) ret.languageParam = *v;
                else if (auto v = other_param()) ret.otherParams.push_back(*v);
                else break;
        }

        ptran.commit();
        return ret;
}

//       tzname     = "TZNAME" tznparam ":" text CRLF
optional<TzName> IcalParser::tzname() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzName ret;

        if (!token("TZNAME")) return nullopt;

        if (auto v = tznparam()) ret.param = *v;
        else return nullopt; // error

        if (!token(":")) return nullopt; // error

        if (auto v = text()) ret.text = *v;
        else return nullopt; // error

        if (!newline()) return nullopt; // error

        ptran.commit();
        return ret;
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
optional<TzProp> IcalParser::tzprop() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzProp ret;

        while (true) {
                if (auto v = dtstart()) ret.dtStart = *v;
                else if (auto v = tzoffsetto()) ret.offsetTo = *v;
                else if (auto v = tzoffsetfrom()) ret.offsetFrom = *v;
                else if (auto v = rrule()) ret.rRule = *v;
                else if (auto v = comment()) ret.comments.push_back(*v);
                else if (auto v = rdate()) ret.rDates.push_back(*v);
                else if (auto v = tzname()) ret.tzNames.push_back(*v);
                else if (auto v = x_prop()) ret.xProps.push_back(*v);
                else if (auto v = iana_prop()) ret.ianaProps.push_back(*v);
        }

        ptran.commit();
        return ret;
}

//       daylightc  = "BEGIN" ":" "DAYLIGHT" CRLF
//                    tzprop
//                    "END" ":" "DAYLIGHT" CRLF
optional<DaylightC> IcalParser::daylightc() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       standardc  = "BEGIN" ":" "STANDARD" CRLF
//                    tzprop
//                    "END" ":" "STANDARD" CRLF
optional<StandardC> IcalParser::standardc() {
        CALLSTACK;
        save_input_pos ptran(is);
        StandardC ret;

        if (!key_value("BEGIN", "STANDARD")) return nullopt;
        if (!newline()) return nullopt; // error

        if (auto v = tzprop()) ret.tzProp = *v;
        else return nullopt; // error

        if (!key_value("END", "STANDARD")) return nullopt; // error
        if (!newline()) return nullopt; // error

        ptran.commit();
        return ret;
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
optional<TimezoneComp> IcalParser::timezonec() {
        CALLSTACK;
        save_input_pos ptran(is);
        TimezoneComp ret;

        if (!key_value("BEGIN", "VTIMEZONE"))
                return nullopt;

        while (true) {
                if (auto v = tzid()) ret.tzId = *v;
                else if (auto v = last_mod()) ret.lastMod = *v;
                else if (auto v = tzurl()) ret.tzUrl = *v;
                else if (auto v = standardc()) ret.observance = *v;
                else if (auto v = daylightc()) ret.observance = *v;
                else if (auto v = x_prop()) ret.xProps.push_back(*v);
                else if (auto v = iana_prop()) ret.ianaProps.push_back(*v);
                else break;
        }

        if (!key_value("END", "VTIMEZONE"))
                return nullopt;

        ptran.commit();
        return ret;
}

//       iana-comp  = "BEGIN" ":" iana-token CRLF
//                    1*contentline
//                    "END" ":" iana-token CRLF
optional<IanaComp> IcalParser::iana_comp() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                token("BEGIN") &&
                token(":") &&
                iana_token() &&
                newline();
        if (!success_pro)
                return nullopt;

        if (!contentline())
                return nullopt;
        while(contentline()) {
        }

        const auto success_epi =
                token("END") &&
                token(":") &&
                iana_token() &&
                newline();
        if (!success_epi)
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       x-comp     = "BEGIN" ":" x-name CRLF
//                    1*contentline
//                    "END" ":" x-name CRLF
optional<XComp> IcalParser::x_comp() {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto success_pro =
                token("BEGIN") &&
                token(":") &&
                x_name() &&
                newline();
        if (!success_pro)
                return nullopt;

        if (!contentline())
                return nullopt;
        while(contentline()) {
        }

        const auto success_epi =
                token("END") &&
                token(":") &&
                x_name() &&
                newline();
        if (!success_epi)
                return nullopt;

        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       component  = 1*(eventc / todoc / journalc / freebusyc /
//                    timezonec / iana-comp / x-comp)
//
optional<Component> IcalParser::component_single() {
        CALLSTACK;
        save_input_pos ptran(is);
        Component ret;
        if (auto v = eventc()) ret = *v;
        else if (auto v = todoc()) ret = *v;
        else if (auto v = journalc()) ret = *v;
        else if (auto v = freebusyc()) ret = *v;
        else if (auto v = timezonec()) ret = *v;
        else if (auto v = iana_comp()) ret = *v;
        else if (auto v = x_comp()) ret = *v;
        else return optional<Component>();
        ptran.commit();
        return ret;
}
Component IcalParser::expect_component_single() {
        CALLSTACK;
        if (auto v = component_single())
                return *v;
        throw syntax_error(is.tellg());
}
vector<Component> IcalParser::expect_component() {
        CALLSTACK;
        save_input_pos ptran(is);
        vector<Component> ret;
        ret.push_back(expect_component_single());
        while(auto v = component_single()) {
                ret.push_back(*v);
        }
        ptran.commit();
        return ret;
}

// altrepparam = "ALTREP" "=" DQUOTE uri DQUOTE
optional<AltRepParam> IcalParser::altrepparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!token("ALTREP")) return nullopt;
        if (!token("=")) return nullopt;
        const auto v = quoted_string();
        if (!v) return nullopt;
        ptran.commit();
        return {{*v}};
}

// cnparam    = "CN" "=" param-value
optional<CnParam> IcalParser::cnparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!token("CN")) return nullopt;
        if (!token("=")) return nullopt;
        const auto v = param_value();
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
optional<CuTypeParam> IcalParser::cutypeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        if (!token("CUTYPE")) return nullopt;
        if (!token("=")) return nullopt;

        string val;
        if (auto v = token("INDIVIDUAL")) val = *v;
        else if (token("GROUP")) val = *v;
        else if (token("RESOURCE")) val = *v;
        else if (token("ROOM")) val = *v;
        else if (token("UNKNOWN")) val = *v;
        else if (x_name()) val = *v;
        else if (iana_token()) val = *v;
        else return nullopt;

        ptran.commit();
        return {{val}};
}

//      delfromparam       = "DELEGATED-FROM" "=" DQUOTE cal-address DQUOTE
//                           *("," DQUOTE cal-address DQUOTE)
optional<DelFromParam> IcalParser::delfromparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DelFromParam ret;

        if (!token("DELEGATED-FROM")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = quoted_string()) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (token(",")) {
                if (auto v = quoted_string()) {
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
optional<DelToParam> IcalParser::deltoparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DelToParam ret;

        if (!token("DELEGATED-TO")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = quoted_string()) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (token(",")) {
                if (auto v = quoted_string()) {
                        ret.values.push_back(*v);
                } else {
                        return nullopt;
                }
        }
        ptran.commit();
        return ret;
}

//      dirparam   = "DIR" "=" DQUOTE uri DQUOTE
optional<DirParam> IcalParser::dirparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        DirParam ret;

        if (!token("DIR")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = quoted_string()) {
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
optional<EncodingParam> IcalParser::encodingparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        EncodingParam ret;
        if (!token("ENCODING")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("8BIT")) {
                ret.value = *v;
        } else if (auto v = token("BASE64")) {
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
optional<FmtTypeParam> IcalParser::fmttypeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        FmtTypeParam ret;

        if (!token("FMTTYPE")) return nullopt;
        if (!token("=")) return nullopt;

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
optional<FbTypeParam> IcalParser::fbtypeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        FbTypeParam ret;
        if (!token("FBTYPE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("FREE")) {
                ret.value = *v;
        } else if (auto v = token("BUSY")) {
                ret.value = *v;
        } else if (auto v = token("BUSY-UNAVAILABLE")) {
                ret.value = *v;
        } else if (auto v = token("BUSY-TENTATIVE")) {
                ret.value = *v;
        } else if (auto v = x_name()) {
                ret.value = *v;
        } else if (auto v = iana_token()) {
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
optional<LanguageParam> IcalParser::languageparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        LanguageParam ret;
        if (!token("LANGUAGE")) return nullopt;
        if (!token("=")) return nullopt;

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
optional<MemberParam> IcalParser::memberparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        MemberParam ret;

        if (!token("MEMBER")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = cal_address()) {
                ret.values.push_back(*v);
        } else {
                return nullopt;
        }
        while (token(",")) {
                if (auto v = cal_address()) {
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
optional<PartStatJour> IcalParser::partstat_jour() {
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
optional<PartStatTodo> IcalParser::partstat_todo() {
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
optional<PartStatEvent> IcalParser::partstat_event() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       partstatparam    = "PARTSTAT" "="
//                         (partstat-event
//                        / partstat-todo
//                        / partstat-jour)
optional<PartStatParam> IcalParser::partstatparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        PartStatParam ret;

        if (!token("PARTSTAT")) return optional<PartStatParam>();
        if (!token("=")) return optional<PartStatParam>();

        if (auto v = partstat_event()) {
                ret = *v;
        } else if (auto v = partstat_todo()) {
                ret = *v;
        } else if (auto v = partstat_jour()) {
                ret = *v;
        } else {
                return optional<PartStatParam>();
        }
        ptran.commit();
        return ret;
}

//       rangeparam = "RANGE" "=" "THISANDFUTURE"
//       ; To specify the instance specified by the recurrence identifier
//       ; and all subsequent recurrence instances.
optional<RangeParam> IcalParser::rangeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        RangeParam ret;

        if (!token("RANGE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("THISANDFUTURE")) {
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
optional<TrigRelParam> IcalParser::trigrelparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        TrigRelParam ret;

        if (!token("RELATED")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("START")) {
                ret.value = *v;
        } else if (auto v = token("END")) {
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
optional<RoleParam> IcalParser::roleparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        RoleParam ret;

        if (!token("ROLE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("CHAIR")) {
                ret.value = *v;
        } else if (auto v = token("REQ-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = token("OPT-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = token("NON-PARTICIPANT")) {
                ret.value = *v;
        } else if (auto v = x_name()) {
                ret.value = *v;
        } else if (auto v = iana_token()) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       rsvpparam = "RSVP" "=" ("TRUE" / "FALSE")
//       ; Default is FALSE
optional<RsvpParam> IcalParser::rsvpparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        RsvpParam ret;

        if (!token("RSVP")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = token("TRUE")) {
                ret.value = *v;
        } else if (auto v = token("FALSE")) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       sentbyparam        = "SENT-BY" "=" DQUOTE cal-address DQUOTE
optional<SentByParam> IcalParser::sentbyparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        SentByParam ret;

        if (!token("SENT-BY")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = quoted_string()) {
                ret.value = *v;
        } else {
                return nullopt;
        }

        ptran.commit();
        return ret;
}

//       tzidprefix = "/"
optional<TzIdPrefix> IcalParser::tzidprefix() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzIdPrefix ret;

        if (auto v = token("/")) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       tzidparam  = "TZID" "=" [tzidprefix] paramtext
optional<TzIdParam> IcalParser::tzidparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        TzIdParam ret;

        if (!token("TZID")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = tzidprefix()) {
                ret.prefix = *v;
        }

        if (auto v = paramtext()) {
                ret.paramtext = *v;
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
optional<ValueType> IcalParser::valuetype() {
        CALLSTACK;
        if (auto v = token("BINARY")) return {{*v}};
        if (auto v = token("BOOLEAN")) return {{*v}};
        if (auto v = token("CAL-ADDRESS")) return {{*v}};
        if (auto v = token("DATE")) return {{*v}};
        if (auto v = token("DATE-TIME")) return {{*v}};
        if (auto v = token("DURATION")) return {{*v}};
        if (auto v = token("FLOAT")) return {{*v}};
        if (auto v = token("INTEGER")) return {{*v}};
        if (auto v = token("PERIOD")) return {{*v}};
        if (auto v = token("RECUR")) return {{*v}};
        if (auto v = token("TEXT")) return {{*v}};
        if (auto v = token("TIME")) return {{*v}};
        if (auto v = token("URI")) return {{*v}};
        if (auto v = token("UTC-OFFSET")) return {{*v}};
        if (auto v = x_name()) return {{*v}};
        if (auto v = iana_token()) return {{*v}};
        return nullopt;
}

//       valuetypeparam = "VALUE" "=" valuetype
optional<ValueTypeParam> IcalParser::valuetypeparam() {
        CALLSTACK;
        save_input_pos ptran(is);
        ValueTypeParam ret;

        if (!token("VALUE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = valuetype()) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//     iana-param  = iana-token "=" param-value *("," param-value)
//     ; Some other IANA-registered iCalendar parameter.
optional<IanaParam> IcalParser::iana_param() {
        CALLSTACK;
        save_input_pos ptran(is);
        IanaParam ret;

        if (auto v = iana_token()) ret.token = *v;
        else return nullopt;

        if (!token("=")) return nullopt;

        do {
                if (auto v = param_value()) ret.values.push_back(*v);
                else return nullopt;
        } while (token(","));

        ptran.commit();
        return ret;
}

//     x-param     = x-name "=" param-value *("," param-value)
//     ; A non-standard, experimental parameter.
optional<XParam> IcalParser::x_param() {
        CALLSTACK;
        save_input_pos ptran(is);
        XParam ret;

        if (auto v = x_name()) ret.name = *v;
        else return nullopt;

        if (!token("=")) return nullopt;

        do {
                if (auto v = param_value()) ret.values.push_back(*v);
                else return nullopt; // error
        } while (token(","));

        ptran.commit();
        return ret;
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
ICalParameter IcalParser::expect_icalparameter() {
        CALLSTACK;
        if (auto v = icalparameter())
                return *v;
        throw syntax_error(is.tellg());
}
optional<ICalParameter> IcalParser::icalparameter() {
        CALLSTACK;
        save_input_pos ptran(is);
        ICalParameter ret;

        if (auto v = altrepparam()) ret = *v;
        else if (auto v = cnparam()) ret = *v;
        else if (auto v = cutypeparam()) ret = *v;
        else if (auto v = delfromparam()) ret = *v;
        else if (auto v = deltoparam()) ret = *v;
        else if (auto v = dirparam()) ret = *v;
        else if (auto v = encodingparam()) ret = *v;
        else if (auto v = fmttypeparam()) ret = *v;
        else if (auto v = fbtypeparam()) ret = *v;
        else if (auto v = languageparam()) ret = *v;
        else if (auto v = memberparam()) ret = *v;
        else if (auto v = partstatparam()) ret = *v;
        else if (auto v = rangeparam()) ret = *v;
        else if (auto v = trigrelparam()) ret = *v;
        else if (auto v = reltypeparam()) ret = *v;
        else if (auto v = roleparam()) ret = *v;
        else if (auto v = rsvpparam()) ret = *v;
        else if (auto v = sentbyparam()) ret = *v;
        else if (auto v = tzidparam()) ret = *v;
        else if (auto v = valuetypeparam()) ret = *v;
        else if (auto v = other_param()) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}
