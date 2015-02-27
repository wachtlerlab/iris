
#include <string>
#include <vector>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_ascii.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace csv {
namespace qi      = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii   = boost::spirit::ascii;

struct record {
    std::vector<std::string> fields;
};

template<typename Iter, typename Skip>
struct csv_grammar : qi::grammar<Iter, std::vector<std::string>(), Skip> {

    csv_grammar() : csv_grammar::base_type(rec) {
        using qi::alpha;
        using qi::lexeme;
        using ascii::char_;

        qstr %= lexeme['"' >> +(char_ - '"') >> '"'];
        str  %= +(char_ - (qi::eol | ',' | "\""));

        field = str | qstr;

        rec %= field >> *(',' >> field ) >> (qi::eol | qi::eoi);
    }

    qi::rule<Iter, std::string(), Skip> str;
    qi::rule<Iter, std::string(), Skip> qstr;
    qi::rule<Iter, std::string(), Skip> field;
    qi::rule<Iter, std::vector<std::string>(), Skip> rec;
};

};

int
main(int argc, char **argv) {

    std::string s("a, \"b d\", 3\n5, 6, 7");

    csv::csv_grammar<std::string::const_iterator, boost::spirit::qi::blank_type> g;

    std::vector<std::string> v;
    auto i = s.cbegin();

    while (i != s.cend()) {
        bool r = boost::spirit::qi::phrase_parse(i, s.cend(), g, boost::spirit::qi::blank, v);
        std::cout << "[";
        for (const auto &k : v) {
            std::cout << k << ", ";
        }

        std::cout << "]" << std::endl;
        v.clear();
    }
    return 0;
}