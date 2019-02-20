
#include "IcalParser.hh"
#include "parser_exceptions.hh"
#include "icalstream.hh"
#include <fstream>
#include <iostream>
#include <sstream>

void read_file(std::string const &filename) {
        std::ifstream f(filename, std::ifstream::binary);
        if (!f.good()) {
                std::cerr << "error opening \"" << filename << "\"\n";
                return;
        }
        //std::stringstream ss; ss << f.rdbuf();
        try {
                IcalParser parser(f);
                auto ical = parser.expect_ical();
                std::cout << ical << std::endl;
        } catch (syntax_error &e) {
                std::cerr << "syntax-error:" << e.what();
                print_location(e.pos, f);
        } catch (not_implemented &e) {
                std::cerr << "not-implemented:" << e.what();
                print_location(e.pos, f);
        } catch (std::exception &e) {
                std::cerr << "unknown error:" << e.what();
        }
}

int main() {
        read_file("dev-assets/f1calendar.com/2019_full.ics");
        //read_file("dev-assets/mini1.ics");
        return 0;
}
