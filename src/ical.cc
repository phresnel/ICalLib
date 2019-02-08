#include "ical.hh"
#include <iostream>


std::ostream& operator<<(std::ostream& os, ProdId const& v) {
        return os << "<ProdId: " << v.value << ">";
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

std::ostream& operator<<(std::ostream& os, CalProps const& v) {
        os << "    CalProps: \n"
           << "      prodId:" << v.prodId << "\n"
           << "      version:" << v.version << "\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, Component const& v) {
        if (auto p = get_opt<EventComp>(v)) {
                std::cerr << "event-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<TodoComp>(v)) {
                std::cerr << "todo-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<JournalComp>(v)) {
                std::cerr << "journal-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<FreeBusyComp>(v)) {
                std::cerr << "free-busy-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<TimezoneComp>(v)) {
                std::cerr << "timezone-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<IanaComp>(v)) {
                std::cerr << "iana-comp" << std::endl;
                return os << *p;
        }
        if (auto p = get_opt<XComp>(v)) {
                std::cerr << "x-comp" << std::endl;
                return os << *p;
        }
        std::cerr << "invalid variant type" << std::endl;
        return os << "invalid variant type in Component";
}

std::ostream& operator<<(std::ostream& os, std::vector<Component> const& v) {
        for (auto i=0; i!=v.size(); ++i) {
                if (i) os << ",\n";
                os << "    " << v[i];
        }
        return os;
}

std::ostream& operator<<(std::ostream& os, Calendar const&v) {
        return os << "Calendar:\n"
                  << "  properties: \n"
                  << v.properties << ",\n"
                  << "  components: \n"
                  << v.components << "\n";
}

std::ostream& operator<<(std::ostream& os, CalScale const &v) {
        os << "<CalScale:\n"
           << "  value: " << v.value << ",\n"
           << "  param-count: " << v.params.size() << "\n"
           << ">\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, Method const &v) {
        os << "<Method:\n"
           << "  value: " << v.value << ",\n"
           << "  param-count: " << v.params.size() << "\n"
           << ">\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, XProp const &v) {
        os << "<XProp:\n"
           << "  name: " << v.name << ",\n"
           << "  value: " << v.value << ",\n"
           << "  param-count: " << v.params.size() << "\n"
           << ">\n";
        return os;
}

std::ostream& operator<<(std::ostream& os, IanaProp const &v) {
        os << "<IanaProp>\n";
        return os;
}
