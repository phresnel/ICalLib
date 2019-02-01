#include "parser.hh"
#include "parser_helpers.hh"

inline namespace parser_helpers {

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


// -- Parser Helpers. ----------------------------------------------------------
string expect_token(std::istream &is, string const &tok) {
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
        return tok;
}

optional<string> read_token(std::istream &is, string const &tok) {
        CALLSTACK;
        try {
                return expect_token(is, tok);
        } catch(syntax_error &) {
                return nullopt;
        }
}

optional<string> read_eof(std::istream &is) {
        save_input_pos ptran(is);
        const auto c = is.get();
        if (c != EOF)
                return nullopt;
        ptran.commit();
        return string() + (char)c;
}

string expect_newline(std::istream &is) {
        CALLSTACK;
        // Even though RFC 5545 says just "CRLF", we also handle "CR" and "LF".
        if (auto v = read_token(is, "\r\n"))
                return *v;
        if (auto v = read_token(is, "\n"))
                return *v;
        if (auto v = read_token(is, "\r"))
                return *v;
        if (auto v = read_eof(is))
                return *v;
        throw unexpected_token(is.tellg());
}

optional<string> read_newline(std::istream &is) {
        CALLSTACK;
        try {
                return expect_newline(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

optional<string> read_hex(std::istream &is) {
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

string expect_alpha(std::istream &is) {
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

optional<string> read_alpha(std::istream &is) {
        CALLSTACK;
        try {
                return expect_alpha(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}


string expect_digit(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        const auto i = is.get();
        if (i<'0' || i>'9')
                throw syntax_error(is.tellg(), "expected digit");
        ptran.commit();
        return string() + (char)i;
}

optional<string> read_digit(std::istream &is) {
        CALLSTACK;
        try {
                return expect_digit(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

optional<string> read_digits(std::istream &is, int at_least, int at_most) {
        CALLSTACK;
        save_input_pos ptran(is);
        int c = 0;
        string ret;
        optional<string> tmp;
        while ((tmp=read_digit(is)) && (at_most<0 || c<at_most)) {
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

string expect_alnum(std::istream &is) {
        CALLSTACK;
        save_input_pos ptran(is);
        if (auto v = read_alpha(is)) {
                ptran.commit();
                return *v;
        } else if (auto v = read_digit(is)) {
                ptran.commit();
                return *v;
        }
        throw syntax_error(is.tellg(), "expected alpha or digit");
}

optional<string> read_alnum(std::istream &is) {
        CALLSTACK;
        try {
                return expect_alnum(is);
        } catch (syntax_error &) {
                return nullopt;
        }
}

}