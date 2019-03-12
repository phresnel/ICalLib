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

        struct Unfolder {
        private:
                std::istream &is_;
        public:
                Unfolder(std::istream &is) : is_(is) {
                }

                auto tellg() { return is_.tellg(); }
                auto get() {
                        absorb_folds();
                        return is_.get();
                }

                auto& is() { return is_; }
                std::istream& operator* () { return is_; }
                std::istream* operator-> () { return &is_; }

                friend void print_location(
                        std::istream::pos_type pos,
                        Unfolder &u
                ) {
                        return print_location(pos, u.is_);
                }

        private:
                void absorb_folds() {
                        save_input_pos ptran(is_);

                        using ct = std::char_traits<char>;

                        auto x = is_.get();
                        if (x == ct::eof()) {
                                return;
                        } else if (x == ct::to_int_type('\n')) {
                                x = is_.get();
                        } else if (x == ct::to_int_type('\r')) {
                                x = is_.get();
                                if (x == ct::to_int_type('\n')) {
                                        x = is_.get();
                                }
                        } else {
                                return;
                        }

                        if (x == ct::eof()) {
                                return;
                        } else if (x == ' ' || x == '\t') {
                                do {
                                        x = is_.get();
                                } while (x == ' ' || x == '\t');
                                is_.unget();
                        } else {
                                return;
                        }

                        ptran.commit();
                }
        };
        Unfolder is;
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
        result<string> token(string const &tok);
        result<string> eof();
        result<string> newline();
        result<string> hex();
        result<string> alpha();
        result<string> digit();
        result<string> digit(int min, int max);
        result<string> digits(int at_least, int at_most);
        result<string> digits(int num);
        result<string> alnum();


        // -- Methods. ---------------------------------------------------------
        tuple<string, string> expect_key_value_newline(string const &k, string const &v);
        result<tuple<string, string>> key_value_newline(string const &k, string const &v);

        result<ContentLine> contentline();
        result<string> name();
        result<string> iana_token_char();
        result<string> iana_token();
        result<string> vendorid();
        result<string> safe_char();
        result<string> value_char();
        result<string> qsafe_char();
        result<string> non_us_ascii();
        result<string> control();
        result<string> value();
        result<Param> param();
        result<string> param_name();
        result<string> param_value();
        result<string> quoted_string();
        result<string> paramtext();
        result<Calendar> icalobject();
        result<Calendar> icalbody();
        result<CalProps> calprops();
        result<ProdId> prodid();
        Version expect_version();
        result<Version> version();
        result<std::vector<OtherParam>> verparam();
        result<string> vervalue();
        CalScale expect_calscale();
        result<CalScale> calscale();
        result<vector<OtherParam>> calparam();
        result<string> calvalue();
        result<Method> method();
        result<std::vector<OtherParam>> metparam();
        result<string> metvalue();
        result<XProp> x_prop();
        result<string> x_name();
        result<IanaProp> iana_prop();
        result<std::vector<OtherParam>> pidparam();
        OtherParam expect_other_param();
        result<string> pidvalue();
        result<string> escaped_char();
        result<string> tsafe_char();
        result<string> text_char();
        result<string> dquote();
        result<string> text();
        bool binary();
        result<string> float_();
        result<int> integer();
        result<string> actionvalue();
        result<ActionParam> actionparam();
        result<Action> action();
        result<TrigAbs> trigabs();
        result<TrigRel> trigrel();
        result<Trigger> trigger();
        result<RepParam> repparam();
        result<Repeat> repeat();
        result<AudioProp> audioprop();
        result<DispProp> dispprop();
        result<EmailProp> emailprop();
        result<Alarm> alarmc();
        result<string> date_fullyear();
        result<string> date_month();
        result<string> date_mday();
        result<Date> date_value();
        result<Date> date();
        result<TimeHour> time_hour();
        result<TimeMinute> time_minute();
        result<TimeSecond> time_second();
        result<string> time_utc();
        result<Time> time();
        result<DateTime> date_time();
        result<DurWeek> dur_week();
        result<DurSecond> dur_second();
        result<DurMinute> dur_minute();
        result<DurHour> dur_hour();
        result<DurDay>  dur_day();
        result<DurTime> dur_time();
        result<DurDate> dur_date();
        result<DurValue> dur_value();
        bool period_start();
        bool period_explicit();
        bool period();
        result<OtherParam> other_param();
        result<DtStampParams> stmparam();
        result<DtStamp> dtstamp();
        result<vector<OtherParam>> uidparam();
        result<Uid> uid();
        result<DtStartVal> dtstval();
        result<DtStartParams> dtstparam();
        result<DtStart> dtstart();
        result<string> classvalue();
        result<ClassParams> classparam();
        result<Class> class_();
        result<CreaParam> creaparam();
        result<Created> created();
        result<DescParams> descparam();
        result<Description> description();
        result<GeoValue> geovalue();
        result<GeoParams> geoparam();
        result<Geo> geo();
        result<LstParams> lstparam();
        result<LastMod> last_mod();
        bool locparam_single();
        result<LocParams> locparam();
        result<Location> location();
        result<OrgParams> orgparam();
        result<Uri> cal_address();
        result<Organizer> organizer();
        bool priovalue();
        bool prioparam();
        result<Priority> priority();
        result<SeqParams> seqparam();
        result<Seq> seq();
        result<StatvalueEvent> statvalue_event();
        result<StatvalueTodo> statvalue_todo();
        result<StatvalueJour> statvalue_jour();
        result<Statvalue> statvalue();
        result<StatParams> statparam();
        result<Status> status();
        bool summparam_single();
        result<SummParams> summparam();
        result<Summary> summary();
        bool transparam();
        bool transvalue();
        result<Transp> transp();
        result<Uri> uri();
        bool urlparam();
        result<Url> url();
        bool ridval();
        bool ridparam_single();
        bool ridparam();
        result<SetPosDay> setposday();
        result<BySpList> bysplist();
        result<MonthNum> monthnum();
        result<ByMoList> bymolist();
        result<Weeknum> weeknum();
        result<ByWkNoList> bywknolist();
        result<OrdYrDay> ordyrday();
        result<YearDayNum> yeardaynum();
        result<ByYrDayList> byyrdaylist();
        result<OrdMoDay> ordmoday();
        result<MonthDayNum> monthdaynum();
        result<ByMoDayList> bymodaylist();
        result<WeekDay> weekday();
        result<OrdWk> ordwk();
        bool minus();
        bool plus();
        result<WeekDayNum> weekdaynum();
        result<ByWDayList> bywdaylist();

        result<Hour> hour();
        result<Minutes> minutes();
        result<Seconds> seconds();

        result<ByHrList> byhrlist();
        result<ByMinList> byminlist();
        result<BySecList> byseclist();
        result<EndDate> enddate();
        result<Freq> freq();

        result<Recur> recur();
        result<RecurId> recurid();
        result<RRulParam> rrulparam();
        result<RRule> rrule();
        result<DtEndVal> dtendval();
        result<DtEndParams> dtendparam();
        result<DtEnd> dtend();
        bool durparam();
        result<Duration> duration();
        bool attachparam();
        result<Attach> attach();
        bool attparam();
        result<Attendee> attendee();
        result<CatParams> catparam();
        result<Categories> categories ();
        bool commparam();
        result<Comment> comment();
        bool contparam();
        result<Contact> contact();
        bool exdtval();
        bool exdtparam();
        result<ExDate> exdate();
        bool extdata();
        bool statdesc();
        bool statcode();
        bool rstatparam();
        result<RStatus> rstatus();
        result<RelTypeParam> reltypeparam();
        bool relparam();
        result<Related> related();
        bool resrcparam();
        result<Resources> resources();
        bool rdtval();
        bool rdtparam_single();
        bool rdtparam();
        result<RDate> rdate();
        result<EventProp> eventprop_single();
        result<vector<EventProp>> eventprop();
        result<EventComp> eventc();
        bool todoprop();
        result<TodoComp> todoc();
        bool jourprop();
        result<JournalComp> journalc();
        bool fbprop();
        result<FreeBusyComp> freebusyc();
        result<TzIdPropParam> tzidpropparam();
        result<TzId> tzid();
        result<TzUrlParam> tzurlparam();
        result<TzUrl> tzurl();
        result<TzProp> tzprop();
        result<FrmParam> frmparam();
        result<ToParam> toparam();
        result<UtcOffset> utc_offset();
        result<TimeNumZone> time_numzone();
        result<TzOffsetTo> tzoffsetto();
        result<TzOffsetFrom> tzoffsetfrom();
        result<TzNParam> tznparam();
        result<TzName> tzname();
        result<DaylightC> daylightc();
        result<StandardC> standardc();
        result<TimezoneComp> timezonec();
        result<IanaComp> iana_comp();
        result<XComp> x_comp();
        result<Component> component_single();
        result<vector<Component>> component();
        result<AltRepParam> altrepparam();
        result<CnParam> cnparam();
        result<CuTypeParam> cutypeparam();
        result<DelFromParam> delfromparam();
        result<DelToParam> deltoparam();
        result<DirParam> dirparam();
        result<EncodingParam> encodingparam();
        result<FmtTypeParam> fmttypeparam();
        result<FbTypeParam> fbtypeparam();
        result<LanguageParam> languageparam();
        result<MemberParam> memberparam();
        result<PartStatJour> partstat_jour();
        result<PartStatTodo> partstat_todo();
        result<PartStatEvent> partstat_event();
        result<PartStatParam> partstatparam();
        result<RangeParam> rangeparam();
        result<TrigRelParam> trigrelparam();
        result<RoleParam> roleparam();
        result<RsvpParam> rsvpparam();
        result<SentByParam> sentbyparam();
        result<TzIdPrefix> tzidprefix();
        result<TzIdParam> tzidparam();
        result<ValueType> valuetype();
        result<ValueTypeParam> valuetypeparam();
        result<IanaParam> iana_param();
        result<XParam> x_param();
        ICalParameter expect_icalparameter();
        result<ICalParameter> icalparameter();
};

#endif //PARSER_AS_CLASS_HH_INCLUDED_20190220
