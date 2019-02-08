#ifndef PARSER_HELPERS_HH_INCLUDED_20190201
#define PARSER_HELPERS_HH_INCLUDED_20190201

#include <iostream> // should only need iosfwd here.
#include <vector>
#include <string>
#include <optional>

#include <sstream>

inline namespace parser_helpers {

using std::string;
using std::vector;
using std::optional;
using std::nullopt;

class CallStack {
public:
        class Entry {
        public:
                Entry() = delete;
                Entry(Entry const &) = delete;
                Entry& operator= (Entry const &) = delete;

                Entry(Entry &&other) {
                        entries_ = other.entries_;
                        other.entries_ = nullptr;
                }
                Entry& operator= (Entry &&other) {
                        entries_ = other.entries_;
                        other.entries_ = nullptr;
                }

                Entry(std::vector<const char*> &entries, const char *what)
                        : entries_(&entries)
                {
                        entries_->push_back(what);

                        /*
                        std::cerr << "  ";
                        for (auto i=0; i<entries_->size(); ++i)
                                std::cerr << "  ";
                        std::cerr << "entered " << what << "\n";
                        */
                        std::stringstream ss;
                        bool first = true;
                        for (auto &f : *entries_) {
                                if (!first) ss << " ";
                                ss << "[" << f << "]";
                                first = false;
                        }
                        std::cerr << shorten("[" + ss.str() + "]\n", 120);
                }
                ~Entry() {
                        if (entries_)
                                entries_->pop_back();
                }
        private:
                friend class CallStack;
                std::vector<const char*> *entries_;

                static std::string shorten(std::string const &v, int maxLen=8) {
                        if (v.size()<maxLen)
                                return v;
                        const auto rem = (v.size() - maxLen) + 3;
                        const auto h = (v.size() - rem) / 2;
                        const auto left = v.substr(0, h + !(maxLen%2));
                        const auto right = v.substr(v.size() - h);
                        return left + "..." + right;
                }
        };

        CallStack() = default;
        CallStack(CallStack const&) = default;
        CallStack& operator= (CallStack const&) = default;

        Entry push(const char* name) {
                Entry ret {entries_, name};
                //std::cerr << *this;
                return std::move(ret);
        }

        friend std::ostream& operator<< (
                std::ostream& os, CallStack const &cs
        ) {
                os << "CallStack{\n";
                for (auto i=0; i!=cs.entries_.size(); ++i) {
                        /*os << " ..";
                        for (auto x=0; x!=i; ++x) {
                                os << "..";
                        }*/
                        os << "  " << cs.entries_[i] << "\n";
                }
                os << "}\n";
                return os;
        }
private:

        std::vector<const char*> entries_;
};

extern CallStack callStack;

#if 0
#define CALLSTACK \
        print_location(is.tellg(), is);\
        CallStack::Entry \
                call_stack_entry_##__LINE__ = callStack.push(__func__);
#else
#define CALLSTACK
#endif

class not_implemented : public std::runtime_error {
public:
        not_implemented(std::istream::pos_type pos,
                        std::string const &what,
                        std::string const &file,
                        int line) :
                std::runtime_error(
                        what +
                        " [Line " + std::to_string(line) + ", " +
                        " File '" + file + "']"
                ),
                pos(pos)
        {}

        std::istream::pos_type pos = 0;
};
#define NOT_IMPLEMENTED \
        do{ \
                throw not_implemented( \
                        is.tellg(), __func__, __FILE__, __LINE__); \
        }while(false)

void print_location(std::istream::pos_type pos, std::istream &is);

// -- Utils. -------------------------------------------------------------------
class save_flags final {
        std::ios *s_;
        std::ios::fmtflags flags_;
public:
        explicit save_flags(std::ios &s) : s_(&s), flags_(s.flags()) { }
        ~save_flags() {
                if (s_ != nullptr) {
                        try {
                                s_->flags(flags_);
                        } catch(...) {
                                // must not throw here.
                        }
                }
        }

        save_flags(save_flags &&) = default;
        save_flags& operator= (save_flags &&) = default;

        save_flags() = delete;
        save_flags(save_flags const &) = delete;
        save_flags& operator= (save_flags const &) = delete;

        void commit() { s_ = nullptr; }
};

class save_input_pos final {
        std::istream *s_;
        std::istream::pos_type pos_;
public:
        explicit save_input_pos(std::istream &s) : s_(&s), pos_(s.tellg()) { }
        ~save_input_pos() {
                if (s_ != nullptr) {
                        try { s_->seekg(pos_); }
                        catch(...) { /* must not throw here. */ }
                }
        }

        save_input_pos(save_input_pos &&) = default;
        save_input_pos& operator= (save_input_pos &&) = default;

        save_input_pos() = delete;
        save_input_pos(save_input_pos const &) = delete;
        save_input_pos& operator= (save_input_pos const &) = delete;

        void commit() { s_ = nullptr; }
};

void dump_remainder(std::istream &is);
void dump_remainder_and_exit(std::istream &is);

// -- Parser Helpers. ----------------------------------------------------------
string expect_token(std::istream &is, string const &tok);
string expect_newline(std::istream &is);
string expect_alpha(std::istream &is);
string expect_digit(std::istream &is);
string expect_digit(std::istream &is, int min, int max);
string expect_alnum(std::istream &is);
optional<string> read_token(std::istream &is, string const &tok);
optional<string> read_eof(std::istream &is);
optional<string> read_newline(std::istream &is);
optional<string> read_hex(std::istream &is);
optional<string> read_alpha(std::istream &is);
optional<string> read_digit(std::istream &is);
optional<string> read_digit(std::istream &is, int min, int max);
optional<string> read_digits(std::istream &is, int at_least, int at_most);
optional<string> read_digits(std::istream &is, int num);
optional<string> read_alnum(std::istream &is);

}


#endif //PARSER_HELPERS_HH_INCLUDED_20190201
