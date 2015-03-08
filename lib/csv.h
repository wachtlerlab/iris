#ifndef IRIS_CSV_H
#define IRIS_CSV_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_ascii.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

#include <cstddef>
#include <string>
#include <vector>
#include <fstream>

namespace iris {
namespace csv {

struct comment_tag_ {
    comment_tag_() : text() {}
    comment_tag_(const std::string &s) : text(s) {}
    std::string text;
};

class record {
public:

    record(const std::vector<std::string> &v) : data(v), comment(false) { }
    record(): data(), comment(false) { }
    record(const std::string &v): data({v}) { }
    record(const record &other) : data(other.data), comment(other.comment) { }
    record(const comment_tag_ &tag) : data({tag.text}), comment(true) { }

    record &operator=(const record &other) {
        if (&other == this) {
            return *this;
        }
        std::cout << "[c =]" << std::endl;
        this->data = other.data;
        this->comment = other.comment;
        return *this;
    }

    bool is_empty() const {
        return data.empty();
    };

    bool is_comment() const {
        return comment;
    };

    size_t nfields() const {
        return data.size();
    };


    std::vector<std::string> data;
    bool comment = false;
};

namespace qi      = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii   = boost::spirit::ascii;


template<typename Iter, typename Skip>
struct csv_grammar : boost::spirit::qi::grammar<Iter, record(), Skip> {


    csv_grammar() : csv_grammar::base_type(rec) {


        using qi::alpha;
        using qi::lexeme;
        using qi::lit;
        using qi::eps;
        using qi::_val;
        using qi::_1;
        using ascii::char_;

        text_data = lexeme[*(char_ - (qi::eol | qi::eoi))];

        qstr = lexeme['"' >> +(char_ - '"') >> '"'];
        str  = +(char_ - (qi::eol | ',' | "\""));

        field = str | qstr;
        fields = field >> *(',' >> field );

        comment = lit('#') >> text_data;

        rec = -(comment | fields) >> (qi::eol | qi::eoi);
    }

    qi::rule<Iter, std::string(), Skip> text_data;
    qi::rule<Iter, iris::csv::comment_tag_(), Skip> comment;
    qi::rule<Iter, std::string(), Skip> str;
    qi::rule<Iter, std::string(), Skip> qstr;
    qi::rule<Iter, std::string(), Skip> field;
    qi::rule<Iter, std::vector<std::string>(), Skip> fields;
    qi::rule<Iter, iris::csv::record(), Skip> rec;
};

} //iris::cvs
} //iris::

#endif