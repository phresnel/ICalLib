// RFCs:
// - [RFC 5545](https://tools.ietf.org/html/rfc5545)
// - [RFC 5546](https://tools.ietf.org/html/rfc5546)
// - [RFC 6868](https://tools.ietf.org/html/rfc6868)
// - [RFC 7529](https://tools.ietf.org/html/rfc7529)
// - [RFC 7986](https://tools.ietf.org/html/rfc7986)

#include <istream>
#include <iostream> // TODO: Remove iostream include.
#include <string>


// -- Typedefs. ----------------------------------------------------------------
using string = std::string;


// -- Exceptions. --------------------------------------------------------------
class invalid_ical : public std::runtime_error {
public:
        invalid_ical() : std::runtime_error("Invalid ICalendar file") {}
};

class unexpected_token : public std::runtime_error {
public:
        unexpected_token() : std::runtime_error("Unexpected token") {}
        unexpected_token(std::istream const &is, string const &tok)
                : std::runtime_error("Expected token '" + tok + "' not found")
        {}
};

class key_value_pair_expected : public std::runtime_error {
public:
        key_value_pair_expected()
                : std::runtime_error("Expected key-value-pair") {}

        key_value_pair_expected(std::istream const &is,
                                string const &k,
                                string const &v)
                : std::runtime_error(
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
                return false;
        } catch(...) {
                return true;
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
        } catch (...) {
                return false;
        }
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
        } catch (...) {
                return false;
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
        // TODO: expect_icalbody();
        // TODO: expect_key_value(is, "END", "VCALENDAR");
        ptran.commit();
}

bool read_ical(std::istream &is) {
        try {
                expect_ical(is);
                return true;
        } catch (...) {
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
