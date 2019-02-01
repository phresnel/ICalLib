#include "ical.hh"
#include <iostream>


std::ostream& operator<<(std::ostream& os, ProdId const&) {
        return os << "<ProdId>";
}

std::ostream& operator<<(std::ostream& os, Version const& v) {
        return os << "<Version: " << v.value << " / "
                  << v.params.size() << " args>";
}

std::ostream& operator<<(std::ostream& os, CalProps const&) {
        return os << "<CalProps>";
}

std::ostream& operator<<(std::ostream& os, Calendar const&) {
        return os << "<Calendar>";
}
