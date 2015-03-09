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
    comment_tag_() : text() { }
    comment_tag_(const std::string &s) : text(s) { }

    std::string text;
};

class record {
public:

    record() : data(), comment(false) { }
    record(const std::vector<std::string> &v) : data(v), comment(false) { }
    record(const std::string &v) : data({v}) { }
    record(const record &other) : data(other.data), comment(other.comment) { }
    record(const comment_tag_ &tag) : data({tag.text}), comment(true) { }

    record &operator=(const record &other) {
        if (&other == this) {
            return *this;
        }
        this->data = other.data;
        this->comment = other.comment;
        return *this;
    }

    bool is_empty() const { return data.empty(); };
    bool is_comment() const { return comment; };
    size_t nfields() const { return data.size(); };

    std::vector<std::string> data;
    bool comment = false;
};

namespace qi      = boost::spirit::qi;
namespace phoenix = boost::phoenix;
namespace ascii   = boost::spirit::ascii;


template<typename Iter, typename Skip>
struct csv_grammar : boost::spirit::qi::grammar<Iter, record(), Skip> {

    csv_grammar(const char d) : csv_grammar::base_type(rec), delimiter(d) {
        using qi::alpha;
        using qi::lexeme;
        using qi::lit;
        using qi::eps;
        using qi::_val;
        using qi::_1;
        using ascii::char_;

        text_data = lexeme[*(char_ - (qi::eol | qi::eoi))];

        qstr = lexeme['"' >> +(char_ - '"') >> '"'];
        str = +(char_ - (qi::eol | delimiter | "\""));

        field = str | qstr;
        fields = field >> *(delimiter >> field);

        comment = lit('#') >> text_data;

        rec = -(comment | fields) >> (qi::eol | qi::eoi);
    }

    const char delimiter;

    qi::rule<Iter, std::string(), Skip> text_data;
    qi::rule<Iter, iris::csv::comment_tag_(), Skip> comment;
    qi::rule<Iter, std::string(), Skip> str;
    qi::rule<Iter, std::string(), Skip> qstr;
    qi::rule<Iter, std::string(), Skip> field;
    qi::rule<Iter, std::vector<std::string>(), Skip> fields;
    qi::rule<Iter, iris::csv::record(), Skip> rec;
};

template<typename Iterator>
struct ws_skipper : public qi::grammar<Iterator> {

    ws_skipper() : ws_skipper::base_type(skip) {
        using qi::lit;
        skip = lit(' ') | lit('\r');
    }
    qi::rule<Iterator> skip;
};
} //iris::csv

template<typename Iterator>
class csv_iterator {
public:
    typedef iris::csv::csv_grammar<Iterator, csv::ws_skipper<Iterator>> grammar_type;
    typedef csv_iterator<Iterator> iter_type;
    typedef csv::record value_type;
    typedef ptrdiff_t difference_type;
    typedef const value_type *pointer;
    typedef const value_type &reference;
    typedef std::input_iterator_tag iterator_category;

    csv_iterator() : valid_result(false), pos(), last(), r(), grammar(',') { }

    csv_iterator(Iterator first, Iterator last, const char delimiter = ',')
            : valid_result(false), pos(first), last(last), r(), grammar(delimiter) {
        next_record();
    }

    csv_iterator(const csv_iterator &other) :
            valid_result(other.valid_result), pos(other.pos), last(other.last),
            r(other.r), grammar(other.grammar.delimiter) { }

    iter_type &operator++() {
        next_record();
        return *this;
    }

    iter_type &operator++(int) {
        iter_type tmp(*this);
        ++(*this);
        return tmp;
    }

    const csv::record &operator*() const { return r; }

    const csv::record *operator->() const { return &r; }

    bool operator==(const iter_type &other) {
        if (!valid_result && !other.valid_result) {
            return true;
        } else if (!valid_result || !other.valid_result) {
            return false;
        } else {
            return last == other.last && pos == other.pos;
        }
    }

    bool operator!=(const iter_type &other) {
        return !(*this == other);
    }

private:
    void next_record() {
        if (pos == last) {
            valid_result = false;
        } else {
            r = csv::record();
            csv::ws_skipper<Iterator> skipper{};
            valid_result = boost::spirit::qi::phrase_parse(pos, last, grammar, skipper, r);
        }
    }

private:
    bool valid_result;
    Iterator pos;
    Iterator last;
    csv::record r;
    grammar_type grammar;
};

class csv_file {
    typedef boost::spirit::istream_iterator internal_iter;
    typedef std::fstream fstream_type;
public:
    typedef csv_iterator<internal_iter> iterator;

    csv_file(const std::string &path) : ifs(path, std::ios::in), delimiter('\t') {
        ifs >> std::noskipws;
    }

    iterator begin() {
        boost::spirit::istream_iterator f(ifs), l;
        return iterator(f, l, delimiter);
    }

    iterator end() {
        return iterator();
    }

    const char detect_delim(const std::string dknown = ",;\t") {

        auto pos = ifs.tellg();
        ifs.clear();
        ifs.seekg(0, std::ios::beg);

        std::vector<size_t> dcount(dknown.size(), 0);

        for (std::string line; std::getline(ifs, line);) {
            for(size_t i = 0; i < dknown.size(); i++) {
                dcount[i] += std::count(line.begin(), line.end(), dknown[i]);
            }
        }

        auto imax = std::max_element(dcount.begin(), dcount.end());
        auto p = std::distance(dcount.begin(), imax);

        ifs.clear();
        ifs.seekg(pos);

        return dknown[p];
    }

private:
    fstream_type ifs;
    fstream_type::char_type delimiter;
};

} // iris::

#endif