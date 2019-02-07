#ifndef RFC3986_HH_INCLUDED_20190201
#define RFC3986_HH_INCLUDED_20190201

// -- URI (RFC 3986) Parser Helpers. ------------------------------------
// [RFC 3986](https://tools.ietf.org/html/rfc3986)

#include <iosfwd>
#include <string>
#include <optional>


inline namespace rfc3986 {

using std::string;
using std::optional;
using std::nullopt;


struct Authority {
        optional<string> userinfo;
        string host;
        optional<string> port;
};
inline std::string to_string(Authority const &v) {
        return (v.userinfo ? (*v.userinfo + '@') : (string(""))) +
               v.host + ":" +
               (v.port ? (":" + *v.port) : (string("")));
}


struct Uri {
        string scheme;
        string hier_part;
        optional<string> query;
        optional<string> fragment;
};
inline std::string to_string(Uri const &v) {
        return v.scheme + ":" +
               v.hier_part +
               (v.query    ? ("?" + *v.query) : (string(""))) +
               (v.fragment ? ("#" + *v.fragment) : (string("")));
}


optional<string> read_gen_delims(std::istream &is);
optional<string> read_sub_delims(std::istream &is);
optional<string> read_reserved(std::istream &is);
optional<string> read_unreserved(std::istream &is);
optional<string> read_pct_encoded(std::istream &is);
optional<string> read_pchar(std::istream &is);
optional<string> read_segment(std::istream &is);
optional<string> read_segment_nz(std::istream &is);
optional<string> read_segment_nz_nc_single(std::istream &is);
optional<string> read_segment_nz_nc(std::istream &is);
optional<string> read_path_abempty(std::istream &is);
optional<string> read_path_absolute(std::istream &is);
optional<string> read_path_noscheme(std::istream &is);
optional<string> read_path_rootless(std::istream &is);
optional<string> read_path_empty(std::istream &is);
optional<string> read_path(std::istream &is);
optional<string> read_port(std::istream &is);
optional<string> read_reg_name(std::istream &is);
optional<string> read_IPv4address(std::istream &is);
optional<string> read_dec_octet(std::istream &is);
optional<string> read_IPv6address(std::istream &is);
optional<string> read_ls32(std::istream &is);
optional<string> read_h16(std::istream &is);
optional<string> read_IP_literal(std::istream &is);
optional<string> read_IPvFuture(std::istream &is);
optional<string> read_host(std::istream &is);
optional<string> read_userinfo(std::istream &is);
optional<Authority> read_authority(std::istream &is);
optional<string> read_relative_part(std::istream &is);
optional<string> read_URI_reference(std::istream &is);
optional<string> read_fragment(std::istream &is);
optional<string> read_query(std::istream &is);
optional<string> read_hier_part(std::istream &is);
optional<Uri> read_URI(std::istream &is);
optional<string> read_absolute_URI(std::istream &is);
optional<string> read_relative_ref(std::istream &is);

}

#endif //RFC3986_HH_INCLUDED_20190201
