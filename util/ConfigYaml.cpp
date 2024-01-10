#include "ConfigYaml.h"
#include <iostream>

std::ostream& operator <<(std::ostream &os, const Label& label) {
    os << "\"" << label.label_ << "\"" << "=" << label.text_;
    return os;
}

std::ostream& operator << (std::ostream &os, const Formula& formula) {
    os << formula.formula_ << "=" << formula.content_;
    return os;
}

std::ostream& operator << (std::ostream& os, const Action& action) {
    os << action.action_;
    return os;
}

std::ostream& operator << (std::ostream& os, const Constant& constant) {
    os << "const " << constant.type_ << " " << constant.constant_ << " = " << constant.value_;
    return os;
}

std::ostream& operator << (std::ostream& os, const Module& module) {
    os << "Module: " << module.module_ << std::endl;
    for (auto& action : module.actions_) {
      os << action << std::endl;
    }
    return os;
}

std::string Label::createExpression() const {
    if (overwrite_) {
        return "label \"" + label_ + "\" = " + text_ + Configuration::overwrite_identifier_;
    } 

    return "label \"" + label_ + "\" = " + text_ + Configuration::configuration_identifier_;
}

std::string Formula::createExpression() const {
    if (overwrite_) {
        return "formula " + formula_ + " = " + content_ + Configuration::overwrite_identifier_;
    }

    return "formula " + formula_ + " = " + content_ + Configuration::configuration_identifier_;
}

std::string Action::createExpression() const {
    if (overwrite_) {
        return action_  + "\t" + guard_ + "-> " + update_  + Configuration::overwrite_identifier_;
    }
    
    return "\t" + action_  + "\t" + guard_ + "-> " + update_+ Configuration::configuration_identifier_;
}

std::string Constant::createExpression() const {
    if (overwrite_) {
        return "const " + type_ + " " + constant_ +  " = " + value_  + Configuration::overwrite_identifier_;
    }
    
    return "const " + type_ + " " + constant_ +  " = " + value_  + Configuration::configuration_identifier_;
}

YAML::Node YAML::convert<Module>::encode(const Module& rhs) {
    YAML::Node node;
    
    node.push_back(rhs.module_);
    node.push_back(rhs.actions_);

    return node;
}

bool YAML::convert<Module>::decode(const YAML::Node& node, Module& rhs) {
    if (!node.Type() == NodeType::Map) {
      return false;
    }
    rhs.actions_ = node["actions"].as<std::vector<Action>>();
    rhs.module_ = node["module"].as<std::string>();
    return true;
}

YAML::Node YAML::convert<Action>::encode(const Action& rhs) {
    YAML::Node node;

    node.push_back(rhs.action_);
    node.push_back(rhs.guard_);
    node.push_back(rhs.overwrite_);
    node.push_back(rhs.update_);

    return node;
}

bool YAML::convert<Action>::decode(const YAML::Node& node, Action& rhs) {
    if (!node.Type() == NodeType::Map) {
        return false;
    }

    rhs.action_ = node["action"].as<std::string>();
    rhs.guard_ = node["guard"].as<std::string>();
    rhs.update_ = node["update"].as<std::string>();

    if (node["overwrite"]) {
        rhs.overwrite_ = node["overwrite"].as<bool>();
    }
    if (node["index"]) {
        rhs.index_ = node["index"].as<int>();
    }

    return true;
}


YAML::Node YAML::convert<Label>::encode(const Label& rhs) {
    YAML::Node node;

    node.push_back(rhs.label_);
    node.push_back(rhs.text_);

    return node;
}

bool YAML::convert<Label>::decode(const YAML::Node& node, Label& rhs) {
    if (!node.Type() == NodeType::Map || !node["label"] || !node["text"]) {
        return false;
    }
    rhs.label_ = node["label"].as<std::string>();
    rhs.text_ = node["text"].as<std::string>();

    if (node["overwrite"]) {
        rhs.overwrite_ = node["overwrite"].as<bool>();
    }

    return true;
}

YAML::Node YAML::convert<Formula>::encode(const Formula& rhs) {
    YAML::Node node;

    node.push_back(rhs.content_);
    node.push_back(rhs.formula_);
    node.push_back(rhs.overwrite_);

    return node;
}

bool YAML::convert<Formula>::decode(const YAML::Node& node, Formula& rhs) {
    if (!node.IsDefined() || !node.Type() == NodeType::Map || !node["formula"] || !node["content"]) {
      return false;
    }

    rhs.formula_ = node["formula"].as<std::string>();
    rhs.content_ = node["content"].as<std::string>();

    if(node["overwrite"]) {
      rhs.overwrite_ = node["overwrite"].as<bool>();
    }

    return true;
}

YAML::Node YAML::convert<Constant>::encode(const Constant& rhs) {
    YAML::Node node;

    node.push_back(rhs.constant_);
    node.push_back(rhs.value_);
    node.push_back(rhs.type_);
    node.push_back(rhs.overwrite_);

    return node;
}

bool YAML::convert<Constant>::decode(const YAML::Node& node, Constant& rhs) {
   if (!node.IsDefined() || !node.Type() == NodeType::Map || !node["constant"] || !node["type"] || !node["value"]) {
      return false;
    }

    rhs.constant_ = node["constant"].as<std::string>();
    rhs.type_ = node["type"].as<std::string>();
    rhs.value_ = node["value"].as<std::string>();

    if(node["overwrite"]) {
      rhs.overwrite_ = node["overwrite"].as<bool>();
    }

    return true;
}

YAML::Node YAML::convert<Probability>::encode(const Probability& rhs) {
    YAML::Node node;
    
    node.push_back(rhs.probability_);
    node.push_back(rhs.value_);

    return node;
}

bool YAML::convert<Probability>::decode(const YAML::Node& node, Probability& rhs) {
    if (!node.IsDefined() || !node["probability"] || !node["value"]) {
        return false;
    }

    rhs.probability_ = node["probability"].as<std::string>();
    rhs.value_ = node["value"].as<double>();

    return true;
}

const std::string Configuration::configuration_identifier_ { "; // created through configuration"};
const std::string Configuration::overwrite_identifier_{"; // Overwritten through configuration"};

YamlConfigParseResult YamlConfigParser::parseConfiguration() {
        std::vector<Configuration> configuration;
        std::vector<Probability> probabilities;

        try {
            YAML::Node config = YAML::LoadFile(file_);  
            std::vector<Label> labels;
            std::vector<Formula> formulas;
            std::vector<Module> modules;
            std::vector<Constant> constants;

            if (config["labels"]) {
                labels = config["labels"].as<std::vector<Label>>();
            }
            if (config["formulas"]) {
                formulas = config["formulas"].as<std::vector<Formula>>();
            }
            if (config["modules"]) {
                modules = config["modules"].as<std::vector<Module>>();
            }

            if (config["constants"]) {
                constants = config["constants"].as<std::vector<Constant>>();
            }

            if (config["probabilities"]) {
                probabilities = config["probabilities"].as<std::vector<Probability>>();
            }
        
            for (auto& label : labels) {
                configuration.push_back({label.createExpression(), label.label_ , ConfigType::Label, label.overwrite_});
            }
            for (auto& formula : formulas) {
                configuration.push_back({formula.createExpression(), formula.formula_ ,ConfigType::Formula, formula.overwrite_});
            }
            for (auto& module : modules) {
                for (auto& action : module.actions_) {
                    configuration.push_back({action.createExpression(), action.action_, ConfigType::Module, action.overwrite_, module.module_, action.index_});
                }
            }
            for (auto& constant : constants) {
                // std::cout << constant.constant_ << std::endl;
                configuration.push_back({constant.createExpression(), "const " + constant.type_ + " " + constant.constant_, ConfigType::Constant, constant.overwrite_});
            }
        }
        catch(const std::exception& e) {
            std::cout << "Exception '" << typeid(e).name() << "' caught:" << std::endl;
            std::cout << "\t" << e.what() << std::endl;
            std::cout << "while parsing configuration " << file_ << std::endl;
        }

        return YamlConfigParseResult(configuration, probabilities);
}