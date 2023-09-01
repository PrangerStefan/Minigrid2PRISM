#pragma once

#include <vector>
#include <ostream>

#include "yaml-cpp/yaml.h"

typedef std::string expressions;

enum class ConfigType : char {
  Label = 'L',
  Formula = 'F',
  Module = 'M'
};

struct Configuration
{
  expressions expressions_;
  std::string derivation_;
  ConfigType type_ {ConfigType::Label};
  bool overwrite_;

  Configuration() = default;
  Configuration(std::string expression, std::string derivation, ConfigType type, bool overwrite = false) : expressions_(expression), derivation_(derivation), type_(type), overwrite_(overwrite) {}
  ~Configuration() = default;
  Configuration(const Configuration&) = default;

  friend std::ostream& operator << (std::ostream& os, const Configuration& config) {
    os << "Configuration with Type: " << static_cast<char>(config.type_) << std::endl; 
    os << "\tExpression=" << config.expressions_ << std::endl;
    return os << "\tDerviation=" << config.derivation_;
  }
};



struct Label {
  private:

  public:
  std::string text_;
  std::string label_;
  bool overwrite_;

  friend std::ostream& operator <<(std::ostream &os, const Label& label);
};

struct Formula {
  private:
  
  public:
  std::string formula_;
  std::string content_;
  bool overwrite_;

  friend std::ostream& operator << (std::ostream &os, const Formula& formula);
};

struct Action {
  public:
  std::string action_;
  std::string guard_;
  std::string update_;
  bool overwrite_;

  friend std::ostream& operator << (std::ostream& os, const Action& action);
};

struct Module {
  public:

  std::vector<Action> actions_;
  std::string module_;

  friend std::ostream& operator << (std::ostream& os, const Module& module);
};


template<> 
struct YAML::convert<Module> {
  static YAML::Node encode(const Module& rhs);
  static bool decode(const YAML::Node& node, Module& rhs);
};

template<>
struct YAML::convert<Action> {
  static YAML::Node encode(const Action& rhs);
  static bool decode(const YAML::Node& node, Action& rhs);
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


struct YamlConfigParser {
    public:
        YamlConfigParser(std::string file) : file_(file) {}
        YamlConfigParser(const YamlConfigParser&) = delete;
        ~YamlConfigParser() = default;

        std::vector<Configuration> parseConfiguration();
    private:

        std::string file_;
};