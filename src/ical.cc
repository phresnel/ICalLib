#include "ical.hh"
#include <iostream>


std::ostream& operator<<(std::ostream& os, ProdId const&) {
        return os << "<ProdId>";
}

std::ostream& operator<<(std::ostream& os, Version const& v) {
        return os << "<Version: " << v.value << " / "
                  << v.params.size() << " args>";
}

std::ostream& operator<<(std::ostream& os, ContentLine const& v) {
        os << "<ContentLine: \n"
           << "  name:" << v.name << "\n";
        for(auto &i : v.params) {
                os << "  param:" << i << "\n";
        }
        os << "  value:" << v.value << "\n"
           << ">\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, Param const& v) {
        os << "<Param: \n"
           << "  name:" << v.name << "\n";
        for(auto &i : v.values) {
                os << "  value:" << i << "\n";
        }
        os << ">\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, CalProps const&) {
        return os << "<CalProps>";
}

std::ostream& operator<<(std::ostream& os, Calendar const&) {
        return os << "<Calendar>";
}
