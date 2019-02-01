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

struct OtherParam {
};

struct Param {
        string name;
        vector<string> values;
};

struct ProdId {
        vector<OtherParam> params;
        string value;
};

struct Version {
        vector<OtherParam> params;
        string value; // TODO: this is currently just a hack
};

struct ContentLine {
        string name;
        vector<Param> params;
        string value;
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
struct XProp {};
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
