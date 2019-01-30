#ifndef ICAL_HH_INCLUDED_20190130
#define ICAL_HH_INCLUDED_20190130

#include <vector>
#include <string>
#include <string>
#include <optional>

using std::string;
using std::optional;
using std::nullopt;

struct OtherParam {
};

struct ProdId {
        std::vector<OtherParam> params;
        std::string value;
};

struct Version {
        std::vector<OtherParam> params;
        std::string value; // TODO: this is currently just a hack
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

struct ICalendar {

};

#endif //ICAL_HH_INCLUDED_20190130
