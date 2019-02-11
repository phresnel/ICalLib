#include "icalstream.hh"
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
        using std::visit;
        visit([&os](auto &&v){
                os << v;
        }, v);
        return os;
}

std::ostream& operator<<(std::ostream& os, EventComp const &v) {
        os << "      Event:\n";
        for (auto const &prop : v.properties) {
                os << "        " << prop << '\n';
        }
        return os;
}

std::ostream& operator<<(std::ostream& os, DtStamp const &v) {
        return os << "<DtStamp>";
}

std::ostream& operator<<(std::ostream& os, DtStart const &v) {
        return os << "<DtStart>";
}

std::ostream& operator<<(std::ostream& os, DtEnd const &v) {
        return os << "<DtEnd>";
}

std::ostream& operator<<(std::ostream& os, Uid const &v) {
        return os << "<Uid>";
}

std::ostream& operator<<(std::ostream& os, Class const &v) {
        return os << "<Class>";
}

std::ostream& operator<<(std::ostream& os, Created const &v) {
        return os << "<Created>";
}

std::ostream& operator<<(std::ostream& os, Description const &v) {
        return os << "<Description>";
}

std::ostream& operator<<(std::ostream& os, Geo const &v) {
        return os << "<Geo>";
}

std::ostream& operator<<(std::ostream& os, LastMod const &v) {
        return os << "<LastMod>";
}

std::ostream& operator<<(std::ostream& os, Location const &v) {
        return os << "<Location>";
}

std::ostream& operator<<(std::ostream& os, Organizer const &v) {
        return os << "<Organizer>";
}

std::ostream& operator<<(std::ostream& os, Priority const &v) {
        return os << "<Priority>";
}

std::ostream& operator<<(std::ostream& os, Seq const &v) {
        return os << "<Seq>";
}

std::ostream& operator<<(std::ostream& os, Status const &v) {
        return os << "<Status>";
}

std::ostream& operator<<(std::ostream& os, Summary const &v) {
        return os << "<Summary>";
}

std::ostream& operator<<(std::ostream& os, Transp const &v) {
        return os << "<Transp>";
}

std::ostream& operator<<(std::ostream& os, Url const &v) {
        return os << "<Url>";
}

std::ostream& operator<<(std::ostream& os, RecurId const &v) {
        return os << "<RecurId>";
}

std::ostream& operator<<(std::ostream& os, RRule const &v) {
        return os << "<RRule>";
}

std::ostream& operator<<(std::ostream& os, Duration const &v) {
        return os << "<Duration>";
}

std::ostream& operator<<(std::ostream& os, Attach const &v) {
        return os << "<Attach>";
}

std::ostream& operator<<(std::ostream& os, Attendee const &v) {
        return os << "<Attendee>";
}

std::ostream& operator<<(std::ostream& os, Categories const &v) {
        return os << "<Categories>";
}

std::ostream& operator<<(std::ostream& os, Comment const &v) {
        return os << "<Comment>";
}

std::ostream& operator<<(std::ostream& os, Contact const &v) {
        return os << "<Contact>";
}

std::ostream& operator<<(std::ostream& os, ExDate const &v) {
        return os << "<ExDate>";
}

std::ostream& operator<<(std::ostream& os, RStatus const &v) {
        return os << "<RStatus>";
}

std::ostream& operator<<(std::ostream& os, Related const &v) {
        return os << "<Related>";
}

std::ostream& operator<<(std::ostream& os, Resources const &v) {
        return os << "<Resources>";
}

std::ostream& operator<<(std::ostream& os, RDate const &v) {
        return os << "<RDate>";
}
