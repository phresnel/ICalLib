#ifndef PARSER_HELPERS_HH_INCLUDED_20190201
#define PARSER_HELPERS_HH_INCLUDED_20190201

#include <iostream> // should only need iosfwd here.
#include <vector>
#include <string>
#include <optional>

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

                        std::cerr << "  ";
                        for (auto i=0; i<entries_->size(); ++i)
                                std::cerr << "  ";
                        std::cerr << "entered " << what << "\n";
                }
                ~Entry() {
                        if (entries_)
                                entries_->pop_back();
                }
        private:
                friend class CallStack;
                std::vector<const char*> *entries_;
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

//CallStack callStack;
//#define CALLSTACK CallStack::Entry call_stack_entry_##__LINE__ = callStack.push(__func__);
//*/
#define CALLSTACK

class not_implemented : public std::runtime_error {
public:
        not_implemented(std::istream::pos_type pos, std::string const &what) :
                std::runtime_error("Not implemented: " + what), pos(pos)
        {}

        std::istream::pos_type pos = 0;
};
#define NOT_IMPLEMENTED do{throw not_implemented(is.tellg(), __func__);}while(false)

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

// -- Parser Helpers. ----------------------------------------------------------
string expect_token(std::istream &is, string const &tok);
string expect_newline(std::istream &is);
string expect_alpha(std::istream &is);
string expect_digit(std::istream &is);
string expect_alnum(std::istream &is);
optional<string> read_token(std::istream &is, string const &tok);
optional<string> read_eof(std::istream &is);
optional<string> read_newline(std::istream &is);
optional<string> read_hex(std::istream &is);
optional<string> read_alpha(std::istream &is);
optional<string> read_digit(std::istream &is);
optional<string> read_digits(std::istream &is, int at_least, int at_most = -1);
optional<string> read_alnum(std::istream &is);

}


#endif //PARSER_HELPERS_HH_INCLUDED_20190201