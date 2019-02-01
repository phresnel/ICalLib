
#include "parser.hh"
#include "parser_helpers.hh"
#include <fstream>
#include <iostream>

int main() {
        std::ifstream f("dev-assets/f1calendar.com/2019_full.ics");
        try {
                expect_ical(f);
                std::cout << "done." << std::endl;
        } catch (syntax_error &e) {
                std::cerr << "syntax-error:" << e.what();
                print_location(e.pos, f);
        } catch (not_implemented &e) {
                std::cerr << "not-implemented:" << e.what();
                print_location(e.pos, f);
        } catch (std::exception &e) {
                std::cerr << "unknown error:" << e.what();
        }
        return 0;
}
