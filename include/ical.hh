#ifndef ICAL_HH_INCLUDED_20190130
#define ICAL_HH_INCLUDED_20190130

#include <vector>
#include <string>
#include <string>
#include <optional>
#include <iosfwd>


using std::string;
using std::string;
using std::vector;
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

struct Calendar {
};

std::ostream& operator<<(std::ostream& os, ProdId const&);
std::ostream& operator<<(std::ostream& os, Version const&);
std::ostream& operator<<(std::ostream& os, CalProps const&);
std::ostream& operator<<(std::ostream& os, Calendar const&);

#endif //ICAL_HH_INCLUDED_20190130
