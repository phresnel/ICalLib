#ifndef PARSER_AS_CLASS_HH_INCLUDED_20190220
#define PARSER_AS_CLASS_HH_INCLUDED_20190220

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
#include <variant>
#include "parser_helpers.hh"
#include "ical.hh"

// -- Typedefs. ----------------------------------------------------------------
using std::string;
using std::vector;
using std::optional;
using std::nullopt;
using std::istream;
using std::variant;

class IcalParser {
        std::istream &is;
public:
        explicit IcalParser(std::istream &is) : is{is} {
        }

        // -- Helpers. ---------------------------------------------------------
        string expect_token(string const &tok);
        string expect_newline();
        string expect_alpha();
        string expect_digit();
        string expect_digit(int min, int max);
        string expect_alnum();
        optional<string> token(string const &tok);
        optional<string> eof();
        optional<string> newline();
        optional<string> hex();
        optional<string> alpha();
        optional<string> digit();
        optional<string> digit(int min, int max);
        optional<string> digits(int at_least, int at_most);
        optional<string> digits(int num);
        optional<string> alnum();


        // -- Methods. ---------------------------------------------------------
        tuple<string, string> expect_key_value(string const &k, string const &v);
        optional<tuple<string, string>> key_value(string const &k, string const &v);

        ContentLine expect_contentline();
        optional<ContentLine> contentline();
        string expect_name();
        optional<string> iana_token_char();
        optional<string> iana_token();
        string expect_iana_token();
        string expect_vendorid();
        optional<string> vendorid();
        optional<string> safe_char();
        optional<string> value_char();
        optional<string> qsafe_char();
        optional<string> non_us_ascii();
        optional<string> control();
        string expect_value();
        Param expect_param();
        string expect_param_name();
        optional<string> param_value();
        optional<string> quoted_string();
        string expect_param_value();
        optional<string> paramtext();
        Calendar expect_ical();
        optional<Calendar> ical();
        Calendar expect_icalbody();
        CalProps expect_calprops();
        ProdId expect_prodid();
        optional<ProdId> prodid();
        Version expect_version();
        optional<Version> version();
        std::vector<OtherParam> expect_verparam();
        string expect_vervalue();
        CalScale expect_calscale();
        optional<CalScale> calscale();
        vector<OtherParam> expect_calparam();
        string expect_calvalue();
        optional<Method> method();
        Method expect_method();
        std::vector<OtherParam> expect_metparam();
        string expect_metvalue();
        XProp expect_x_prop();
        optional<XProp> x_prop();
        string expect_x_name();
        optional<string> x_name();
        void expect_iana_prop();
        optional<IanaProp> iana_prop();
        std::vector<OtherParam> expect_pidparam();
        OtherParam expect_other_param();
        string expect_pidvalue();
        optional<string> escaped_char();
        optional<string> tsafe_char();
        optional<string> text_char();
        string expect_text();
        optional<string> dquote();
        optional<string> text();
        bool binary();
        optional<string> float_();
        optional<int> integer();
        optional<string> actionvalue();
        optional<ActionParam> actionparam();
        optional<Action> action();
        optional<TrigAbs> trigabs();
        optional<TrigRel> trigrel();
        optional<Trigger> trigger();
        optional<RepParam> repparam();
        optional<Repeat> repeat();
        optional<AudioProp> audioprop();
        optional<DispProp> dispprop();
        optional<EmailProp> emailprop();
        optional<Alarm> alarmc();
        optional<string> date_fullyear();
        optional<string> date_month();
        optional<string> date_mday();
        optional<Date> date_value();
        optional<Date> date();
        optional<string> time_hour();
        optional<string> time_minute();
        optional<string> time_second();
        optional<string> time_utc();
        optional<Time> time();
        optional<DateTime> date_time();
        optional<DurWeek> dur_week();
        optional<DurSecond> dur_second();
        optional<DurMinute> dur_minute();
        optional<DurHour> dur_hour();
        optional<DurDay>  dur_day();
        optional<DurTime> dur_time();
        optional<DurDate> dur_date();
        optional<DurValue> dur_value();
        bool period_start();
        bool period_explicit();
        bool period();
        optional<OtherParam> other_param();
        optional<DtStampParams> stmparam();
        optional<DtStamp> dtstamp();
        optional<vector<OtherParam>> uidparam();
        optional<Uid> uid();
        optional<DtStartVal> dtstval();
        optional<DtStartParams> dtstparam();
        optional<DtStart> dtstart();
        optional<string> classvalue();
        optional<ClassParams> classparam();
        optional<Class> class_();
        bool creaparam();
        optional<Created> created();
        optional<DescParams> descparam();
        optional<Description> description();
        optional<GeoValue> geovalue();
        optional<GeoParams> geoparam();
        optional<Geo> geo();
        bool lstparam();
        optional<LastMod> last_mod();
        bool locparam_single();
        optional<LocParams> locparam();
        optional<Location> location();
        optional<OrgParams> orgparam();
        optional<Uri> cal_address();
        optional<Organizer> organizer();
        bool priovalue();
        bool prioparam();
        optional<Priority> priority();
        optional<SeqParams> seqparam();
        optional<Seq> seq();
        bool statvalue_jour();
        bool statvalue_todo();
        bool statvalue();
        bool statvalue_event();
        bool statparam();
        optional<Status> status();
        bool summparam_single();
        optional<SummParams> summparam();
        optional<Summary> summary();
        bool transparam();
        bool transvalue();
        optional<Transp> transp();
        optional<Uri> uri();
        bool urlparam();
        optional<Url> url();
        bool ridval();
        bool ridparam_single();
        bool ridparam();
        bool setposday();
        bool bysplist();
        bool monthnum();
        bool bymolist();
        bool weeknum();
        bool bywknolist();
        bool ordyrday();
        bool yeardaynum();
        bool byyrdaylist();
        bool ordmoday();
        bool monthdaynum();
        bool bymodaylist();
        bool weekday();
        bool ordwk();
        bool minus();
        bool plus();
        bool weekdaynum();
        bool bywdaylist();
        bool hour();
        bool byhrlist();
        bool minutes();
        bool byminlist();
        bool seconds();
        bool byseclist();
        bool enddate();
        bool freq();
        bool recur_rule_part();
        bool recur();
        optional<RecurId> recurid();
        bool rrulparam();
        optional<RRule> rrule();
        optional<DtEndVal> dtendval();
        optional<DtEndParams> dtendparam();
        optional<DtEnd> dtend();
        bool durparam();
        optional<Duration> duration();
        bool attachparam();
        optional<Attach> attach();
        bool attparam();
        optional<Attendee> attendee();
        optional<CatParams> catparam();
        optional<Categories> categories ();
        bool commparam();
        optional<Comment> comment();
        bool contparam();
        optional<Contact> contact();
        bool exdtval();
        bool exdtparam();
        optional<ExDate> exdate();
        bool extdata();
        bool statdesc();
        bool statcode();
        bool rstatparam();
        optional<RStatus> rstatus();
        optional<RelTypeParam> reltypeparam();
        bool relparam();
        optional<Related> related();
        bool resrcparam();
        optional<Resources> resources();
        bool rdtval();
        bool rdtparam_single();
        bool rdtparam();
        optional<RDate> rdate();
        optional<EventProp> eventprop_single();
        optional<vector<EventProp>> eventprop();
        optional<EventComp> eventc();
        bool todoprop();
        optional<TodoComp> todoc();
        bool jourprop();
        optional<JournalComp> journalc();
        bool fbprop();
        optional<FreeBusyComp> freebusyc();
        bool tzid();
        bool tzurl();
        bool tzprop();
        bool daylightc();
        bool standardc();
        optional<TimezoneComp> timezonec();
        optional<IanaComp> iana_comp();
        optional<XComp> x_comp();
        optional<Component> component_single();
        Component expect_component_single();
        vector<Component> expect_component();
        optional<AltRepParam> altrepparam();
        optional<CnParam> cnparam();
        optional<CuTypeParam> cutypeparam();
        optional<DelFromParam> delfromparam();
        optional<DelToParam> deltoparam();
        optional<DirParam> dirparam();
        optional<EncodingParam> encodingparam();
        optional<FmtTypeParam> fmttypeparam();
        optional<FbTypeParam> fbtypeparam();
        optional<LanguageParam> languageparam();
        optional<MemberParam> memberparam();
        optional<PartStatJour> partstat_jour();
        optional<PartStatTodo> partstat_todo();
        optional<PartStatEvent> partstat_event();
        optional<PartStatParam> partstatparam();
        optional<RangeParam> rangeparam();
        optional<TrigRelParam> trigrelparam();
        optional<RoleParam> roleparam();
        optional<RsvpParam> rsvpparam();
        optional<SentByParam> sentbyparam();
        optional<string> tzidprefix();
        optional<TzIdParam> tzidparam();
        optional<ValueType> valuetype();
        optional<ValueTypeParam> valuetypeparam();
        optional<IanaParam> iana_param();
        optional<XParam> x_param();
        ICalParameter expect_icalparameter();
        optional<ICalParameter> icalparameter();
};

#endif //PARSER_AS_CLASS_HH_INCLUDED_20190220
