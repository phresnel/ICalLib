#ifndef ICAL_HH_INCLUDED_20190130
#define ICAL_HH_INCLUDED_20190130

#include <iosfwd>
#include <optional>
#include <string>
#include <tuple>
#include <vector>

#include "rfc3986.hh"
#include "xvariant.hh"

// string
using std::string;

// containers
using std::vector;

// optional
using std::optional;
using std::nullopt;

// TODO: provide types which express requiredness:
//         mandatory<>
//         optional<>
//         vector<>

// tuple
using std::tuple;
using std::make_tuple;

// parse result
struct parse_error {
        int line, col;
        string msg;
};
struct no_match {
};
template <typename T>
using result = xvariant<parse_error, no_match, T>;

// mixins
struct having_string_name { string name; };
struct having_string_value { string value; };
struct having_integer_value { int value; };
template <typename T> struct having_value { T value; };
struct having_string_values { vector<string> values; };
struct having_uri_values { vector<Uri> values; };

// ICalendar types
struct XParam : having_string_name, having_string_values {};
struct IanaParam : having_string_values { string token; };
using OtherParam = xvariant<IanaParam, XParam>;

struct Param : having_string_name, having_string_values {};

struct having_other_params {  vector<OtherParam> params; };
struct having_params { vector<Param> params; };

struct ProdId : having_string_value, having_other_params {};

struct Version : having_other_params, having_string_value {
        // TODO: this is currently just a hack
};

struct ContentLine :  having_string_name, having_string_value, having_params {};

struct CalScale :  having_string_value, having_other_params { };
struct Method : having_other_params, having_string_value  {};

struct CalProps {
        // required, once :
        ProdId prodId;
        Version version;

        // optional, once :
        optional<CalScale> calScale;
        optional<Method> method;

        // optional, multiple :
        //  x-prop
        //  iana-prop
};

struct PartStatEvent : having_string_value {};
struct PartStatTodo : having_string_value {};
struct PartStatJour: having_string_value {};
using PartStatParam = xvariant<PartStatEvent, PartStatTodo, PartStatJour>;

struct AltRepParam : having_string_value {};
struct CnParam : having_string_value {};
struct CuTypeParam : having_string_value {};
struct DelFromParam : having_string_values {};
struct DelToParam : having_string_values  {};
struct DirParam : having_string_value {};
struct EncodingParam : having_string_value {};
struct FmtTypeParam : having_string_value {};
struct FbTypeParam : having_string_value {};
struct LanguageParam : having_string_value {};
struct MemberParam : having_uri_values {};
struct RangeParam : having_string_value {};
struct TrigRelParam : having_string_value {};
struct RelTypeParam : having_string_value {};
struct RoleParam : having_string_value {};
struct RsvpParam : having_string_value {};
struct SentByParam : having_string_value {};

struct TzIdPrefix : having_string_value {};
struct TzIdParam : having_string_value {
        optional<TzIdPrefix> prefix;
        string paramtext;
};

struct ValueType : having_string_value {};
struct ValueTypeParam : having_value<ValueType> {};

using ICalParameter = xvariant<AltRepParam,
                               CnParam,
                               CuTypeParam,
                               DelFromParam,
                               DelToParam,
                               DirParam,
                               EncodingParam,
                               FmtTypeParam,
                               FbTypeParam,
                               LanguageParam,
                               MemberParam,
                               PartStatParam,
                               RangeParam,
                               TrigRelParam,
                               RelTypeParam,
                               RoleParam,
                               RsvpParam,
                               SentByParam,
                               TzIdParam,
                               ValueTypeParam,
                               OtherParam>;

struct OrgParams : having_other_params {
        optional<CnParam> cn;
        optional<DirParam> dir;
        optional<SentByParam> sentBy;
        optional<LanguageParam> language;
};

struct Date {
        string year;
        string month;
        string day;
};

struct TimeHour : having_string_value {};
struct TimeMinute : having_string_value {};
struct TimeSecond : having_string_value {};

struct Time {
        TimeHour hour;
        TimeMinute minute;
        TimeSecond second;
        optional<string> utc;
};

struct DateTime {
        Date date;
        Time time;
};

struct DtStampParams : having_other_params {
};
struct DtStamp {
        DtStampParams params;
        DateTime date_time;
};

struct Uid : having_string_value,  having_other_params {};

struct DtStartParams : having_string_value, having_other_params {
        TzIdParam tz_id;
};
using DtStartVal = xvariant<DateTime, Date>;
struct DtStart : having_string_value {
        DtStartParams params;
        DtStartVal value;
};

struct DtEndParams : having_string_value, having_other_params {
        TzIdParam tz_id;
};
using DtEndVal = xvariant<DateTime, Date>;
struct DtEnd : having_string_value {
        DtEndParams params;
        DtEndVal value;
};

struct ClassParams : having_other_params {};
struct Class : having_string_value {
        ClassParams params;
};
struct CreaParam : having_other_params {};
struct Created {
        CreaParam params;
        DateTime dateTime;
};
struct DescParams : having_other_params {
        optional<AltRepParam> alt_rep;
        optional<LanguageParam> language;
};
struct Description : having_string_value {
        DescParams params;
};
struct GeoParams : having_other_params {};
struct GeoValue {
        string latitude;
        string longitude;
};
struct Geo {
        GeoParams params;
        GeoValue value;
};
struct LastMod {};

struct LocParams : having_other_params {
        optional<AltRepParam> alt_rep;
        optional<LanguageParam> language;
};
struct Location : having_string_value {
        LocParams params;
};

struct Organizer {
        OrgParams params;
        Uri address;
};
struct Priority {};
struct SeqParams : having_other_params { };
struct Seq : having_integer_value {
        SeqParams params;
};
struct Status {};
struct SummParams : having_other_params {
        optional<AltRepParam> alt_rep;
        optional<LanguageParam> language;
};
struct Summary : having_string_value {
        SummParams params;
};
struct Transp {};
struct Url {};

struct RecurId {};

enum Freq {
        Secondly,
        Minutely,
        Hourly,
        Daily,
        Weekly,
        Monthly,
        Yearly
};
using EndDate = xvariant<Date, DateTime>;

using OrdMoDay = string;
using OrdYrDay = string;
using OrdWk = string;
struct SignedOrdWk {
        int sign;
        OrdWk ordWk;
};
using MonthNum = string;

using Seconds = string;
using Minutes = string;
using Hour = string;

enum WeekDay {
        Sunday,
        Monday,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday
};
struct WeekDayNum {
        optional<SignedOrdWk> week;
        WeekDay weekDay;
};
struct MonthDayNum {
        int sign;
        OrdMoDay day;
};
struct YearDayNum {
        int sign;
        OrdMoDay day;
};

struct Weeknum {};

using SetPosDay = YearDayNum;

using BySecList = vector<Seconds>;
using ByMinList = vector<Minutes>;
using ByHrList = vector<Hour>;
using ByWDayList = vector<WeekDayNum>;
using ByMoDayList = vector<MonthDayNum>;
using ByYrDayList = vector<YearDayNum>;
using ByWkNoList = vector<Weeknum>;
using ByMoList = vector<MonthNum>;
using BySpList = vector<SetPosDay>;

struct Recur {
        Freq freq;
        xvariant<EndDate, string> duration;

        optional<string> interval;
        optional<BySecList> bySecond;
        optional<ByMinList> byMinute;
        optional<ByHrList> byHour;
        optional<ByWDayList> byDay;
        optional<ByMoDayList> byMonthDay;
        optional<ByYrDayList> byYearDay;
        optional<ByWkNoList> byweekNo;
        optional<ByMoList> byMonth;
        optional<BySpList> bySetpos;
        optional<WeekDay> wkst;
};
struct RRulParam {
        vector<OtherParam> otherParams;
};
struct RRule {
        RRulParam param;
        Recur recur;
};
struct Duration {};
struct Attach {};
struct Attendee {};
struct CatParams : having_other_params {
        optional<LanguageParam> language;
};
struct Categories : having_string_values {
        CatParams params;
};
struct Comment {};
struct Contact {};
struct ExDate {};
struct RStatus {};
struct Related {};
struct Resources {};
struct RDate {};
struct XProp :
        having_string_name,
        having_string_value
{
vector<ICalParameter> params;
};

struct DurSecond {
        string second;
};
struct DurMinute {
        string minute;
        optional<DurSecond> second;
};
struct DurHour {
        string hour;
        optional<DurMinute> minute;
};
struct DurDay : having_string_value {};

using DurTime = xvariant<DurHour, DurMinute, DurSecond>;
struct DurDate {
        DurDay day;
        optional<DurTime> time;
};
struct DurWeek : having_string_value {};

struct DurValue {
        bool positive = true;
        xvariant<DurDate, DurTime, DurWeek> value;
};
struct ActionParam : having_other_params {};
struct Action : having_string_value {
        ActionParam params;
};
struct TrigRel : having_other_params {
        string value;
        DurValue durValue;
        TrigRelParam trigRelParam;
};

struct TrigAbs : having_other_params {
        string value;
        DateTime dateTime;
};
using Trigger = xvariant<TrigRel, TrigAbs>;

struct RepParam : having_other_params {};
struct Repeat : having_integer_value {
        RepParam params;
};

struct IanaProp {};

struct AudioProp {
        Action action;
        Trigger trigger;

        optional<Duration> duration;
        optional<Repeat> repeat;

        optional<Attach> attach;

        vector<XProp> xProps;
        vector<IanaProp> ianaProps;

};
struct DispProp {
        Action action;
        Description description;
        Trigger trigger;

        optional<Duration> duration;
        optional<Repeat> repeat;

        vector<XProp> xProps;
        vector<IanaProp> ianaProps;
};
struct EmailProp {
        Action action;
        Description description;
        Trigger trigger;
        Summary summary;

        Attendee attendee;

        optional<Duration> duration;
        optional<Repeat> repeat;

        vector<Attach> attach;
        vector<XProp> xProps;
        vector<IanaProp> ianaProps;
};
using Alarm = xvariant<AudioProp, DispProp, EmailProp>;

using EventProp = xvariant<DtStamp,
                           Uid,
                           DtStart,
                           Class,
                           Created,
                           Description,
                           Geo,
                           LastMod,
                           Location,
                           Organizer,
                           Priority,
                           Seq,
                           Status,
                           Summary,
                           Transp,
                           Url,
                           RecurId,
                           RRule,
                           DtEnd,
                           Duration,
                           Attach,
                           Attendee,
                           Categories,
                           Comment,
                           Contact,
                           ExDate,
                           RStatus,
                           Related,
                           Resources,
                           RDate,
                           XProp,
                           IanaProp>;
struct EventComp {
        vector<EventProp> properties;
        vector<Alarm> alarms;
};

struct TodoComp {};
struct JournalComp {};
struct FreeBusyComp {};

struct TzIdPropParam : having_other_params {};
struct TzUrlParam : having_other_params {};
struct TzId {
        TzIdPropParam propParams;
        TzIdPrefix prefix;
        string text;
};
struct TzUrl {
        TzUrlParam params;
        Uri uri;
};
struct TzNParam {
        LanguageParam languageParam;
        vector<OtherParam> otherParams;
};
struct TzName {
        TzNParam param;
        string text;
};
struct FrmParam {
        vector<OtherParam> otherParams;
};
struct ToParam {
        vector<OtherParam> otherParams;
};
struct TimeNumZone {
        int sign;
        TimeHour hour;
        TimeMinute minute;
        optional<TimeSecond> second;
};
struct UtcOffset {
        TimeNumZone numZone;
};
struct TzOffsetTo {
        ToParam param;
        UtcOffset utcOffset;
};
struct TzOffsetFrom {
        FrmParam param;
        UtcOffset utcOffset;
};
struct TzProp {
        DtStart dtStart;
        TzOffsetTo offsetTo;
        TzOffsetFrom offsetFrom;
        optional<RRule> rRule;
        vector<Comment> comments;
        vector<RDate> rDates;
        vector<TzName> tzNames;
        vector<XProp> xProps;
        vector<IanaProp> ianaProps;
};
struct StandardC {
        TzProp tzProp;
};
struct DaylightC {
        TzProp tzProp;
};
struct TimezoneComp {
        TzId tzId;
        optional<LastMod> lastMod;
        optional<TzUrl> tzUrl;
        xvariant<StandardC, DaylightC> observance; // TODO: not sure if this
                                                   // name is proper
        vector<XProp> xProps;
        vector<IanaProp> ianaProps;
};
struct IanaComp {};
struct XComp {};
using Component = xvariant<EventComp,
                           TodoComp,
                           JournalComp,
                           FreeBusyComp,
                           TimezoneComp,
                           IanaComp,
                           XComp>;

struct Calendar {
        CalProps properties;
        vector<Component> components;
};

#endif //ICAL_HH_INCLUDED_20190130
