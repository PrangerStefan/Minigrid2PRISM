#pragma once

#include <iostream>
#include <functional>
#include "MinigridGrammar.h"
#include "PrismPrinter.h"
#include "ConfigYaml.h"

namespace prism {
  class PrismFormulaPrinter {
    public:
      PrismFormulaPrinter();

      std::ostream& printRestrictionFormula(std::ostream& os, const AgentName &agentName, const std::string &direction, const cells &grid_cells);
      std::ostream& printRestrictionFormulaWithCondition(std::ostream& os, const AgentName &agentName, const std::string &direction, const cells &grid_cells, const std::vector<std::string> conditions);
    private:
      std::string buildFormula(const std::string &formulaName, const std::string &formula);
      std::string buildLabel(const std::string &labelName, const std::string &label);
      std::string buildDisjunction(const AgentName &agentName, const cells &cells, const std::vector<std::string> &conditions = {});
  };
}
