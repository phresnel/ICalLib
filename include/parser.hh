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

// -- Includes. ----------------------------------------------------------------
#include <iosfwd>
#include <string>
#include <vector>
#include <optional>
#include "parser_helpers.hh"
#include "ical.hh"

// -- Typedefs. ----------------------------------------------------------------
using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::istream;

// -- Forward declarations. ----------------------------------------------------
optional<tuple<string, string>> expect_key_value(
        istream &is,
        string const &k,
        string const &v
);
bool read_key_value(istream &is, string const &k, string const &v);

void expect_contentline(istream &is);
bool read_contentline(istream &is);
void expect_name(istream &is);
bool read_iana_token_char(istream &is);
bool read_iana_token(istream &is);
void expect_iana_token(istream &is);
void expect_vendorid(istream &is);
bool read_vendorid(istream &is);
bool read_safe_char(istream &is);
bool read_value_char(istream &is);
bool read_qsafe_char(istream &is);
bool read_non_us_ascii(istream &is);
bool read_control(istream &is);
string expect_value(istream &is);
void expect_param(istream &is);
void expect_param_name(istream &is);
bool read_param_value(istream &is);
bool read_quoted_string(istream &is);
void expect_param_value(istream &is);
bool read_paramtext(istream &is);
void expect_ical(istream &is);
bool read_ical(istream &is);
void expect_icalbody(istream &is);
CalProps expect_calprops(istream &is);
ProdId expect_prodid(istream &is);
optional<ProdId> read_prodid(istream &is);
Version expect_version(istream &is);
optional<Version> read_version(istream &is);
std::vector<OtherParam> expect_verparam(istream &is);
string expect_vervalue(istream &is);
void expect_calscale(istream &is);
bool read_calscale(istream &is);
void expect_calparam(istream &is);
void expect_calvalue(istream &is);
bool read_method(istream &is);
void expect_method(istream &is);
void expect_metparam(istream &is);
void expect_metvalue(istream &is);
void expect_x_prop(istream &is);
bool read_x_prop(istream &is);
string expect_x_name(istream &is);
optional<string> read_x_name(istream &is);
void expect_iana_prop(istream &is);
bool read_iana_prop(istream &is);
std::vector<OtherParam> expect_pidparam(istream &is);
OtherParam expect_other_param(istream &is);
string expect_pidvalue(istream &is);
optional<string> read_escaped_char(istream &is);
optional<string> read_tsafe_char(istream &is);
optional<string> read_text_char(istream &is);
string expect_text(istream &is);
optional<string> read_dquote(istream &is);
optional<string> read_text(istream &is);
bool read_binary(istream &is);
bool read_float(istream &is);
bool read_integer(istream &is);
bool read_actionvalue(istream &is);
bool read_actionparam(istream &is);
bool read_action(istream &is);
bool read_trigabs(istream &is);
bool read_trigrel_single(istream &is);
bool read_trigrel(istream &is);
bool read_trigger(istream &is);
bool read_repparam(istream &is);
bool read_repeat(istream &is);
bool read_audioprop_single(istream &is);
bool read_dispprop_single(istream &is);
bool read_emailprop_single(istream &is);
bool read_alarmc_prop(istream &is);
bool read_alarmc(istream &is);
bool read_date_fullyear(istream &is);
bool read_date_month(istream &is);
bool read_date_mday(istream &is);
bool read_date_value(istream &is);
bool read_date(istream &is);
bool read_time_hour(istream &is);
bool read_time_minute(istream &is);
bool read_time_second(istream &is);
bool read_time_utc(istream &is);
bool read_time(istream &is);
bool read_date_time(istream &is);
bool read_dur_week(istream &is);
bool read_dur_second(istream &is);
bool read_dur_minute(istream &is);
bool read_dur_hour(istream &is);
bool read_dur_day(istream &is);
bool read_dur_time(istream &is);
bool read_dur_date(istream &is);
bool read_dur_value(istream &is);
bool read_period_start(istream &is);
bool read_period_explicit(istream &is);
bool read_period(istream &is);
bool read_other_param(istream &is);
bool read_stmparam(istream &is);
bool read_dtstamp(istream &is);
bool read_uidparam(istream &is);
bool read_uid(istream &is);
bool read_dtstval(istream &is);
bool read_dtstparam_single(istream &is);
bool read_dtstparam(istream &is);
bool read_dtstart(istream &is);
bool read_classvalue(istream &is);
bool read_classparam(istream &is);
bool read_class(istream &is);
bool read_creaparam(istream &is);
bool read_created(istream &is);
bool read_descparam(istream &is);
bool read_description(istream &is);
bool read_geovalue(istream &is);
bool read_geoparam(istream &is);
bool read_geo(istream &is);
bool read_lstparam(istream &is);
bool read_last_mod(istream &is);
bool read_locparam_single(istream &is);
bool read_locparam(istream &is);
bool read_location(istream &is);
bool read_orgparam(istream &is);
bool read_cal_address(istream &is);
bool read_organizer(istream &is);
bool read_priovalue(istream &is);
bool read_prioparam(istream &is);
bool read_priority(istream &is);
bool read_seqparam(istream &is);
bool read_seq(istream &is);
bool read_statvalue_jour(istream &is);
bool read_statvalue_todo(istream &is);
bool read_statvalue(istream &is);
bool read_statvalue_event(istream &is);
bool read_statparam(istream &is);
bool read_status(istream &is);
bool read_summparam_single(istream &is);
bool read_summparam(istream &is);
bool read_summary(istream &is);
bool read_transparam(istream &is);
bool read_transvalue(istream &is);
bool read_transp(istream &is);
bool read_uri(istream &is);
bool read_urlparam(istream &is);
bool read_url(istream &is);
bool read_ridval(istream &is);
bool read_ridparam_single(istream &is);
bool read_ridparam(istream &is);
bool read_setposday(istream &is);
bool read_bysplist(istream &is);
bool read_monthnum(istream &is);
bool read_bymolist(istream &is);
bool read_weeknum(istream &is);
bool read_bywknolist(istream &is);
bool read_ordyrday(istream &is);
bool read_yeardaynum(istream &is);
bool read_byyrdaylist(istream &is);
bool read_ordmoday(istream &is);
bool read_monthdaynum(istream &is);
bool read_bymodaylist(istream &is);
bool read_weekday(istream &is);
bool read_ordwk(istream &is);
bool readminus(istream &is);
bool read_plus(istream &is);
bool read_weekdaynum(istream &is);
bool read_bywdaylist(istream &is);
bool read_hour(istream &is);
bool read_byhrlist(istream &is);
bool read_minutes(istream &is);
bool read_byminlist(istream &is);
bool read_seconds(istream &is);
bool read_byseclist(istream &is);
bool read_enddate(istream &is);
bool read_freq(istream &is);
bool read_recur_rule_part(istream &is);
bool read_recur(istream &is);
bool read_recurid(istream &is);
bool read_rrulparam(istream &is);
bool read_rrule(istream &is);
bool read_dtendval(istream &is);
bool read_dtendparam(istream &is);
bool read_dtend(istream &is);
bool read_durparam(istream &is);
bool read_duration(istream &is);
bool read_attachparam(istream &is);
bool read_attach(istream &is);
bool read_attparam(istream &is);
bool read_attendee(istream &is);
bool read_catparam(istream &is);
bool read_categories (istream &is);
bool read_commparam(istream &is);
bool read_comment(istream &is);
bool read_contparam(istream &is);
bool read_contact(istream &is);
bool read_exdtval(istream &is);
bool read_exdtparam(istream &is);
bool read_exdate(istream &is);
bool read_extdata(istream &is);
bool read_statdesc(istream &is);
bool read_statcode(istream &is);
bool read_rstatparam(istream &is);
bool read_rstatus(istream &is);
bool read_reltypeparam(istream &is);
bool read_relparam(istream &is);
bool read_related(istream &is);
bool read_resrcparam(istream &is);
bool read_resources(istream &is);
bool read_rdtval(istream &is);
bool read_rdtparam_single(istream &is);
bool read_rdtparam(istream &is);
bool read_rdate(istream &is);
bool read_eventprop_single(istream &is);
bool read_eventprop(istream &is);
bool read_eventc(istream &is);
bool read_todoprop(istream &is);
bool read_todoc(istream &is);
bool read_jourprop(istream &is);
bool read_journalc(istream &is);
bool read_fbprop(istream &is);
bool read_freebusyc(istream &is);
bool read_tzid(istream &is);
bool read_tzurl(istream &is);
bool read_tzprop(istream &is);
bool read_daylightc(istream &is);
bool read_standardc(istream &is);
bool read_timezonec(istream &is);
bool read_iana_comp(istream &is);
bool read_x_comp(istream &is);
bool read_component_single(istream &is);
void expect_component_single(istream &is);
void expect_component(istream &is);
bool read_dquoted_value(istream &is);
bool read_altrepparam(istream &is);
bool read_cnparam(istream &is);
bool read_cutypeparam(istream &is);
bool read_delfromparam(istream &is);
bool read_deltoparam(istream &is);
bool read_dirparam(istream &is);
bool read_encodingparam(istream &is);
bool read_fmttypeparam(istream &is);
bool read_fbtypeparam(istream &is);
bool read_languageparam(istream &is);
bool read_memberparam(istream &is);
bool read_partstat_jour(istream &is);
bool read_partstat_todo(istream &is);
bool read_partstat_event(istream &is);
bool read_partstatparam(istream &is);
bool read_rangeparam(istream &is);
bool read_trigrelparam(istream &is);
bool read_roleparam(istream &is);
bool read_rsvpparam(istream &is);
bool read_sentbyparam(istream &is);
optional<string> read_tzidprefix(istream &is);
optional<string> read_optional_tzidprefix(istream &is);
bool read_tzidparam(istream &is);
bool read_valuetype(istream &is);
bool read_valuetypeparam(istream &is);
bool read_iana_param(istream &is);
bool read_x_param(istream &is);
void expect_icalparameter(istream &is);
bool read_icalparameter(istream &is);


// -- Exceptions. --------------------------------------------------------------
class invalid_ical : public std::runtime_error {
public:
        invalid_ical() : std::runtime_error("Invalid ICalendar file") {}
};

class syntax_error : public std::runtime_error {
public:
        syntax_error(istream::pos_type pos) :
                std::runtime_error("Syntax error."),
                pos(pos)
        {}

        syntax_error(istream::pos_type pos, std::string const &msg) :
                std::runtime_error(msg),
                pos(pos)
        {}

        istream::pos_type pos = 0;
};

class unexpected_token : public syntax_error {
public:
        unexpected_token(istream::pos_type pos) : syntax_error(pos) {}

        unexpected_token(istream::pos_type pos, string const &tok) :
                syntax_error(pos, "Expected token '" + tok + "' not found")
        {}
};

class key_value_pair_expected : public syntax_error {
public:
        key_value_pair_expected(istream::pos_type pos) :
                syntax_error(pos, "Expected key-value-pair")
        {}

        key_value_pair_expected(istream::pos_type pos,
                                string const &k,
                                string const &v)
                : syntax_error(
                pos,
                "Expected key-value-pair '" + k + ":" + v + "'")
        {}
};

#endif //PARSER_HH_INCLUDED_20190130
