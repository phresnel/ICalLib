#ifndef PARSER_EXCEPTIONS_HH_INCLUDED_20190220
#define PARSER_EXCEPTIONS_HH_INCLUDED_20190220

#include <istream>

// -- Exceptions. --------------------------------------------------------------
class invalid_ical : public std::runtime_error {
public:
        invalid_ical() : std::runtime_error("Invalid ICalendar file") {}
};

class syntax_error : public std::runtime_error {
public:
        syntax_error(std::istream::pos_type pos) :
                std::runtime_error("Syntax error."),
                pos(pos)
        {}

        syntax_error(std::istream::pos_type pos, std::string const &msg) :
                std::runtime_error(msg),
                pos(pos)
        {}

        std::istream::pos_type pos = 0;
};

class unexpected_token : public syntax_error {
public:
        unexpected_token(std::istream::pos_type pos) : syntax_error(pos) {}

        unexpected_token(std::istream::pos_type pos, string const &tok) :
                syntax_error(pos, "Expected token '" + tok + "' not found")
        {}
};

class key_value_pair_expected : public syntax_error {
public:
        key_value_pair_expected(std::istream::pos_type pos) :
                syntax_error(pos, "Expected key-value-pair")
        {}

        key_value_pair_expected(std::istream::pos_type pos,
                                string const &k,
                                string const &v)
                : syntax_error(pos,
                               "Expected key-value-pair '" + k + ":" + v + "'")
        {}
};

#endif //PARSER_EXCEPTIONS_HH_INCLUDED_20190220
