#include <iostream>
#include "rfc3629.hh"
#include "rfc3986.hh"
#include "rfc4288.hh"
#include "rfc5234.hh"
#include "rfc5646.hh"
#include "IcalParser.hh"
#include "parser_helpers.hh"
#include "parser_exceptions.hh"

string IcalParser::expect_token(string const &tok) {
        CALLSTACK;
        save_input_pos ptran(*is);
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

result<string> IcalParser::token(string const &tok) {
        CALLSTACK;
        try {
                return expect_token(tok);
        } catch(syntax_error &) {
                return no_match;
        }
}

result<string> IcalParser::eof() {
        save_input_pos ptran(*is);
        const auto c = is.get();
        if (c != EOF)
                return no_match;
        ptran.commit();
        return string() + (char)c;
}

string IcalParser::expect_newline() {
        CALLSTACK;
        // Even though RFC 5545 says just "CRLF", we also handle "CR" and "LF".
        // TODO: Is there a need to unify this with Unfolder::absorb_folds()?

        std::istream::sentry se(*is, true);
        std::streambuf* sb = is->rdbuf();

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
                is->setstate(std::ios::eofbit);
                return "";
        };
        throw unexpected_token(is->tellg());
}

result<string> IcalParser::newline() {
        CALLSTACK;
        try {
                return expect_newline();
        } catch (syntax_error &) {
                return no_match;
        }
}

result<string> IcalParser::hex() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto i = is.get();
        switch(i) {
        default:
        case EOF:
                return no_match;
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
        save_input_pos ptran(*is);
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

result<string> IcalParser::alpha() {
        CALLSTACK;
        try {
                return expect_alpha();
        } catch (syntax_error &) {
                return no_match;
        }
}


string IcalParser::expect_digit() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto i = is.get();
        if (i<'0' || i>'9')
                throw syntax_error(is.tellg(), "expected digit");
        ptran.commit();
        return string() + (char)i;
}

string IcalParser::expect_digit(int min, int max) {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto i = is.get();
        if (i<'0'+min || i>'0'+max)
                throw syntax_error(is.tellg(),
                                   "expected digit in range [" +
                                   std::to_string(min) + ".." +
                                   std::to_string(max) + "]");
        ptran.commit();
        return string() + (char)i;
}

result<string> IcalParser::digit() {
        CALLSTACK;
        try {
                return expect_digit();
        } catch (syntax_error &) {
                return no_match;
        }
}

result<string> IcalParser::digit(int min, int max) {
        CALLSTACK;
        try {
                return expect_digit(min, max);
        } catch (syntax_error &) {
                return no_match;
        }
}

result<string> IcalParser::digits(int at_least, int at_most) {
        CALLSTACK;
        save_input_pos ptran(*is);
        int c = 0;
        string ret;
        while (at_most<0 || c<at_most) {
                const auto tmp = digit();
                if (is_error(tmp)) return tmp;
                if (!is_match(tmp)) break;
                ++c;
                ret += *tmp;
        }

        if (at_least >= 0 && c<at_least)
                return no_match;
        if (at_most >= 0 && c>at_most)
                return no_match;

        ptran.commit();
        return ret;
}

result<string> IcalParser::digits(int num) {
        CALLSTACK;
        return digits(num, num);
}

string IcalParser::expect_alnum() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (auto v = alpha(); is_match(v)) {
                ptran.commit();
                return *v;
        } else if (auto v = digit(); is_match(v)) {
                ptran.commit();
                return *v;
        }
        throw syntax_error(is.tellg(), "expected alpha or digit");
}

result<string> IcalParser::alnum() {
        CALLSTACK;
        try {
                return expect_alnum();
        } catch (syntax_error &) {
                return no_match;
        }
}


tuple<string, string> IcalParser::expect_key_value_newline(
        string const &k,
        string const &v
) {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto success =
                is_match(token(k)) &&
                is_match(token(":")) &&
                is_match(token(v)) &&
                is_match(newline());
        if (!success) {
                throw key_value_pair_expected(is.tellg(), k, v);
        }
        ptran.commit();
        return {k, v};
}

result<tuple<string, string>> IcalParser::key_value_newline(
        string const &k,
        string const &v
) {
        CALLSTACK;
        try {
                return expect_key_value_newline(k, v);
        } catch (syntax_error &) {
                return no_match;
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
result<ContentLine> IcalParser::contentline() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ContentLine ret;

        if (auto v = name(); is_match(v)) ret.name = *v;
        else return no_match;

        while (is_match(token(";"))) {
                if (auto v = param(); is_match(v)) ret.params.push_back(*v);
                else PARSING_ERROR("");
        }
        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = value(); is_match(v)) ret.value = *v;
        else return no_match;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//     name          = iana-token / x-name
result<string> IcalParser::name() {
        CALLSTACK;
        save_input_pos ptran(*is);

        if (auto v = iana_token(); is_match(v))
                return *v;
        if (auto v = x_name(); is_match(v))
                return *v;

        ptran.commit();
        return no_match;
}

//     iana-token    = 1*(ALPHA / DIGIT / "-")
//     ; iCalendar identifier registered with IANA
result<string> IcalParser::iana_token_char() {
        CALLSTACK;
        save_input_pos ptran(*is);

        if (auto v = alnum(); is_match(v)) return *v;
        if (auto v = token("-"); is_match(v)) return *v;

        ptran.commit();
        return no_match;
}
result<string> IcalParser::iana_token() {
        CALLSTACK;
        save_input_pos ptran(*is);

        string ret;
        if (auto v = iana_token_char(); is_match(v)) {
                ret += *v;
        } else {
                return no_match;
        }
        for (auto v = iana_token_char(); is_match(v); v = iana_token_char())
                ret += *v;
        // TODO: IANA iCalendar identifiers

        ptran.commit();
        return ret;
}

//     vendorid      = 3*(ALPHA / DIGIT)
//     ; Vendor identification
result<string> IcalParser::vendorid() {
        CALLSTACK;
        save_input_pos ptran(*is);

        string ret;
        if (auto v = alnum(); is_match(v)) ret += *v;
        else return no_match;

        if (auto v = alnum(); is_match(v)) ret += *v;
        else return no_match;

        if (auto v = alnum(); is_match(v)) ret += *v;
        else return no_match;

        for (auto v = alnum(); is_match(v); v = alnum())
                ret += *v;

        ptran.commit();
        return ret;
}

//     SAFE-CHAR     = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-7E / NON-US-ASCII
//     ; Any character except CONTROL, DQUOTE, ";", ":", ","
result<string> IcalParser::safe_char() {
        CALLSTACK;
        {
                save_input_pos ptran(*is);
                // WSP
                if (auto v = read_wsp(*is)) {
                        ptran.commit();
                        return *v;
                }
                const auto i = is.get();
                switch (i) {
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
        }

        if (auto v = non_us_ascii(); is_match(v))
                return *v;
        return no_match;
}

//     VALUE-CHAR    = WSP / %x21-7E / NON-US-ASCII
//     ; Any textual character
result<string> IcalParser::value_char() {
        CALLSTACK;
        {
                save_input_pos ptran(*is);
                // WSP
                if (auto v = read_wsp(*is)) {
                        ptran.commit();
                        return *v;
                }
                const auto i = is.get();
                switch (i) {
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
        }

        if (auto v = non_us_ascii(); is_match(v))
                return *v;
        return no_match;
}

//     QSAFE-CHAR    = WSP / %x21 / %x23-7E / NON-US-ASCII
//     ; Any character except CONTROL and DQUOTE
result<string> IcalParser::qsafe_char() {
        CALLSTACK;
        {
                save_input_pos ptran(*is);
                // WSP
                if (auto v = read_wsp(*is)) {
                        ptran.commit();
                        return *v;
                }
                const auto i = is.get();
                switch (i) {
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
        }

        if (auto v = non_us_ascii(); is_match(v))
                return *v;
        return no_match;
}

//     NON-US-ASCII  = UTF8-2 / UTF8-3 / UTF8-4
//     ; UTF8-2, UTF8-3, and UTF8-4 are defined in [RFC3629]
result<string> IcalParser::non_us_ascii() {
        CALLSTACK;

        if (auto v = read_utf8_2(*is)) return *v;
        else if (auto v = read_utf8_3(*is)) return *v;
        else if (auto v = read_utf8_4(*is)) return *v;

        return no_match;
}

//     CONTROL       = %x00-08 / %x0A-1F / %x7F
//     ; All the controls except HTAB
result<string> IcalParser::control() {
        CALLSTACK;
        save_input_pos ptran(*is);

        const auto a = is.get();

        const auto A = (a>=0x00) && (a<=0x08);
        const auto B = (a>=0x0A) && (a<=0x1F);
        const auto C = (a==0x7F);

        const auto match = A || B || C;
        if (!match)
                return no_match;

        ptran.commit();
        return string() + char(a);
}

//     value         = *VALUE-CHAR
result<string> IcalParser::value() {
        CALLSTACK;
        string ret;
        for (auto v = value_char(); is_match(v); v = value_char())
                ret += *v;
        return ret;
}


//     param         = param-name "=" param-value *("," param-value)
//     ; Each property defines the specific ABNF for the parameters
//     ; allowed on the property.  Refer to specific properties for
//     ; precise parameter ABNF.
//
result<Param> IcalParser::param() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Param ret;

        if (auto v = param_name(); is_match(v)) ret.name = *v;
        else return no_match;

        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = param_value(); is_match(v)) ret.values.push_back(*v);
        else return PARSING_ERROR("");

        for (auto v = token(","); is_match(v); v = token(",")) {
                if (auto v = param_value(); is_match(v))
                        ret.values.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//     param-name    = iana-token / x-name
result<string> IcalParser::param_name() {
        CALLSTACK;
        if (auto v = iana_token(); is_match(v))
                return *v;
        if (auto v = x_name(); is_match(v))
                return *v;
        throw syntax_error(is.tellg(), "expected param-name");
}


//     param-value   = paramtext / quoted-string
result<string> IcalParser::param_value() {
        CALLSTACK;
        // NOTE: the testing order of paramtext and quoted-string
        //       as per the RFC is errorful, as paramtext can be
        //       validly empty, meaning that when in fact we have a
        //       quoted string, it will never be matched, because paramtext
        //       will successfully return an empty string.
        //       Therefore, we switched it here.
        if (auto v = quoted_string(); is_match(v))  {
                return *v;
        }
        if (auto v = paramtext(); is_match(v)) {
                return *v;
        }
        return no_match;
}

//     paramtext     = *SAFE-CHAR
result<string> IcalParser::paramtext() {
        CALLSTACK;
        string ret;
        for (auto v = safe_char(); is_match(v); v = safe_char())
                ret += *v;
        return ret;
}

//     quoted-string = DQUOTE *QSAFE-CHAR DQUOTE
result<string> IcalParser::quoted_string() {
        CALLSTACK;
        save_input_pos ptran(*is);

        if (!is_match(dquote()))
                return no_match;

        string ret;
        for (auto v = qsafe_char(); is_match(v); v = qsafe_char()) {
                ret += *v;
        }

        if (!is_match(dquote()))
                return PARSING_ERROR("");

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
result<Calendar> IcalParser::icalobject() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Calendar ret;
        if (!is_match(key_value_newline("BEGIN", "VCALENDAR")))
                return no_match;
        if (auto v = icalbody(); is_match(v))
                ret = *v;
        if (!is_match(key_value_newline("END", "VCALENDAR")))
                return PARSING_ERROR("");
        ptran.commit();
        // TODO: The grammar says that there can be more than 1 icalobject.
        return ret;
}

//       icalbody   = calprops component
result<Calendar> IcalParser::icalbody() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Calendar ret;

        std::cerr << "expect_icalbody: parsing calprops ...\n";
        if (auto v = calprops(); is_match(v)) ret.properties = *v;
        else return PARSING_ERROR("");

        std::cerr << "expect_icalbody: parsing components ...\n";
        if (auto v = component(); is_match(v)) ret.components = *v;
        else return PARSING_ERROR("");

        std::cerr << "expect_icalbody: done\n";
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
result<CalProps> IcalParser::calprops() {
        CALLSTACK;
        save_input_pos ptran(*is);
        CalProps ret;
        int prodidc = 0,
            versionc = 0,
            calscalec = 0,
            methodc = 0;

        while (true) {
                if (auto val = prodid(); is_match(val)) {
                        ret.prodId = *val;
                        ++prodidc;
                } else if (auto val = version(); is_match(val)) {
                        // ret.version = ...
                        ++versionc;
                } else if (auto val = calscale(); is_match(val)) {
                        ++calscalec;
                } else if (auto val = method(); is_match(val)) {
                        ++methodc;
                } else if (auto val = x_prop(); is_match(val)) {
                } else if (auto val = iana_prop(); is_match(val)) {
                } else {
                        break;
                }
        }
        ptran.commit();
        return ret;
}

//       prodid     = "PRODID" pidparam ":" pidvalue CRLF
result<ProdId> IcalParser::prodid() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ProdId ret;

        if (!is_match(token("PRODID"))) return no_match;

        if (auto v = pidparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = pidvalue(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       version    = "VERSION" verparam ":" vervalue CRLF
result<Version> IcalParser::version() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Version ret;

        if (!is_match(token("VERSION"))) return no_match;

        if (auto v = verparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = vervalue(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       verparam   = *(";" other-param)
result<std::vector<OtherParam>> IcalParser::verparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        std::vector<OtherParam> ret;
        while(is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
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
result<string> IcalParser::vervalue() {
        CALLSTACK;
        return text();
}

//       calscale   = "CALSCALE" calparam ":" calvalue CRLF
result<CalScale> IcalParser::calscale() {
        CALLSTACK;
        save_input_pos ptran(*is);
        CalScale ret;
        if (!is_match(token("CALSCALE"))) return no_match;

        if (auto v = calparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = calvalue(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       calparam   = *(";" other-param)
result<vector<OtherParam>> IcalParser::calparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        vector<OtherParam> ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       calvalue   = "GREGORIAN"
result<string> IcalParser::calvalue() {
        CALLSTACK;
        if (auto v = token("GREGORIAN"); is_match(v))
                return *v;
        return no_match;
}

//       method     = "METHOD" metparam ":" metvalue CRLF
result<Method> IcalParser::method() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Method ret;

        if (!is_match(token("METHOD"))) return no_match;

        if (auto v = metparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = metvalue(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");
        ptran.commit();
        return ret;
}

//       metparam   = *(";" other-param)
result<std::vector<OtherParam>> IcalParser::metparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        vector<OtherParam> ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       metvalue   = iana-token
result<string> IcalParser::metvalue() {
        CALLSTACK;
        if (auto v = iana_token(); is_match(v)) return *v;
        else return no_match;
}

//       x-prop = x-name *(";" icalparameter) ":" value CRLF
result<XProp> IcalParser::x_prop() {
        CALLSTACK;
        save_input_pos ptran(*is);
        XProp ret;
        if (auto v = x_name(); is_match(v)) ret.name = *v;
        else return no_match;

        while (is_match(token(";"))) {
                if (auto v = icalparameter(); is_match(v))
                        ret.params.push_back(*v);
                else return PARSING_ERROR("");
        }
        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = value(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}


//     x-name        = "X-" [vendorid "-"] 1*(ALPHA / DIGIT / "-")
//     ; Reserved for experimental use.
result<string> IcalParser::x_name() {
        CALLSTACK;
        save_input_pos ptran(*is);

        string ret;

        // "X-"
        if (auto v = token("X-"); is_match(v)) {
                ret += *v;
        } else {
                return no_match;
        }

        // [vendorid "-"]
        if (auto v = vendorid(); is_match(v)) {
                ret += *v;

                if (auto v = token("-"); is_match(v)) ret += *v;
                else PARSING_ERROR("");
        }

        // 1*(ALPHA / DIGIT / "-")
        if (auto v = alnum(); is_match(v)) {
                ret += *v;
        } else if (auto v = token("-"); is_match(v)) {
                ret += *v;
        } else {
                return PARSING_ERROR("");
        }
        while (true) {
                if (auto v = alnum(); is_match(v)) {
                        ret += *v;
                } else if (auto v = token("-"); is_match(v)) {
                        ret += *v;
                } else {
                        break;
                }
        }

        ptran.commit();
        return ret;
}

//       iana-prop = iana-token *(";" icalparameter) ":" value CRLF
result<IanaProp> IcalParser::iana_prop() {
        CALLSTACK;
        save_input_pos ptran(*is);
        IanaProp ret;

        if (auto v = iana_token(); is_match(v)) ret.ianaToken = *v;
        else no_match;

        while (is_match(token(";"))) {
                if (auto v = icalparameter(); is_match(v))
                        ret.params.push_back(*v);
                else return PARSING_ERROR("");
        }

        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = value(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");
        ptran.commit();
}

//       pidparam   = *(";" other-param)
result<std::vector<OtherParam>> IcalParser::pidparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        vector<OtherParam> ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
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
result<string> IcalParser::pidvalue() {
        CALLSTACK;
        return text();
}


//       ESCAPED-CHAR = ("\\" / "\;" / "\," / "\N" / "\n")
//          ; \\ encodes \, \N or \n encodes newline
//          ; \; encodes ;, \, encodes ,
result<string> IcalParser::escaped_char() {
        CALLSTACK;
        if (auto val = token("\\\\"); is_match(val))
                return val;
        if (auto val = token("\\;"); is_match(val))
                return val;
        if (auto val = token("\\,"); is_match(val))
                return val;
        if (auto val = token("\\N"); is_match(val))
                return val;
        if (auto val = token("\\n"); is_match(val))
                return val;
        return no_match;
}

//       TSAFE-CHAR = WSP / %x21 / %x23-2B / %x2D-39 / %x3C-5B /
//                    %x5D-7E / NON-US-ASCII
//          ; Any character except CONTROLs not needed by the current
//          ; character set, DQUOTE, ";", ":", "\", ","
result<string> IcalParser::tsafe_char() {
        CALLSTACK;
        {
                save_input_pos ptran(*is);
                // WSP
                if (auto c = read_wsp(*is)) {
                        ptran.commit();
                        return *c;
                }
                const auto i = is.get();
                switch (i) {
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
                        return string() + (char) i;
                }
        }

        if (auto v = non_us_ascii(); is_match(v))
                return *v;
        return no_match; // error
}


//       text       = *(TSAFE-CHAR / ":" / DQUOTE / ESCAPED-CHAR)
//          ; Folded according to description above
//
result<string> IcalParser::text_char() {
        CALLSTACK;
        if (auto v = tsafe_char(); is_match(v)) return *v;
        if (auto v = token(":"); is_match(v)) return *v;
        if (auto v = dquote(); is_match(v)) return *v;
        if (auto v = escaped_char(); is_match(v)) return *v;
        return no_match;
}

result<string> IcalParser::text() {
        CALLSTACK;
        save_input_pos ptran(*is);
        string ret;
        for (auto c = text_char(); is_match(c); c = text_char()) {
                ret += *c;
        }
        ptran.commit();
        return ret;
}


// DQUOTE: ASCII 22 == '"'
result<string> IcalParser::dquote() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto c = is.get();
        if (c != '"')
                return no_match;
        ptran.commit();
        return "\"";
}

bool IcalParser::binary() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       float      = (["+"] / "-") 1*DIGIT ["." 1*DIGIT]
result<string> IcalParser::float_() {
        CALLSTACK;
        save_input_pos ptran(*is);
        string ret;

        // (["+"] / "-")
        if (auto v = token("+"); is_match(v)) ret += *v;
        else if (auto v = token("-"); is_match(v)) ret += *v;

        // 1*DIGIT
        if (auto v = digit(); is_match(v)) ret += *v;
        else return no_match;

        for (auto v = digit(); is_match(v); v = digit()) {
                ret += *v;
        }

        // ["." 1*DIGIT]
        if (auto v = token("."); is_match(v)) {
                ret += *v;

                if (auto v = digit(); is_match(v)) ret += *v;
                else return no_match;

                for (auto v = digit(); is_match(v); v = digit()) {
                        ret += *v;
                }
        }

        ptran.commit();
        return ret;
}

//       integer    = (["+"] / "-") 1*DIGIT
result<int> IcalParser::integer() {
        CALLSTACK;
        save_input_pos ptran(*is);

        string raw;
        // (["+"] / "-")
        if (auto v = token("+"); is_match(v)) raw += *v;
        else if (auto v = token("-"); is_match(v)) raw += *v;

        // 1*DIGIT
        if (auto v = digit(); is_match(v)) raw += *v;
        else return no_match;

        for (auto v = digit(); is_match(v); v = digit()) {
                raw += *v;
        }
        const auto ret = std::stoi(raw);
        ptran.commit();
        return ret;
}

//       actionvalue = "AUDIO" / "DISPLAY" / "EMAIL" / iana-token / x-name
result<string> IcalParser::actionvalue() {
        CALLSTACK;
        save_input_pos ptran(*is);
        string ret;

        if (auto v = token("AUDIO"); is_match(v)) ret = *v;
        else if (auto v = token("DISPLAY"); is_match(v)) ret = *v;
        else if (auto v = token("EMAIL"); is_match(v)) ret = *v;
        else if (auto v = iana_token(); is_match(v)) ret = *v;
        else if (auto v = x_name(); is_match(v)) ret = *v;
        else return no_match;

        ptran.commit();
        return ret;
}
//       actionparam = *(";" other-param)
result<ActionParam> IcalParser::actionparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ActionParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v))
                        ret.params.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       action      = "ACTION" actionparam ":" actionvalue CRLF
result<Action> IcalParser::action() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Action ret;
        if (!is_match(token("ACTION"))) return no_match;

        if (auto v = actionparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = actionvalue(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

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
result<TrigAbs> IcalParser::trigabs() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TrigAbs ret;

        while (is_match(token(";"))) {
                {
                        save_input_pos ptran(*is);
                        auto match = is_match(token("VALUE")) &&
                                     is_match(token("=")) &&
                                     is_match(token("DATE-TIME"));
                        if (match) {
                                ret.value = "DATE-TIME";
                                ptran.commit();
                                continue;
                        }
                }

                if (auto v = other_param(); is_match(v)) {
                        ret.params.push_back(*v);
                } else {
                        return PARSING_ERROR("");
                }
        }

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = date_time(); is_match(v)) ret.dateTime = *v;
        else return PARSING_ERROR("");

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

result<TrigRel> IcalParser::trigrel() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TrigRel ret;

        while (is_match(token(";"))) {
                {
                        save_input_pos ptran(*is);
                        auto match = is_match(token("VALUE")) &&
                                     is_match(token("=")) &&
                                     is_match(token("DURATION"));
                        if (match) {
                                ret.value = "DURATION";
                                ptran.commit();
                                continue;
                        }
                }

                if (auto v = trigrelparam(); is_match(v)) {
                        ret.trigRelParam = *v;
                } else if (auto v = other_param(); is_match(v)) {
                        ret.params.push_back(*v);
                } else {
                        return PARSING_ERROR("");
                }
        }

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = dur_value(); is_match(v)) ret.durValue = *v;
        else return PARSING_ERROR("");

        ptran.commit();
        return ret;
}
//       trigger    = "TRIGGER" (trigrel / trigabs) CRLF
result<Trigger> IcalParser::trigger() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Trigger ret;

        if (!is_match(token("TRIGGER"))) return no_match;

        if (auto v = trigrel(); is_match(v)) ret = *v;
        else if (auto v = trigabs(); is_match(v)) ret = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       repparam   = *(";" other-param)
result<RepParam> IcalParser::repparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        RepParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v))
                        ret.params.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       repeat  = "REPEAT" repparam ":" integer CRLF  ;Default is "0", zero.
result<Repeat> IcalParser::repeat() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Repeat ret;

        if (!is_match(token("REPEAT"))) return no_match;

        if (auto v = repparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = integer(); is_match(v)) ret.value = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

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
result<AudioProp> IcalParser::audioprop() {
        CALLSTACK;
        save_input_pos ptran(*is);
        AudioProp ret;

        bool req_act = false, req_trig = false;
        while(true) {
                if (auto v = action(); is_match(v)) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = trigger(); is_match(v)) {
                        req_trig = true;
                        ret.trigger = *v;
                }
                else if (auto v = duration(); is_match(v))
                        ret.duration = *v;
                else if (auto v = repeat(); is_match(v))
                        ret.repeat = *v;
                else if (auto v = attach(); is_match(v))
                        ret.attach = *v;
                else if (auto v = x_prop(); is_match(v))
                        ret.xProps.push_back(*v);
                else if (auto v = iana_prop(); is_match(v))
                        ret.ianaProps.push_back(*v);
                else break;
        }
        // std::cerr << "audioprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        if (!(req_act && req_trig)) {
                return PARSING_ERROR("Missing action and/or trigger.");
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
result<DispProp> IcalParser::dispprop() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DispProp ret;

        bool req_act = false, req_desc = false, req_trig = false;
        while(true) {
                if (auto v = action(); is_match(v)) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = description(); is_match(v)) {
                        req_desc = true;
                        ret.description = *v;
                }
                else if (auto v = trigger(); is_match(v)) {
                        req_trig = true;
                        ret.trigger = *v;
                }

                else if (auto v = duration(); is_match(v))
                        ret.duration = *v;
                else if (auto v = repeat(); is_match(v))
                        ret.repeat = *v;

                else if (auto v = x_prop(); is_match(v))
                        ret.xProps.push_back(*v);
                else if (auto v = iana_prop(); is_match(v))
                        ret.ianaProps.push_back(*v);

                else break;
        }

        // std::cerr << "dispprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has desc   : " << (req_desc?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        if (!(req_act && req_desc && req_trig)) {
                 return PARSING_ERROR(
                         "Missing action and/or description and/or trigger");
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
result<EmailProp> IcalParser::emailprop() {
        CALLSTACK;
        save_input_pos ptran(*is);
        EmailProp ret;

        bool req_act = false,
             req_desc = false,
             req_trig = false,
             req_summ = false;
        while(true) {
                if (auto v = action(); is_match(v)) {
                        req_act = true;
                        ret.action = *v;
                }
                else if (auto v = description(); is_match(v)) {
                        req_desc = true;
                        ret.description = *v;
                }
                else if (auto v = trigger(); is_match(v)) {
                        req_trig = true;
                        ret.trigger = *v;
                }
                else if (auto v = summary(); is_match(v)) {
                        req_summ = true;
                        ret.summary = *v;
                }

                else if (auto v = attendee(); is_match(v))
                        ret.attendee = *v;

                else if (auto v = duration(); is_match(v))
                        ret.duration = *v;
                else if (auto v = repeat(); is_match(v))
                        ret.repeat = *v;

                else if (auto v = attach(); is_match(v))
                        ret.attach.push_back(*v);
                else if (auto v = x_prop(); is_match(v))
                        ret.xProps.push_back(*v);
                else if (auto v = iana_prop(); is_match(v))
                        ret.ianaProps.push_back(*v);

                else break;
        }

        // std::cerr << "emailprop:" << std::endl;
        // std::cerr << "  has action : " << (req_act?"yes":"no") << std::endl;
        // std::cerr << "  has desc   : " << (req_desc?"yes":"no") << std::endl;
        // std::cerr << "  has trig   : " << (req_trig?"yes":"no") << std::endl;
        // std::cerr << "  has summ   : " << (req_summ?"yes":"no") << std::endl;
        if (!(req_act && req_desc && req_trig && req_summ)) {
                return PARSING_ERROR("Missing action and/or description "
                                     "and/or trigger and/or summary");
        }

        ptran.commit();
        return ret;
}

//       alarmc     = "BEGIN" ":" "VALARM" CRLF
//                    (audioprop / dispprop / emailprop)
//                    "END" ":" "VALARM" CRLF
result<Alarm> IcalParser::alarmc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Alarm ret;

        if (!is_match(key_value_newline("BEGIN", "VALARM")))
                return no_match;

        // Compared to the grammar, we invert the checking-order
        // because the required fields overlap, so we check for most
        // specialized first.

        if (auto v = emailprop(); is_match(v)) ret = *v;
        else if (auto v = dispprop(); is_match(v)) ret = *v;
        else if (auto v = audioprop(); is_match(v)) ret = *v;
        else return PARSING_ERROR("Must have audioprop, dispprop or emailprop");

        if (!is_match(key_value_newline("END", "VALARM")))
                return PARSING_ERROR("Missing END:VALARM");

        ptran.commit();
        return ret;
}

//       date-fullyear      = 4DIGIT
result<string> IcalParser::date_fullyear() {
        CALLSTACK;
        return digits(4);
}

//       date-month         = 2DIGIT        ;01-12
result<string> IcalParser::date_month() {
        CALLSTACK;
        return digits(2);
}

//       date-mday          = 2DIGIT        ;01-28, 01-29, 01-30, 01-31
//                                          ;based on month/year
result<string> IcalParser::date_mday() {
        CALLSTACK;
        return digits(2);
}

//       date-value         = date-fullyear date-month date-mday
result<Date> IcalParser::date_value() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Date ret;

        if (auto v = date_fullyear(); is_match(v)) ret.year = *v;
        else return no_match;

        if (auto v = date_month(); is_match(v)) ret.month = *v;
        else return no_match;

        if (auto v = date_mday(); is_match(v)) ret.day = *v;
        else return no_match;

        ptran.commit();
        return ret;
}

//       date               = date-value
result<Date> IcalParser::date() {
        CALLSTACK;
        return date_value();
}

//       time-hour    = 2DIGIT        ;00-23
result<TimeHour> IcalParser::time_hour() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TimeHour ret;
        if (auto v = digits(2); is_match(v)) ret.value = *v;
        else return no_match;
        ptran.commit();
        return ret;
}

//       time-minute  = 2DIGIT        ;00-59
result<TimeMinute> IcalParser::time_minute() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TimeMinute ret;
        if (auto v = digits(2); is_match(v)) ret.value = *v;
        else return no_match;
        ptran.commit();
        return ret;
}

//       time-second  = 2DIGIT        ;00-60
//       ;The "60" value is used to account for positive "leap" seconds.
result<TimeSecond> IcalParser::time_second() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TimeSecond ret;
        if (auto v = digits(2); is_match(v)) ret.value = *v;
        else return no_match;
        ptran.commit();
        return ret;
}

//       time-utc     = "Z"
result<string> IcalParser::time_utc() {
        CALLSTACK;
        return token("Z");
}

//       time         = time-hour time-minute time-second [time-utc]
result<Time> IcalParser::time() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Time ret;

        if (auto v = time_hour(); is_match(v)) ret.hour = *v;
        else return no_match;

        if (auto v = time_minute(); is_match(v)) ret.minute = *v;
        else return no_match;

        if (auto v = time_second(); is_match(v)) ret.second = *v;
        else return no_match;

        if (auto v = time_utc(); is_match(v)) ret.utc = *v;

        ptran.commit();
        return ret;
}

//       date-time  = date "T" time ;As specified in the DATE and TIME
//                                  ;value definitions
result<DateTime> IcalParser::date_time() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DateTime ret;

        if (auto v = date(); is_match(v)) ret.date = *v;
        else return no_match;

        if (!is_match(token("T"))) return no_match;

        if (auto v = time(); is_match(v)) ret.time = *v;
        else return no_match;

        ptran.commit();
        return ret;
}

//       dur-week   = 1*DIGIT "W"
result<DurWeek> IcalParser::dur_week() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurWeek ret;

        if (auto v = digits(1, -1); is_match(v)) ret.value = *v;
        else return no_match;

        if (!is_match(token("W"))) return no_match;

        ptran.commit();
        return ret;
}

//       dur-second = 1*DIGIT "S"
result<DurSecond> IcalParser::dur_second() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurSecond ret;

        if (auto v = digits(1, -1); is_match(v)) ret.second = *v;
        else return no_match;

        if (!is_match(token("S"))) return no_match;

        ptran.commit();
        return ret;
}

//       dur-minute = 1*DIGIT "M" [dur-second]
result<DurMinute> IcalParser::dur_minute() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurMinute ret;

        if (auto v = digits(1, -1); is_match(v)) ret.minute = *v;
        else return no_match;

        if (!is_match(token("M"))) return no_match;

        if (auto v = dur_second(); is_match(v)) ret.second = *v;

        ptran.commit();
        return ret;
}

//       dur-hour   = 1*DIGIT "H" [dur-minute]
result<DurHour> IcalParser::dur_hour() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurHour ret;

        if (auto v = digits(1, -1); is_match(v)) ret.hour = *v;
        else return no_match;

        if (!is_match(token("H"))) return no_match;

        if (auto v = dur_minute(); is_match(v)) ret.minute = *v;

        ptran.commit();
        return ret;
}

//       dur-day    = 1*DIGIT "D"
result<DurDay> IcalParser::dur_day() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurDay ret;

        if (auto v = digits(1, -1); is_match(v)) ret.value += *v;
        else return nullopt;

        if (!is_match(token("D"))) return nullopt;

        ptran.commit();
        return ret;
}

//       dur-time   = "T" (dur-hour / dur-minute / dur-second)
result<DurTime> IcalParser::dur_time() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurTime ret;

        if (!is_match(token("T"))) return nullopt;

        if (auto v = dur_hour(); is_match(v)) {
                ret = *v;
        }
        else if (auto v = dur_minute(); is_match(v)) {
                ret = *v;
        }
        else if (auto v = dur_second(); is_match(v)) {
                ret = *v;
        }
        else {
                return no_match;
        }
        ptran.commit();
        return ret;
}

//       dur-date   = dur-day [dur-time]
result<DurDate> IcalParser::dur_date() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurDate ret;

        if (auto v = dur_day(); is_match(v)) ret.day = *v;
        else return no_match;

        if (auto v = dur_time(); is_match(v)) ret.time = *v;

        ptran.commit();
        return ret;
}

//       dur-value  = (["+"] / "-") "P" (dur-date / dur-time / dur-week)
result<DurValue> IcalParser::dur_value() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DurValue ret;

        if(is_match(token("+"))) ret.positive = true;
        else if (is_match(token("-"))) ret.positive = false;

        if (!is_match(token("P"))) return no_match; // error

        if (auto v = dur_date(); is_match(v)) {
                ret.value = *v;
        }
        else if (auto v = dur_time(); is_match(v)) {
                ret.value = *v;
        }
        else if (auto v = dur_week(); is_match(v)) {
                ret.value = *v;
        }
        else return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       period-start = date-time "/" dur-value
//       ; [ISO.8601.2004] complete representation basic format for a
//       ; period of time consisting of a start and positive duration
//       ; of time.
bool IcalParser::period_start() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        const auto match =
                period_explicit() ||
                period_start() ;
        if (!match)
                return false;
        ptran.commit();
        return true;
}

//     other-param   = (iana-param / x-param)
result<OtherParam> IcalParser::other_param() {
        CALLSTACK;
        if (auto v = iana_param(); is_match(v)) return OtherParam{*v};
        if (auto v = x_param(); is_match(v)) return OtherParam{*v};
        return result<OtherParam>();
}

// stmparam   = *(";" other-param)
result<DtStampParams> IcalParser::stmparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtStampParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

// dtstamp    = "DTSTAMP" stmparam ":" date-time CRLF
result<DtStamp> IcalParser::dtstamp() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtStamp ret;

        if (!is_match(token("DTSTAMP"))) return nullopt;

        if (auto v = stmparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");

        if (auto v = date_time(); is_match(v)) ret.date_time = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}
//       uidparam   = *(";" other-param)
result<vector<OtherParam>> IcalParser::uidparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        vector<OtherParam> ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       uid        = "UID" uidparam ":" text CRLF
result<Uid> IcalParser::uid() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Uid ret;

        if (!is_match(token("UID"))) return nullopt;

        if (auto v = uidparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = text(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!newline())  return nullopt;

        ptran.commit();
        return ret;
}
//       dtstval    = date-time / date
result<DtStartVal> IcalParser::dtstval() {
        CALLSTACK;
        if (auto v = date_time(); is_match(v)) return DtStartVal{*v};
        if (auto v = date(); is_match(v)) return DtStartVal{*v};
        return result<DtStartVal>();
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
result<DtStartParams> IcalParser::dtstparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtStartParams ret;

        while (token(";")) {
                if (token("VALUE")) {
                        if (!is_match(token("=")))
                                return nullopt;

                        if (auto v = token("DATE-TIME"); is_match(v)) {
                                ret.value = *v;
                        } else if (auto v = token("DATE"); is_match(v) {
                                ret.value = *v;
                        } else {
                                return nullopt;
                        }
                } else if (auto v = tzidparam(); is_match(v)) {
                        ret.tz_id = *v;
                } else if (auto v = other_param(); is_match(v)) {
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
result<DtStart> IcalParser::dtstart() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtStart ret;

        if (!is_match(token("DTSTART"))) return nullopt;

        if (auto v = dtstparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = dtstval(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       classvalue = "PUBLIC" / "PRIVATE" / "CONFIDENTIAL" / iana-token
//                  / x-name
//       ;Default is PUBLIC
result<string> IcalParser::classvalue() {
        CALLSTACK;
        if (auto v = token("PUBLIC"); is_match(v)) return v;
        if (auto v = token("PRIVATE"); is_match(v)) return v;
        if (auto v = token("CONFIDENTIAL"); is_match(v)) return v;
        if (auto v = iana_token(); is_match(v)) return v;
        if (auto v = x_name(); is_match(v)) return v;
        else return nullopt;
}

//       classparam = *(";" other-param)
result<ClassParams> IcalParser::classparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ClassParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v))
                        ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       class      = "CLASS" classparam ":" classvalue CRLF
result<Class> IcalParser::class_() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Class ret;

        if (!is_match(token("CLASS"))) return nullopt;

        if (auto v = classparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = classvalue(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}
//       creaparam  = *(";" other-param)
result<CreaParam> IcalParser::creaparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        CreaParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       created    = "CREATED" creaparam ":" date-time CRLF
result<Created> IcalParser::created() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Created ret;

        if (!is_match(token("CREATED"))) return nullopt;

        if (auto v = creaparam(); is_match(v)) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = date_time(); is_match(v)) ret.dateTime = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

        ptran.commit();
        return ret;
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
result<DescParams> IcalParser::descparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DescParams ret;
        while(token(";")) {
                if (auto v = altrepparam(); is_match(v)) {
                        ret.alt_rep = *v;
                } else if (auto v = languageparam(); is_match(v)) {
                        ret.language = *v;
                } else if (auto v = other_param(); is_match(v)) {
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
result<Description> IcalParser::description() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Description ret;

        if (!is_match(token("DESCRIPTION"))) return nullopt;

        if (auto v = descparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = text(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       geovalue   = float ";" float
//       ;Latitude and Longitude components
result<GeoValue> IcalParser::geovalue() {
        CALLSTACK;
        save_input_pos ptran(*is);
        GeoValue ret;
        if (auto v = float_(); is_match(v)) ret.latitude = *v;
        else return nullopt;

        if (!is_match(token(";"))) return nullopt;

        if (auto v = float_(); is_match(v)) ret.longitude = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}
//       geoparam   = *(";" other-param)
result<GeoParams> IcalParser::geoparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        GeoParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       geo        = "GEO" geoparam ":" geovalue CRLF
result<Geo> IcalParser::geo() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Geo ret;

        if (!is_match(token("GEO"))) return nullopt;

        if (auto v = geoparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = geovalue(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}
//       lstparam   = *(";" other-param)
result<LstParams> IcalParser::lstparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        LstParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       last-mod   = "LAST-MODIFIED" lstparam ":" date-time CRLF
result<LastMod> IcalParser::last_mod() {
        CALLSTACK;
        save_input_pos ptran(*is);
        LastMod ret;

        if (!is_match(token("LAST-MODIFIED"))) return nullopt;

        if (auto v = lstparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = date_time(); is_match(v)) ret.dateTime = *v;
        return nullopt;

        if (!is_match(newline())) return PARSING_ERROR(""); // error

        ptran.commit();
        return ret;
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
result<LocParams> IcalParser::locparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        LocParams ret;
        while(token(";")) {
                if (auto v = altrepparam(); is_match(v)) {
                        ret.alt_rep = *v;
                } else if (auto v = languageparam(); is_match(v)) {
                        ret.language = *v;
                } else if (auto v = other_param(); is_match(v)) {
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
result<Location> IcalParser::location() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Location ret;
        if (!is_match(token("LOCATION")))
                return nullopt;

        if (auto v = locparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = text(); is_match(v)) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

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
result<OrgParams> IcalParser::orgparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        OrgParams ret;
        while(token(";")) {
                if (auto v = cnparam(); is_match(v)) {
                        ret.cn = *v;
                } else if (auto v = dirparam(); is_match(v)) {
                        ret.dir = *v;
                } else if (auto v = sentbyparam(); is_match(v)) {
                        ret.sentBy = *v;
                } else if (auto v = languageparam(); is_match(v)) {
                        ret.language = *v;
                } else if (auto v = other_param(); is_match(v)) {
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
result<Uri> IcalParser::cal_address() {
        CALLSTACK;
        return uri();
}
//       organizer  = "ORGANIZER" orgparam ":" cal-address CRLF
result<Organizer> IcalParser::organizer() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Organizer ret;

        if (!is_match(token("ORGANIZER")))
                return nullopt;

        if (auto v = orgparam(); is_match(v)) {
                ret.params = *v;
        } else {
                return nullopt;
        }

        if (!is_match(token(":")))
                return nullopt;

        if (auto v = cal_address(); is_match(v)) {
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
        save_input_pos ptran(*is);
        ptran.commit();
        return true;
}
//       prioparam  = *(";" other-param)
bool IcalParser::prioparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        /*
        save_input_pos ptran(*is);
        PrioParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;*/
}
//       priority   = "PRIORITY" prioparam ":" priovalue CRLF
//       ;Default is zero (i.e., undefined).
result<Priority> IcalParser::priority() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<SeqParams> IcalParser::seqparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        SeqParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}
//       seq = "SEQUENCE" seqparam ":" integer CRLF     ; Default is "0"
result<Seq> IcalParser::seq() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Seq ret;

        if (!is_match(token("SEQUENCE"))) return nullopt;

        if (auto v = seqparam()) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = integer()) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}


//       statvalue-event = "TENTATIVE"    ;Indicates event is tentative.
//                       / "CONFIRMED"    ;Indicates event is definite.
//                       / "CANCELLED"    ;Indicates event was cancelled.
//       ;Status values for a "VEVENT"
result<StatvalueEvent> IcalParser::statvalue_event() {
        CALLSTACK;
        save_input_pos ptran(*is);
        StatvalueEvent ret;

        if (auto v = token("TENTATIVE")) ret.value = *v;
        else if (auto v = token("CONFIRMED")) ret.value = *v;
        else if (auto v = token("CANCELLED")) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       statvalue-todo  = "NEEDS-ACTION" ;Indicates to-do needs action.
//                       / "COMPLETED"    ;Indicates to-do completed.
//                       / "IN-PROCESS"   ;Indicates to-do in process of.
//                       / "CANCELLED"    ;Indicates to-do was cancelled.
//       ;Status values for "VTODO".
result<StatvalueTodo> IcalParser::statvalue_todo() {
        CALLSTACK;
        save_input_pos ptran(*is);
        StatvalueTodo ret;

        if (auto v = token("NEEDS-ACTION")) ret.value = *v;
        else if (auto v = token("COMPLETED")) ret.value = *v;
        else if (auto v = token("IN-PROCESS")) ret.value = *v;
        else if (auto v = token("CANCELLED")) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       statvalue-jour  = "DRAFT"        ;Indicates journal is draft.
//                       / "FINAL"        ;Indicates journal is final.
//                       / "CANCELLED"    ;Indicates journal is removed.
//      ;Status values for "VJOURNAL".
result<StatvalueJour> IcalParser::statvalue_jour() {
        CALLSTACK;
        save_input_pos ptran(*is);
        StatvalueJour ret;

        if (auto v = token("DRAFT")) ret.value = *v;
        else if (auto v = token("FINAL")) ret.value = *v;
        else if (auto v = token("CANCELLED")) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       statvalue       = (statvalue-event
//                       /  statvalue-todo
//                       /  statvalue-jour)
result<Statvalue> IcalParser::statvalue() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Statvalue ret;

        if (auto v = statvalue_event()) ret = *v;
        else if (auto v = statvalue_todo()) ret = *v;
        else if (auto v = statvalue_jour()) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       statparam       = *(";" other-param)
result<StatParams> IcalParser::statparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        StatParams ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       status          = "STATUS" statparam ":" statvalue CRLF
result<Status> IcalParser::status() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Status ret;

        if (!is_match(token("STATUS"))) return nullopt;

        if (auto v = statparam()) ret.params = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = statvalue()) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR(""); // error

        ptran.commit();
        return ret;
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
result<SummParams> IcalParser::summparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<Summary> IcalParser::summary() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Summary ret;

        if (!is_match(token("SUMMARY")))  return nullopt;

        if (auto v = summparam()) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = text()) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       transparam = *(";" other-param)
bool IcalParser::transparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        const auto success =
                token("OPAQUE") ||
                token("TRANSPARENT");
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       transp     = "TRANSP" transparam ":" transvalue CRLF
result<Transp> IcalParser::transp() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<Uri> IcalParser::uri() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto uri = rfc3986::read_URI(*is);
        if (!uri)
                return nullopt;
        ptran.commit();
        return uri;
}
//       urlparam   = *(";" other-param)
bool IcalParser::urlparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(*is);
        ptran.commit();
        return true;
}
//       url        = "URL" urlparam ":" uri CRLF
result<Url> IcalParser::url() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        while (ridparam_single()) {
        }
        ptran.commit();
        return true;
}

//       setposday   = yeardaynum
result<SetPosDay> IcalParser::setposday() {
        CALLSTACK;
        save_input_pos ptran(*is);
        SetPosDay ret;

        if (auto v = yeardaynum()) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       bysplist    = ( setposday *("," setposday) )
result<BySpList> IcalParser::bysplist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        BySpList ret;

        do {
                if (auto v = setposday()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       monthnum    = 1*2DIGIT       ;1 to 12
result<MonthNum> IcalParser::monthnum() {
        CALLSTACK;
        save_input_pos ptran(*is);
        MonthNum ret;

        if (auto v = digits(1,2)) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       bymolist    = ( monthnum *("," monthnum) )
result<ByMoList> IcalParser::bymolist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByMoList ret;

        do {
                if (auto v = monthnum()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       weeknum     = [plus / minus] ordwk
result<Weeknum> IcalParser::weeknum() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       bywknolist  = ( weeknum *("," weeknum) )
result<ByWkNoList> IcalParser::bywknolist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByWkNoList ret;

        do {
                if (auto v = weeknum()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       ordyrday    = 1*3DIGIT      ;1 to 366
result<OrdYrDay> IcalParser::ordyrday() {
        CALLSTACK;
        save_input_pos ptran(*is);
        OrdYrDay ret;

        if (auto v = digits(1,3)) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       yeardaynum  = [plus / minus] ordyrday
result<YearDayNum> IcalParser::yeardaynum() {
        CALLSTACK;
        save_input_pos ptran(*is);
        YearDayNum ret;

        if (auto v = plus()) ret.sign = +1;
        else if (auto v = minus()) ret.sign = -1;
        else ret.sign = +1;

        if (auto v = ordyrday()) ret.day = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       byyrdaylist = ( yeardaynum *("," yeardaynum) )
result<ByYrDayList> IcalParser::byyrdaylist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByYrDayList ret;

        do {
                if (auto v = yeardaynum()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       ordmoday    = 1*2DIGIT       ;1 to 31
result<OrdMoDay> IcalParser::ordmoday() {
        CALLSTACK;
        save_input_pos ptran(*is);
        OrdMoDay ret;

        if (auto v = digits(1,2)) ret = *v;
        return nullopt;

        ptran.commit();
        return ret;
}

//       monthdaynum = [plus / minus] ordmoday
result<MonthDayNum> IcalParser::monthdaynum() {
        CALLSTACK;
        save_input_pos ptran(*is);
        MonthDayNum ret;

        if (auto v = plus()) ret.sign = +1;
        else if (auto v = minus()) ret.sign = -1;
        else ret.sign = +1;

        if (auto v = ordmoday()) ret.day = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       bymodaylist = ( monthdaynum *("," monthdaynum) )
result<ByMoDayList> IcalParser::bymodaylist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByMoDayList ret;

        do {
                if (auto v = monthdaynum()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       weekday     = "SU" / "MO" / "TU" / "WE" / "TH" / "FR" / "SA"
//       ;Corresponding to SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY,
//       ;FRIDAY, and SATURDAY days of the week.
result<WeekDay> IcalParser::weekday() {
        CALLSTACK;
        save_input_pos ptran(*is);
        WeekDay ret;

        if (token("SU")) ret = WeekDay::Sunday;
        else if (token("MO")) ret = WeekDay::Monday;
        else if (token("TU")) ret = WeekDay::Tuesday;
        else if (token("WE")) ret = WeekDay::Wednesday;
        else if (token("TH")) ret = WeekDay::Thursday;
        else if (token("FR")) ret = WeekDay::Friday;
        else if (token("SA")) ret = WeekDay::Saturday;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       ordwk       = 1*2DIGIT       ;1 to 53
result<OrdWk> IcalParser::ordwk() {
        CALLSTACK;
        save_input_pos ptran(*is);
        OrdWk ret;
        if (auto v = digits(1,2)) ret = *v;
        else return nullopt;
        ptran.commit();
        return ret;
}

//       minus       = "-"
bool IcalParser::minus() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (!is_match(token("-"))) return false;
        ptran.commit();
        return true;
}

//       plus        = "+"
bool IcalParser::plus() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (!is_match(token("+"))) return false;
        ptran.commit();
        return true;
}

//       weekdaynum  = [[plus / minus] ordwk] weekday
result<WeekDayNum> IcalParser::weekdaynum() {
        CALLSTACK;
        save_input_pos ptran(*is);
        WeekDayNum ret;

        const int sign = plus() ? +1 : minus() ? -1 : 0;
        if (sign) {
                if (auto v = ordwk()) {
                        ret.week = SignedOrdWk();
                        ret.week->sign = sign;
                        ret.week->ordWk = *v;
                } else {
                        return nullopt; // error
                }
        } else if (auto v = ordwk()) {
                ret.week = SignedOrdWk();
                ret.week->sign = +1;
                ret.week->ordWk = *v;
        }

        if (auto v = weekday()) ret.weekDay = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       bywdaylist  = ( weekdaynum *("," weekdaynum) )
result<ByWDayList> IcalParser::bywdaylist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByWDayList ret;

        do {
                if (auto v = weekdaynum()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       seconds     = 1*2DIGIT       ;0 to 60
result<Seconds> IcalParser::seconds() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Seconds ret;

        if (auto v = digits(1,2)) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       hour        = 1*2DIGIT       ;0 to 23
result<Hour> IcalParser::hour() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Hour ret;

        if (auto v = digits(1,2)) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       minutes     = 1*2DIGIT       ;0 to 59
result<Minutes> IcalParser::minutes() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Hour ret;

        if (auto v = digits(1,2)) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       byhrlist    = ( hour *("," hour) )
result<ByHrList> IcalParser::byhrlist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByHrList ret;

        do {
                if (auto v = hour()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       byminlist   = ( minutes *("," minutes) )
result<ByMinList> IcalParser::byminlist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ByMinList ret;

        do {
                if (auto v = minutes()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       byseclist   = ( seconds *("," seconds) )
result<BySecList> IcalParser::byseclist() {
        CALLSTACK;
        save_input_pos ptran(*is);
        BySecList ret;

        do {
                if (auto v = seconds()) ret.push_back(*v);
                else break;
        } while (token(","));

        if (ret.empty()) return nullopt;

        ptran.commit();
        return ret;
}

//       enddate     = date / date-time
result<EndDate> IcalParser::enddate() {
        CALLSTACK;
        save_input_pos ptran(*is);
        EndDate ret;

        if (auto v = date()) ret = *v;
        else if (auto v = date_time()) ret = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       freq        = "SECONDLY" / "MINUTELY" / "HOURLY" / "DAILY"
//                   / "WEEKLY" / "MONTHLY" / "YEARLY"
result<Freq> IcalParser::freq() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Freq ret;

        if (token("SECONDLY")) ret = Freq::Secondly;
        else if (token("MINUTELY")) ret = Freq::Minutely;
        else if (token("HOURLY")) ret = Freq::Hourly;
        else if (token("DAILY")) ret = Freq::Daily;
        else if (token("WEEKLY")) ret = Freq::Weekly;
        else if (token("MONTHLY")) ret = Freq::Monthly;
        else if (token("YEARLY")) ret = Freq::Yearly;
        else return nullopt;

        ptran.commit();
        return ret;
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
//
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
result<Recur> IcalParser::recur() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Recur ret;

        do {
                if (is_match(token("FREQ"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = freq(); is_match(v)) ret.freq = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("UNTIL"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = enddate(); is_match(v)) ret.duration = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("COUNT"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = digits(1,-1); is_match(v)) ret.duration = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("INTERVAL"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = digits(1,-1); is_match(v)) ret.interval = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYSECOND"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = byseclist(); is_match(v)) ret.bySecond = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYMINUTE"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = byminlist(); is_match(v)) ret.byMinute = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYHOUR"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = byhrlist(); is_match(v)) ret.byHour = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYDAY"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = bywdaylist(); is_match(v)) ret.byDay = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYMONTHDAY"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = bymodaylist(); is_match(v)) ret.byMonthDay = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYYEARDAY"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = byyrdaylist(); is_match(v)) ret.byYearDay = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYWEEKNO"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = bywknolist(); is_match(v)) ret.byweekNo = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYMONTH"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = bymolist(); is_match(v)) ret.byMonth = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("BYSETPOS"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = bysplist(); is_match(v)) ret.bySetpos = *v;
                        else return PARSING_ERROR("");
                } else if (is_match(token("WKST"))) {
                        if (!is_match(token("="))) return PARSING_ERROR("");
                        if (auto v = weekday(); is_match(v)) ret.wkst = *v;
                        else return PARSING_ERROR("");
                } else {
                        return PARSING_ERROR("");
                }
        } while (token(";"));

        ptran.commit();
        return ret;
}

//       recurid    = "RECURRENCE-ID" ridparam ":" ridval CRLF
result<RecurId> IcalParser::recurid() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<RRulParam> IcalParser::rrulparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        RRulParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       rrule      = "RRULE" rrulparam ":" recur CRLF
result<RRule> IcalParser::rrule() {
        CALLSTACK;
        save_input_pos ptran(*is);
        RRule ret;

        if (!is_match(token("RRULE"))) return nullopt;

        if (auto v = rrulparam()) ret.param = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = recur()) ret.recur = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

        ptran.commit();
        return ret;
}

//       dtendval   = date-time / date
//       ;Value MUST match value type
result<DtEndVal> IcalParser::dtendval() {
        CALLSTACK;
        if (auto v = date_time()) return DtEndVal{*v};
        if (auto v = date()) return DtEndVal{*v};
        return result<DtStartVal>();
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
result<DtEndParams> IcalParser::dtendparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtEndParams ret;

        while (token(";")) {
                if (token("VALUE")) {
                        if (!is_match(token("=")))
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
result<DtEnd> IcalParser::dtend() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DtEnd ret;

        if (!is_match(token("DTEND"))) return nullopt;

        if (auto v = dtendparam()) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = dtendval()) ret.value = *v;
        else return nullopt;

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       durparam   = *(";" other-param)
bool IcalParser::durparam() {
        CALLSTACK;
        NOT_IMPLEMENTED;
        save_input_pos ptran(*is);
        const auto success =
                false;
        if (!success)
                return false;
        ptran.commit();
        return true;
}

//       duration   = "DURATION" durparam ":" dur-value CRLF
//                    ;consisting of a positive duration of time.
result<Duration> IcalParser::duration() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
result<Attach> IcalParser::attach() {
        CALLSTACK;
        save_input_pos ptran(*is);

        const auto match_head =
                token("ATTACH") && attachparam();
        if (!match_head)
                return nullopt;

        // ":" uri
        {
                save_input_pos ptran_uri(*is);
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
                save_input_pos ptran_enc(*is);
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
        save_input_pos ptran(*is);
        ptran.commit();
        return true;
}
//       attendee   = "ATTENDEE" attparam ":" cal-address CRLF
result<Attendee> IcalParser::attendee() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<CatParams> IcalParser::catparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<Categories> IcalParser::categories () {
        CALLSTACK;
        save_input_pos ptran(*is);
        Categories ret;

        if (!is_match(token("CATEGORIES"))) return nullopt;

        if (auto v = catparam()) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");;

        if (auto v = text()) ret.values.push_back(*v);
        else return nullopt;

        while (token(",")) {
                if (auto v = text()) ret.values.push_back(*v);
                else return nullopt;
        }

        if (!is_match(newline())) return PARSING_ERROR("");

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
        save_input_pos ptran(*is);
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
result<Comment> IcalParser::comment() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
result<Contact> IcalParser::contact() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        ptran.commit();
        return true;
}
//       exdate     = "EXDATE" exdtparam ":" exdtval *("," exdtval) CRLF
result<ExDate> IcalParser::exdate() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        if (!text())
                return false;
        ptran.commit();
        return true;
}

//       statdesc   = text
//       ;Textual status description
bool IcalParser::statdesc() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
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
result<RStatus> IcalParser::rstatus() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<RelTypeParam> IcalParser::reltypeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        RelTypeParam ret;

        if (!is_match(token("RELTYPE"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = token("PARENT"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("CHILD"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("SIBLING"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = iana_token(); is_match(v)) {
                ret.value = *v;
        } else if (auto v = x_name(); is_match(v)) {
                ret.value = *v;
        } else {
                return PARSING_ERROR("");
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
        save_input_pos ptran(*is);
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
result<Related> IcalParser::related() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        ptran.commit();
        return true;
}
//       resources  = "RESOURCES" resrcparam ":" text *("," text) CRLF
result<Resources> IcalParser::resources() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
                save_input_pos ptran(*is);
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
        save_input_pos ptran(*is);
        while (rdtparam_single()) {
        }
        ptran.commit();
        return true;
}
//       rdate      = "RDATE" rdtparam ":" rdtval *("," rdtval) CRLF
result<RDate> IcalParser::rdate() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<EventProp> IcalParser::eventprop_single() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<vector<EventProp>> IcalParser::eventprop() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<EventComp> IcalParser::eventc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        EventComp ret;

        if (!key_value_newline("BEGIN", "VEVENT")) return nullopt;

        if (auto v = eventprop()) ret.properties = *v;
        else return nullopt;

        while(auto v = alarmc()) {
                ret.alarms.push_back(*v);
        }

        if (!key_value_newline("END", "VEVENT")) return nullopt;

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
result<TodoComp> IcalParser::todoc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto success_pro =
                key_value_newline("BEGIN", "VTODO") &&
                todoprop();
        if (!success_pro)
                return nullopt;

        while(alarmc()) {
        }

        const auto success_epi =
                key_value_newline("END", "VTODO") ;
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
result<JournalComp> IcalParser::journalc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto success =
                key_value_newline("BEGIN", "VJOURNAL") &&
                jourprop() &&
                        key_value_newline("END", "VJOURNAL") ;
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
result<FreeBusyComp> IcalParser::freebusyc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        const auto success =
                key_value_newline("BEGIN", "VFREEBUSY") &&
                fbprop() &&
                        key_value_newline("END", "VFREEBUSY") ;
        if (!success)
                return nullopt;
        ptran.commit();
        NOT_IMPLEMENTED;
        //return true;
}

//       tzidpropparam      = *(";" other-param)
result<TzIdPropParam> IcalParser::tzidpropparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzIdPropParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       tzid       = "TZID" tzidpropparam ":" [tzidprefix] text CRLF
result<TzId> IcalParser::tzid() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzId ret;

        if (!is_match(token("TZID"))) return nullopt;

        if (auto v = tzidpropparam(); is_match(v)) ret.propParams = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = tzidprefix()) ret.prefix = *v;

        if (auto v = text()) ret.text = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       tzurlparam = *(";" other-param)
result<TzUrlParam> IcalParser::tzurlparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzUrlParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       tzurl      = "TZURL" tzurlparam ":" uri CRLF
result<TzUrl> IcalParser::tzurl() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzUrl ret;

        if (!is_match(token("TZURL"))) return nullopt;

        if (auto v = tzurlparam(); is_match(v)) ret.params = *v;
        else return nullopt;

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = uri(); is_match(v)) ret.uri = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR("");

        ptran.commit();
        return ret;
}

//       frmparam   = *(";" other-param)
result<FrmParam> IcalParser::frmparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        FrmParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       toparam    = *(";" other-param)
result<ToParam> IcalParser::toparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        ToParam ret;
        while (is_match(token(";"))) {
                if (auto v = other_param(); is_match(v)) ret.push_back(*v);
                else return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       time-numzone = ("+" / "-") time-hour time-minute [time-second]
result<TimeNumZone> IcalParser::time_numzone() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TimeNumZone ret;

        if (token("+")) ret.sign = +1;
        else if (token("-")) ret.sign = -1;
        else return nullopt;

        if (auto v = time_hour()) ret.hour = *v;
        else return PARSING_ERROR("");

        if (auto v = time_minute()) ret.minute = *v;
        else return PARSING_ERROR("");

        if (auto v = time_second()) ret.second = *v;

        ptran.commit();
        return ret;
}

//       utc-offset = time-numzone
result<UtcOffset> IcalParser::utc_offset() {
        CALLSTACK;
        save_input_pos ptran(*is);
        UtcOffset ret;

        if (auto v = time_numzone()) ret.numZone = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       tzoffsetto = "TZOFFSETTO" toparam ":" utc-offset CRLF
result<TzOffsetTo> IcalParser::tzoffsetto() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzOffsetTo ret;

        if (!is_match(token("TZOFFSETTO"))) return nullopt;

        if (auto v = toparam()) ret.param = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = utc_offset()) ret.utcOffset = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

        ptran.commit();
        return ret;
}

//       tzoffsetfrom       = "TZOFFSETFROM" frmparam ":" utc-offset CRLF
result<TzOffsetFrom> IcalParser::tzoffsetfrom() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzOffsetFrom ret;

        if (!is_match(token("TZOFFSETFROM"))) return nullopt;

        if (auto v = frmparam()) ret.param = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = utc_offset()) ret.utcOffset = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

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
result<TzNParam> IcalParser::tznparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<TzName> IcalParser::tzname() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzName ret;

        if (!is_match(token("TZNAME"))) return nullopt;

        if (auto v = tznparam()) ret.param = *v;
        else return PARSING_ERROR("");

        if (!is_match(token(":"))) return PARSING_ERROR("");; // error

        if (auto v = text()) ret.text = *v;
        else return PARSING_ERROR("");

        if (!is_match(newline())) return PARSING_ERROR(""); // error

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
result<TzProp> IcalParser::tzprop() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
                else break;
        }

        ptran.commit();
        return ret;
}

//       daylightc  = "BEGIN" ":" "DAYLIGHT" CRLF
//                    tzprop
//                    "END" ":" "DAYLIGHT" CRLF
result<DaylightC> IcalParser::daylightc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DaylightC ret;

        if (!key_value_newline("BEGIN", "DAYLIGHT")) return nullopt;

        if (auto v = tzprop()) ret.tzProp = *v;
        else return PARSING_ERROR("");

        if (!key_value_newline("END", "DAYLIGHT")) return nullopt; // error

        ptran.commit();
        return ret;
}

//       standardc  = "BEGIN" ":" "STANDARD" CRLF
//                    tzprop
//                    "END" ":" "STANDARD" CRLF
result<StandardC> IcalParser::standardc() {
        CALLSTACK;
        save_input_pos ptran(*is);
        StandardC ret;

        if (!key_value_newline("BEGIN", "STANDARD")) return nullopt;

        if (auto v = tzprop()) ret.tzProp = *v;
        else return PARSING_ERROR("");

        if (!key_value_newline("END", "STANDARD")) return nullopt; // error

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
result<TimezoneComp> IcalParser::timezonec() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TimezoneComp ret;

        if (!key_value_newline("BEGIN", "VTIMEZONE")) return nullopt;

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

        if (!key_value_newline("END", "VTIMEZONE")) {
                return nullopt; // error
        }

        ptran.commit();
        return ret;
}

//       iana-comp  = "BEGIN" ":" iana-token CRLF
//                    1*contentline
//                    "END" ":" iana-token CRLF
result<IanaComp> IcalParser::iana_comp() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<XComp> IcalParser::x_comp() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<Component> IcalParser::component_single() {
        CALLSTACK;
        save_input_pos ptran(*is);
        Component ret;
        if (auto v = eventc()) ret = *v;
        else if (auto v = todoc()) ret = *v;
        else if (auto v = journalc()) ret = *v;
        else if (auto v = freebusyc()) ret = *v;
        else if (auto v = timezonec()) ret = *v;
        else if (auto v = iana_comp()) ret = *v;
        else if (auto v = x_comp()) ret = *v;
        else return no_match;
        ptran.commit();
        return ret;
}
result<vector<Component>> IcalParser::component() {
        CALLSTACK;
        save_input_pos ptran(*is);
        vector<Component> ret;
        for (auto v = component_single(); is_match(v); v = component_single()) {
                ret.push_back(*v);
        }
        if (ret.empty())
                return no_match;
        ptran.commit();
        return ret;
}

// altrepparam = "ALTREP" "=" DQUOTE uri DQUOTE
result<AltRepParam> IcalParser::altrepparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (!is_match(token("ALTREP"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");
        const auto v = quoted_string();
        if (!is_match(v)) return PARSING_ERROR("");
        ptran.commit();
        return {{*v}};
}

// cnparam    = "CN" "=" param-value
result<CnParam> IcalParser::cnparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (!is_match(token("CN"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");
        const auto v = param_value();
        if (!is_match(v)) return nullopt;
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
result<CuTypeParam> IcalParser::cutypeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        if (!is_match(token("CUTYPE"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        string val;
        if (auto v = token("INDIVIDUAL"); is_match(v)) val = *v;
        else if (auto v = token("GROUP"); is_match(v)) val = *v;
        else if (auto v = token("RESOURCE"); is_match(v)) val = *v;
        else if (auto v = token("ROOM"); is_match(v)) val = *v;
        else if (auto v = token("UNKNOWN"); is_match(v)) val = *v;
        else if (auto v = x_name(); is_match(v)) val = *v;
        else if (auto v = iana_token(); is_match(v)) val = *v;
        else return PARSING_ERROR("");

        ptran.commit();
        return {{val}};
}

//      delfromparam       = "DELEGATED-FROM" "=" DQUOTE cal-address DQUOTE
//                           *("," DQUOTE cal-address DQUOTE)
result<DelFromParam> IcalParser::delfromparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DelFromParam ret;

        if (!is_match(token("DELEGATED-FROM"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = quoted_string(); is_match(v)) {
                ret.values.push_back(*v);
        } else {
                return PARSING_ERROR("");
        }
        while (is_match(token(","))) {
                if (auto v = quoted_string(); is_match(v)) {
                        ret.values.push_back(*v);
                } else {
                        return PARSING_ERROR("");
                }
        }
        ptran.commit();
        return ret;
}

//      deltoparam = "DELEGATED-TO" "=" DQUOTE cal-address DQUOTE
//                    *("," DQUOTE cal-address DQUOTE)
result<DelToParam> IcalParser::deltoparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DelToParam ret;

        if (!is_match(token("DELEGATED-TO"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = quoted_string(); is_match(v)) {
                ret.values.push_back(*v);
        } else {
                return PARSING_ERROR("");
        }
        while (is_match(token(","))) {
                if (auto v = quoted_string(); is_match(v)) {
                        ret.values.push_back(*v);
                } else {
                        return PARSING_ERROR("");
                }
        }
        ptran.commit();
        return ret;
}

//      dirparam   = "DIR" "=" DQUOTE uri DQUOTE
result<DirParam> IcalParser::dirparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        DirParam ret;

        if (!is_match(token("DIR"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = quoted_string(); is_match(v)) {
                ret.value = *v;
        } else {
                return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

// encodingparam =
//          "ENCODING" "="
//          ( "8BIT"   ; "8bit" text encoding is defined in [RFC2045]
//          / "BASE64" ; "BASE64" binary encoding format is defined in [RFC4648]
//          )
result<EncodingParam> IcalParser::encodingparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        EncodingParam ret;
        if (!is_match(token("ENCODING"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = token("8BIT"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("BASE64"); is_match(v)) {
                ret.value = *v;
        } else {
                return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       fmttypeparam = "FMTTYPE" "=" type-name "/" subtype-name
//                      ; Where "type-name" and "subtype-name" are
//                      ; defined in Section 4.2 of [RFC4288].
result<FmtTypeParam> IcalParser::fmttypeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        FmtTypeParam ret;

        if (!is_match(token("FMTTYPE"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = read_type_name(*is); is_match(v)) {
                ret.value = *v;
        } else if (auto v = read_subtype_name(*is); is_match(v)) {
                ret.value = *v;
        } else {
                return PARSING_ERROR("");
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
result<FbTypeParam> IcalParser::fbtypeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        FbTypeParam ret;
        if (!is_match(token("FBTYPE"))) return no_match;
        if (!is_match(token("="))) return PARSING_ERROR("");

        if (auto v = token("FREE"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("BUSY"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("BUSY-UNAVAILABLE"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = token("BUSY-TENTATIVE"); is_match(v)) {
                ret.value = *v;
        } else if (auto v = x_name(); is_match(v)) {
                ret.value = *v;
        } else if (auto v = iana_token(); is_match(v)) {
                ret.value = *v;
        } else {
                return PARSING_ERROR("");
        }
        ptran.commit();
        return ret;
}

//       languageparam = "LANGUAGE" "=" language
//
//       language = Language-Tag
//                  ; As defined in [RFC5646].
result<LanguageParam> IcalParser::languageparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        LanguageParam ret;
        if (!token("LANGUAGE")) return nullopt;
        if (!token("=")) return nullopt;

        if (auto v = read_language_tag(*is)) {
                ret.value = *v;
        } else {
                return nullopt;
        }
        ptran.commit();
        return ret;
}

//       memberparam        = "MEMBER" "=" DQUOTE cal-address DQUOTE
//                            *("," DQUOTE cal-address DQUOTE)
result<MemberParam> IcalParser::memberparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<PartStatJour> IcalParser::partstat_jour() {
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
result<PartStatTodo> IcalParser::partstat_todo() {
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
result<PartStatEvent> IcalParser::partstat_event() {
        CALLSTACK;
        NOT_IMPLEMENTED;
}

//       partstatparam    = "PARTSTAT" "="
//                         (partstat-event
//                        / partstat-todo
//                        / partstat-jour)
result<PartStatParam> IcalParser::partstatparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
        PartStatParam ret;

        if (!token("PARTSTAT")) return result<PartStatParam>();
        if (!token("=")) return result<PartStatParam>();

        if (auto v = partstat_event()) {
                ret = *v;
        } else if (auto v = partstat_todo()) {
                ret = *v;
        } else if (auto v = partstat_jour()) {
                ret = *v;
        } else {
                return result<PartStatParam>();
        }
        ptran.commit();
        return ret;
}

//       rangeparam = "RANGE" "=" "THISANDFUTURE"
//       ; To specify the instance specified by the recurrence identifier
//       ; and all subsequent recurrence instances.
result<RangeParam> IcalParser::rangeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<TrigRelParam> IcalParser::trigrelparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<RoleParam> IcalParser::roleparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<RsvpParam> IcalParser::rsvpparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<SentByParam> IcalParser::sentbyparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<TzIdPrefix> IcalParser::tzidprefix() {
        CALLSTACK;
        save_input_pos ptran(*is);
        TzIdPrefix ret;

        if (auto v = token("/")) ret.value = *v;
        else return nullopt;

        ptran.commit();
        return ret;
}

//       tzidparam  = "TZID" "=" [tzidprefix] paramtext
result<TzIdParam> IcalParser::tzidparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<ValueType> IcalParser::valuetype() {
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
result<ValueTypeParam> IcalParser::valuetypeparam() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<IanaParam> IcalParser::iana_param() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
result<XParam> IcalParser::x_param() {
        CALLSTACK;
        save_input_pos ptran(*is);
        XParam ret;

        if (auto v = x_name()) ret.name = *v;
        else return nullopt;

        if (!token("=")) return nullopt;

        do {
                if (auto v = param_value()) ret.values.push_back(*v);
                else return PARSING_ERROR("");
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
result<ICalParameter> IcalParser::icalparameter() {
        CALLSTACK;
        save_input_pos ptran(*is);
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
