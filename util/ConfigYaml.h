#pragma once

#include <vector>
#include <ostream>
#include <utility>

#include "yaml-cpp/yaml.h"


enum class ConfigType : char {
  Label = 'L',
  Formula = 'F',
  Module = 'M',
  Constant = 'C'
};

struct Configuration
{
  static const std::string overwrite_identifier_; 
  static const std::string configuration_identifier_; 

  std::string module_ {};
  std::string expression_{};
  std::string identifier_{};
  std::string end_identifier_{};  
  std::vector<int> indexes_{0};

  ConfigType type_ {ConfigType::Label};
  bool overwrite_ {false};
  bool include_identifier_for_overwrite_{true};

  Configuration() = default;
  Configuration(std::string expression
                , std::string identifier
                , ConfigType type
                , bool overwrite = false
                , std::string module = ""
                , std::vector<int> indexes = {0}
                , std::string end_identifier = {";"}
                , bool include_identifier_for_overwrite = true) : expression_(expression), identifier_(identifier), type_(type), overwrite_(overwrite), module_{module}, indexes_(indexes), end_identifier_{end_identifier}, include_identifier_for_overwrite_{include_identifier_for_overwrite}  {}
  
  ~Configuration() = default;
  Configuration(const Configuration&) = default;

  friend std::ostream& operator << (std::ostream& os, const Configuration& config) {
    os << "Configuration with Type: " << static_cast<char>(config.type_) << std::endl; 
    return os << "\tExpression=" << config.expression_ << std::endl;
  }
};

struct Probability {
  Probability() = default;
  Probability(const Probability&) = default;
  ~Probability() = default;

  std::string probability_;
  double value_; 

  friend std::ostream& operator <<(std::ostream& os, const Probability& property);
};

struct Constant {
  private:

  public:
  std::string constant_;
  std::string type_;
  std::string value_;
  bool overwrite_{false};

  std::string createExpression() const;

  friend std::ostream& operator <<(std::ostream &os, const Constant& constant);
};

struct Label {
  private:

  public:
  std::string text_;
  std::string label_;
  bool overwrite_{false};

  std::string createExpression() const;

  friend std::ostream& operator <<(std::ostream &os, const Label& label);
};

struct Formula {
  private:
  
  public:
  std::string formula_;
  std::string content_;
  bool overwrite_ {false};

  std::string createExpression() const;

  friend std::ostream& operator << (std::ostream &os, const Formula& formula);
};

struct Command {
  public:
  std::string action_;
  std::string guard_;
  std::string update_;
  std::vector<int> indexes_{0};
  bool overwrite_ {false};

  std::string createExpression() const;

  friend std::ostream& operator << (std::ostream& os, const Command& command);
};

struct Module {
  public:

  std::vector<Command> commands_;
  std::string module_;
  std::string module_text_;
  bool overwrite_module{false};

  friend std::ostream& operator << (std::ostream& os, const Module& module);
};


template<> 
struct YAML::convert<Module> {
  static YAML::Node encode(const Module& rhs);
  static bool decode(const YAML::Node& node, Module& rhs);
};

template<>
struct YAML::convert<Command> {
  static YAML::Node encode(const Command& rhs);
  static bool decode(const YAML::Node& node, Command& rhs);
};


template<>
struct YAML::convert<Label> {
  static YAML::Node encode(const Label& rhs);
  static bool decode(const YAML::Node& node, Label& rhs);
};

template<>
struct YAML::convert<Formula> {
  static YAML::Node encode(const Formula& rhs);
  static bool decode(const YAML::Node& node, Formula& rhs);
};

template<>
struct YAML::convert<Constant> {
  static YAML::Node encode(const Constant& rhs);
  static bool decode(const YAML::Node& node, Constant& rhs);
};

template<>
struct YAML::convert<Probability> {
  static YAML::Node encode(const Probability& rhs);
  static bool decode(const YAML::Node& node, Probability& rhs);
};

struct YamlConfigParseResult {
  YamlConfigParseResult(std::vector<Configuration> configurations, std::vector<Probability>  probabilities) 
    : configurations_(configurations), probabilities_(probabilities) {}

  ~YamlConfigParseResult() = default;
  YamlConfigParseResult(const YamlConfigParseResult&) = default;

  std::vector<Configuration> configurations_;
  std::vector<Probability>  probabilities_;
};

struct YamlConfigParser {
    public:
        YamlConfigParser(std::string file) : file_(file) {}
        YamlConfigParser(const YamlConfigParser&) = delete;
        ~YamlConfigParser() = default;

        YamlConfigParseResult parseConfiguration();
    private:

        std::string file_;
};
