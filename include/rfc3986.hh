#ifndef RFC3986_HH_INCLUDED_20190201
#define RFC3986_HH_INCLUDED_20190201

#include <iosfwd>
#include <string>
#include <optional>

// -- URI (RFC 3986) Parser Helpers. ------------------------------------

inline namespace rfc3986 {

using std::string;
using std::optional;
using std::nullopt;

//      gen-delims  = ":" / "/" / "?" / "#" / "[" / "]" / "@"
optional<string> read_gen_delims(std::istream &is);

//      sub-delims  = "!" / "$" / "&" / "'" / "(" / ")"
//                  / "*" / "+" / "," / ";" / "="
optional<string> read_sub_delims(std::istream &is);

//      reserved    = gen-delims / sub-delims
optional<string> read_reserved(std::istream &is);

//      unreserved  = ALPHA / DIGIT / "-" / "." / "_" / "~"
optional<string> read_unreserved(std::istream &is);

// pct-encoded = "%" HEXDIG HEXDIG
optional<string> read_pct_encoded(std::istream &is);

//      pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
optional<string> read_pchar(std::istream &is);

//      segment       = *pchar
optional<string> read_segment(std::istream &is);

//      segment-nz    = 1*pchar
optional<string> read_segment_nz(std::istream &is);

//      segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//                    ; non-zero-length segment without any colon ":"
optional<string> read_segment_nz_nc_single(std::istream &is);

optional<string> read_segment_nz_nc(std::istream &is);

//      path-abempty  = *( "/" segment )
optional<string> read_path_abempty(std::istream &is);

//      path-absolute = "/" [ segment-nz *( "/" segment ) ]
optional<string> read_path_absolute(std::istream &is);

//      path-noscheme = segment-nz-nc *( "/" segment )
optional<string> read_path_noscheme(std::istream &is);

//      path-rootless = segment-nz *( "/" segment )
optional<string> read_path_rootless(std::istream &is);

//      path-empty    = 0<pchar>
optional<string> read_path_empty(std::istream &is);

//      path          = path-abempty    ; begins with "/" or is empty
//                    / path-absolute   ; begins with "/" but not "//"
//                    / path-noscheme   ; begins with a non-colon segment
//                    / path-rootless   ; begins with a segment
//                    / path-empty      ; zero characters
optional<string> read_path(std::istream &is);

//      port        = *DIGIT
optional<string> read_port(std::istream &is);

//      reg-name    = *( unreserved / pct-encoded / sub-delims )
optional<string> read_reg_name(std::istream &is);

//      IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
optional<string> read_IPv4address(std::istream &is);

//      dec-octet   = DIGIT                 ; 0-9
//                  / %x31-39 DIGIT         ; 10-99
//                  / "1" 2DIGIT            ; 100-199
//                  / "2" %x30-34 DIGIT     ; 200-249
//                  / "25" %x30-35          ; 250-255
optional<string> read_dec_octet(std::istream &is);

//      IPv6address =                            6( h16 ":" ) ls32
//                  /                       "::" 5( h16 ":" ) ls32
//                  / [               h16 ] "::" 4( h16 ":" ) ls32
//                  / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
//                  / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
//                  / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
//                  / [ *4( h16 ":" ) h16 ] "::"              ls32
//                  / [ *5( h16 ":" ) h16 ] "::"              h16
//                  / [ *6( h16 ":" ) h16 ] "::"
optional<string> read_IPv6address(std::istream &is);

//
//      ls32        = ( h16 ":" h16 ) / IPv4address
//                  ; least-significant 32 bits of address
optional<string> read_ls32(std::istream &is);

//
//      h16         = 1*4HEXDIG
//                  ; 16 bits of address represented in hexadecimal
optional<string> read_h16(std::istream &is);

//      IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
optional<string> read_IP_literal(std::istream &is);

//      IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
optional<string> read_IPvFuture(std::istream &is);

//      host        = IP-literal / IPv4address / reg-name
optional<string> read_host(std::istream &is);

//      userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )
optional<string> read_userinfo(std::istream &is);

//      authority   = [ userinfo "@" ] host [ ":" port ]
optional<string> read_authority(std::istream &is);

//      relative-part = "//" authority path-abempty
//                    / path-absolute
//                    / path-noscheme
//                    / path-empty
optional<string> read_relative_part(std::istream &is);

//      URI-reference = URI / relative-ref
optional<string> read_URI_reference(std::istream &is);

//      fragment    = *( pchar / "/" / "?" )
optional<string> read_fragment(std::istream &is);

//      query       = *( pchar / "/" / "?" )
optional<string> read_query(std::istream &is);

//      hier-part   = "//" authority path-abempty
//                  / path-absolute
//                  / path-rootless
//                  / path-empty
optional<string> read_hier_part(std::istream &is);

//      URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
optional<string> read_URI(std::istream &is);

//      absolute-URI  = scheme ":" hier-part [ "?" query ]
optional<string> read_absolute_URI(std::istream &is);

//      relative-ref  = relative-part [ "?" query ] [ "#" fragment ]
optional<string> read_relative_ref(std::istream &is);

}

#endif //RFC3986_HH_INCLUDED_20190201
