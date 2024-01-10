#include "util/OptionParser.h"
#include "util/MinigridGrammar.h"
#include "util/Grid.h"
#include "util/ConfigYaml.h"

#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>


std::vector<std::string> parseCommaSeparatedString(std::string const& str) {
  std::vector<std::string> result;
  std::stringstream stream(str);
  while(stream.good()) {
    std::string substr;
    getline(stream, substr, ',');
    substr.at(0) = std::toupper(substr.at(0));
    result.push_back(substr);
  }
  return result;
}


struct printer {
    typedef boost::spirit::utf8_string string;

    void element(string const& tag, string const& value, int depth) const {
        for (int i = 0; i < (depth*4); ++i) std::cout << ' ';

        std::cout << "tag: " << tag;
        if (value != "") std::cout << ", value: " << value;
        std::cout << std::endl;
    }
};

void print_info(boost::spirit::info const& what) {
  using boost::spirit::basic_info_walker;

  printer pr;
  basic_info_walker<printer> walker(pr, what.tag, 0);
  boost::apply_visitor(walker, what.value);
}

int main(int argc, char* argv[]) {
  popl::OptionParser optionParser("Allowed options");

  auto helpOption = optionParser.add<popl::Switch>("h", "help", "Print this help message.");
  auto inputFilename = optionParser.add<popl::Value<std::string>>("i", "input-file", "Filename of the input file.");
  auto outputFilename = optionParser.add<popl::Value<std::string>>("o", "output-file", "Filename for the output file.");
  auto configFilename = optionParser.add<popl::Value<std::string>, popl::Attribute::optional>("c", "config-file", "Filename of the predicate configuration file.");


  try {
    optionParser.parse(argc, argv);

    if(helpOption->count() > 0) {
      std::cout << optionParser << std::endl;
      return EXIT_SUCCESS;
    }
  } catch (const popl::invalid_option &e) {
    return io::printPoplException(e);
  } catch (const std::exception &e) {
		std::cerr << "Exception: " << e.what() << "\n";
		return EXIT_FAILURE;
	}

  GridOptions gridOptions = { {}, {} };
  std::fstream file {outputFilename->value(0), file.trunc | file.out};
  std::fstream infile {inputFilename->value(0), infile.in};
  std::string line, content, background, rewards, properties;
  std::cout << "\n";
  bool parsingBackground = false;
  bool parsingStateRewards = false;
  bool parsingEnvironmentProperties = false;
  while (std::getline(infile, line) && !line.empty()) {
    if(line.at(0) == '-' && line.at(line.size() - 1) == '-' && parsingBackground) {
      parsingStateRewards = true;
      parsingBackground = false;
      continue;
    } else if (line.at(0) == '-' && line.at(line.size() - 1 ) == '-' && parsingStateRewards) {
      parsingStateRewards = false;
      parsingEnvironmentProperties = true;
      continue;
    } else if(line.at(0) == '-' && line.at(line.size() - 1) == '-') {
      parsingBackground = true;
      continue;
    }
    if(!parsingBackground && !parsingStateRewards && !parsingEnvironmentProperties) {
      content += line + "\n";
    } else if (parsingBackground) {
      background += line + "\n";
    } else if(parsingStateRewards) {
      rewards += line + "\n";
    } else if (parsingEnvironmentProperties) {
      properties += line + "\n";
    }
  }
  std::cout << "\n";

  pos_iterator_t contentFirst(content.begin());
  pos_iterator_t contentIter = contentFirst;
  pos_iterator_t contentLast(content.end());
  MinigridParser<pos_iterator_t> contentParser(contentFirst);
  pos_iterator_t backgroundFirst(background.begin());
  pos_iterator_t backgroundIter = backgroundFirst;
  pos_iterator_t backgroundLast(background.end());
  MinigridParser<pos_iterator_t> backgroundParser(backgroundFirst);

  cells contentCells;
  cells backgroundCells;
  std::vector<Configuration> configurations;
  std::map<coordinates, float> stateRewards;
  float faultyProbability = 0.1;
  float probIntended = 0.9;

  try {
    bool ok = phrase_parse(contentIter, contentLast, contentParser, qi::space, contentCells);
    // TODO if(background is not empty) {
    ok     &= phrase_parse(backgroundIter, backgroundLast, backgroundParser, qi::space, backgroundCells);
    // TODO }
    if (configFilename->is_set()) {
      YamlConfigParser parser(configFilename->value(0));
      configurations = parser.parseConfiguration();
    }

    boost::escaped_list_separator<char> seps('\\', ';', '\n');
    Tokenizer csvParser(rewards, seps);
    for(auto iter = csvParser.begin(); iter != csvParser.end(); ++iter) {
      int x = std::stoi(*iter);
      int y = std::stoi(*(++iter));
      float reward = std::stof(*(++iter));
      stateRewards[std::make_pair(x,y)] = reward;
    }
    if (!properties.empty()) {
      auto faultProbabilityIdentifier = std::string("FaultProbability:");
      auto start_pos = properties.find(faultProbabilityIdentifier);


      if (start_pos != std::string::npos) {
        auto end_pos = properties.find('\n', start_pos);
        auto value = properties.substr(start_pos + faultProbabilityIdentifier.length(), end_pos - start_pos - faultProbabilityIdentifier.length());
        faultyProbability = std::stod(value);
      }
    }
    if(ok) {
      Grid grid(contentCells, backgroundCells, gridOptions, stateRewards, probIntended, faultyProbability);

      grid.printToPrism(std::cout, configurations , gridOptions.getModelType());
      std::stringstream ss;
      // grid.printToPrism(file, configurations ,prism::ModelType::MDP);
      grid.printToPrism(ss, configurations , gridOptions.getModelType());
      std::string str = ss.str();
      grid.applyOverwrites(str, configurations);
      file << str;
    }
  } catch(qi::expectation_failure<pos_iterator_t> const& e) {
    std::cout << "expected: "; print_info(e.what_);
    std::cout << "got: \"" << std::string(e.first, e.last) << '"' << std::endl;
    std::cout << "Expectation failure: " << e.what() << " at '" << std::string(e.first,e.last) << "'\n";
  } catch(const std::exception& e) {
    std::cerr << "Exception '" << typeid(e).name() << "' caught:" << std::endl;
    std::cerr << "\t" << e.what() << std::endl;
    std::exit(EXIT_FAILURE);
  }

  return 0;
}
