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
                return os << *p;
        }
        if (auto p = get_opt<TodoComp>(v)) {
                return os << "todo-comp\n";
                //return os << *p;
        }
        if (auto p = get_opt<JournalComp>(v)) {
                return os << "journal-comp\n";
                //return os << *p;
        }
        if (auto p = get_opt<FreeBusyComp>(v)) {
                return os << "free-busy-comp\n";
                //return os << *p;
        }
        if (auto p = get_opt<TimezoneComp>(v)) {
                return os << "timezone-comp\n";
                //return os << *p;
        }
        if (auto p = get_opt<IanaComp>(v)) {
                return os << "iana-comp\n";
                //return os << *p;
        }
        if (auto p = get_opt<XComp>(v)) {
                return os << "x-comp\n";
                //return os << *p;
        }
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


template <typename ...Types>
std::ostream& operator<<(std::ostream &os, xvariant<Types...> const &v) {
        xvariant<Types...> b;
        visit([&os](auto&& v) {
                //os << v;
        }, b);
        return os;
}

std::ostream& operator<<(std::ostream& os, EventComp const &v) {
        os << "      Event:\n";
        for (auto const &prop : v.properties) {
                os << "        " << prop << '\n';
        }
        return os;
}
