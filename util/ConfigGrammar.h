#pragma once

#include "cell.h"

#include <vector>

#include <boost/tokenizer.hpp>
#include <boost/fusion/adapted/struct.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/variant/recursive_wrapper.hpp>
#include <boost/spirit/include/support_line_pos_iterator.hpp>

namespace qi      = boost::spirit::qi;
namespace phoenix = boost::phoenix;

typedef std::vector<std::string> expressions;

struct Configuration
{
  expressions expressions_;
  std::string derivation_;

  Configuration() = default;
  Configuration(expressions expressions, std::string derivation) : expressions_(expressions), derivation_(derivation) {}
  ~Configuration() = default;
  Configuration(const Configuration&) = default;

  friend std::ostream& operator << (std::ostream& os, const Configuration& config) {
    os << "Configuration" << std::endl; 

    for (auto& expression : config.expressions_) {
      os << "Expression=" << expression << std::endl;
    }

    return os << "Derviation=" << config.derivation_;
  }
};


BOOST_FUSION_ADAPT_STRUCT(
    Configuration,
    (expressions, expressions_)
    (std::string, derivation_)
)


template <typename It>
    struct ConfigParser : qi::grammar<It, std::vector<Configuration>>
{
  ConfigParser(It first) : ConfigParser::base_type(config_)
  {
    using namespace qi;
   
    expression_ = +char_("a-zA-Z_0-9");
    expressions_ = (expression_ % ',');
    row_ = (expressions_ > ';' > expression_);
    config_ = (row_ % "\n");

    BOOST_SPIRIT_DEBUG_NODE(expression_);
    BOOST_SPIRIT_DEBUG_NODE(expressions_);
    BOOST_SPIRIT_DEBUG_NODE(config_);
  }

  private:
  
    qi::rule<It, expressions()> expressions_;
    qi::rule<It, std::string()> expression_;
    qi::rule<It, Configuration()> row_;
    qi::rule<It, std::vector<Configuration>>  config_;
};