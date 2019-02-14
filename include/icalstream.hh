#ifndef ICALSTREAM_HH_INCLUDED_20190111
#define ICALSTREAM_HH_INCLUDED_20190211

#include "ical.hh"

std::ostream& operator<<(std::ostream& os, ProdId const &);
std::ostream& operator<<(std::ostream& os, Version const &);
std::ostream& operator<<(std::ostream& os, ContentLine const &);
std::ostream& operator<<(std::ostream& os, Param const &);

std::ostream& operator<<(std::ostream& os, Component const &);
std::ostream& operator<<(std::ostream& os, std::vector<Component> const &);
std::ostream& operator<<(std::ostream& os, CalProps const &);
std::ostream& operator<<(std::ostream& os, Calendar const &);

std::ostream& operator<<(std::ostream& os, CalScale const &);
std::ostream& operator<<(std::ostream& os, Method const &);
std::ostream& operator<<(std::ostream& os, XProp const &);
std::ostream& operator<<(std::ostream& os, IanaProp const &);

std::ostream& operator<<(std::ostream& os, EventComp const &);

std::ostream& operator<<(std::ostream& os, DtStamp const &v);
std::ostream& operator<<(std::ostream& os, DtStart const &v);
std::ostream& operator<<(std::ostream& os, DtEnd const &v);
std::ostream& operator<<(std::ostream& os, Uid const &v);
std::ostream& operator<<(std::ostream& os, Class const &v);
std::ostream& operator<<(std::ostream& os, Created const &v);
std::ostream& operator<<(std::ostream& os, Description const &v);
std::ostream& operator<<(std::ostream& os, Geo const &v);
std::ostream& operator<<(std::ostream& os, LastMod const &v);
std::ostream& operator<<(std::ostream& os, Location const &v);
std::ostream& operator<<(std::ostream& os, Organizer const &v);
std::ostream& operator<<(std::ostream& os, Priority const &v);
std::ostream& operator<<(std::ostream& os, Seq const &v);
std::ostream& operator<<(std::ostream& os, Status const &v);
std::ostream& operator<<(std::ostream& os, Summary const &v);
std::ostream& operator<<(std::ostream& os, Transp const &v);
std::ostream& operator<<(std::ostream& os, Url const &v);
std::ostream& operator<<(std::ostream& os, RecurId const &v);
std::ostream& operator<<(std::ostream& os, RRule const &v);
std::ostream& operator<<(std::ostream& os, Duration const &v);
std::ostream& operator<<(std::ostream& os, Attach const &v);
std::ostream& operator<<(std::ostream& os, Attendee const &v);
std::ostream& operator<<(std::ostream& os, Categories const &v);
std::ostream& operator<<(std::ostream& os, Comment const &v);
std::ostream& operator<<(std::ostream& os, Contact const &v);
std::ostream& operator<<(std::ostream& os, ExDate const &v);
std::ostream& operator<<(std::ostream& os, RStatus const &v);
std::ostream& operator<<(std::ostream& os, Related const &v);
std::ostream& operator<<(std::ostream& os, Resources const &v);
std::ostream& operator<<(std::ostream& os, RDate const &v);

std::ostream& operator<<(std::ostream& os, Date const &v);
std::ostream& operator<<(std::ostream& os, Time const &v);
std::ostream& operator<<(std::ostream& os, DateTime const &v);

#endif //ICALSTREAM_HH_INCLUDED_20190211
