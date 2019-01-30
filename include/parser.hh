#ifndef PARSER_HH_INCLUDED_20190130
#define PARSER_HH_INCLUDED_20190130

// RFCs:
// - Uniform Resource Identifier (URI): Generic Syntax:
//   - [RFC 3986](https://tools.ietf.org/html/rfc3986)
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

#include "ical.hh"
#include <istream>

// -- Typedefs. ----------------------------------------------------------------
using string = std::string;


// -- Forward declarations. ----------------------------------------------------
bool read_altrepparam(std::istream &is);
bool read_attach(std::istream &is);
bool read_attendee(std::istream &);
bool read_calscale(std::istream &);
bool read_control(std::istream &);
bool read_description(std::istream &);
bool read_dquote(std::istream &is);
bool read_dur_value(std::istream &is);
bool read_duration(std::istream &is);
bool read_fmttypeparam(std::istream &is);
bool read_iana_param(std::istream &);
bool read_iana_prop(std::istream &);
bool read_iana_token(std::istream &);
bool read_icalparameter(std::istream &);
bool read_languageparam(std::istream &is);
bool read_method(std::istream &);
bool read_newline(std::istream &);
bool read_non_us_ascii(std::istream &);
bool read_other_param(std::istream &);
bool read_param_value(std::istream &);
bool read_paramtext(std::istream &);
bool read_prodid(std::istream &);
bool read_quoted_string(std::istream &);
bool read_qvalue_char(std::istream &);
bool read_rangeparam(std::istream &is);
bool read_safe_char(std::istream &);
bool read_summary(std::istream &);
bool read_text(std::istream &);
bool read_trigrelparam(std::istream &is);
bool read_tzidparam(std::istream &is);
bool read_value_char(std::istream &);
bool read_vendorid(std::istream &);
bool read_version(std::istream &);
bool read_x_name(std::istream &);
bool read_x_param(std::istream &);
bool read_x_prop(std::istream &);
void expect_calparam(std::istream &);
CalProps expect_calprops(std::istream &is);
void expect_calscale(std::istream &);
void expect_calvalue(std::istream &);
void expect_component(std::istream &is);
void expect_contentline(std::istream &is);
void expect_iana_prop(std::istream &);
void expect_iana_token(std::istream &);
void expect_ical(std::istream &);
void expect_icalbody(std::istream &);
void expect_icalparameter(std::istream &);
void expect_method(std::istream &);
void expect_metparam(std::istream &);
void expect_metvalue(std::istream &);
void expect_name(std::istream &);
void expect_other_param(std::istream &);
void expect_param(std::istream &);
void expect_param_name(std::istream &);
void expect_param_value(std::istream &);
void expect_pidparam(std::istream &);
void expect_pidvalue(std::istream &);
void expect_prodid(std::istream &);
void expect_text(std::istream &);
void expect_value(std::istream &);
void expect_verparam(std::istream &);
void expect_version(std::istream &);
void expect_vervalue(std::istream &);
void expect_x_name(std::istream &);
void expect_x_prop(std::istream &);

void print_location(std::istream::pos_type pos, std::istream &is);

// -- Exceptions. --------------------------------------------------------------
class invalid_ical : public std::runtime_error {
public:
        invalid_ical() : std::runtime_error("Invalid ICalendar file") {}
};

class not_implemented : public std::runtime_error {
public:
        not_implemented(std::istream::pos_type pos, std::string const &what) :
                std::runtime_error("Not implemented: " + what), pos(pos)
        {}

        std::istream::pos_type pos = 0;
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
                : syntax_error(
                pos,
                "Expected key-value-pair '" + k + ":" + v + "'")
        {}
};

#endif //PARSER_HH_INCLUDED_20190130
