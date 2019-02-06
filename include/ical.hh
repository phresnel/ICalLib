#ifndef ICAL_HH_INCLUDED_20190130
#define ICAL_HH_INCLUDED_20190130

#include <iosfwd>
#include <optional>
#include <string>
#include <tuple>
#include <variant>
#include <vector>

using std::string;
using std::variant;
using std::vector;
using std::optional;
using std::nullopt;
using std::tuple;
using std::make_tuple;

struct having_string_name { string name; };
struct having_string_value { string value; };
template <typename T> struct having_value { T value; };
struct having_string_values { vector<string> values; };

struct XParam {};
struct IanaParam {};
using OtherParam = variant<XParam, IanaParam>;

struct Param : having_string_name, having_string_values {
        string name;
};

struct ProdId : having_string_value {
        vector<OtherParam> params;
};

struct Version {
        vector<OtherParam> params;
        string value; // TODO: this is currently just a hack
};

struct ContentLine :
        having_string_name,
        having_string_value
{
        vector<Param> params;
};

struct Alarm {
};


struct CalProps {
        // required, once :
        ProdId prodId;
        Version version;

        // optional, once :
        //  calscale
        //  method

        // optional, multiple :
        //  x-prop
        //  iana-prop
};

struct PartStatEvent : having_string_value {};
struct PartStatTodo : having_string_value {};
struct PartStatJour: having_string_value {};
using PartStatParam = variant<PartStatEvent, PartStatTodo, PartStatJour>;

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
struct MemberParam : having_string_values {};
struct RangeParam : having_string_value {};
struct TrigRelParam : having_string_value {};
struct RelTypeParam : having_string_value {};
struct RoleParam : having_string_value {};
struct RsvpParam : having_string_value {};
struct SentByParam : having_string_value {};
struct TzIdParam : having_string_value {};

struct ValueType : having_string_value {};
struct ValueTypeParam : having_value<ValueType> {};

using ICalParameter = variant<AltRepParam,
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

struct DtStamp {};
struct Uid {};
struct DtStart {};
struct Class {};
struct Created {};
struct Description {};
struct Geo {};
struct LastMod {};
struct Location {};
struct Organizer {};
struct Priority {};
struct Seq {};
struct Status {};
struct Summary {};
struct Transp {};
struct Url {};
struct RecurId {};
struct RRule {};
struct DtEnd {};
struct Duration {};
struct Attach {};
struct Attendee {};
struct Categories {};
struct Comment {};
struct Contact {};
struct ExDate {};
struct RStatus {};
struct Related {};
struct Resources {};
struct RDate {};
struct XProp {
        string name;
        vector<ICalParameter> parameters;
        string value;
};
struct IanaProp {};

using EventProp = variant<DtStamp,
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
struct TimezoneComp {};
struct IanaComp {};
struct XComp {};
using Component = variant<EventComp,
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

std::ostream& operator<<(std::ostream& os, ProdId const&);
std::ostream& operator<<(std::ostream& os, Version const&);
std::ostream& operator<<(std::ostream& os, ContentLine const&);
std::ostream& operator<<(std::ostream& os, Param const&);
std::ostream& operator<<(std::ostream& os, CalProps const&);
std::ostream& operator<<(std::ostream& os, Calendar const&);

#endif //ICAL_HH_INCLUDED_20190130
