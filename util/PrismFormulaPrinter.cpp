#include "PrismFormulaPrinter.h"

#include <map>
#include <string>


std::string cellToConjunction(const AgentName &agentName, const cell &c) {
  return "x" + agentName + "=" + std::to_string(c.column) + "&y" + agentName + "=" + std::to_string(c.row);
}

namespace prism {
  PrismFormulaPrinter::PrismFormulaPrinter() {}

  std::ostream& PrismFormulaPrinter::printRestrictionFormula(std::ostream& os, const AgentName &agentName, const std::string &direction, const cells &grid_cells) {
    os << buildFormula(agentName + "CannotMove" + direction, buildDisjunction(agentName, grid_cells));
    return os;
  }
  std::ostream& PrismFormulaPrinter::printRestrictionFormulaWithCondition(std::ostream& os, const AgentName &agentName, const std::string &direction, const cells &grid_cells, const std::vector<std::string> conditions) {
    os << buildFormula(agentName + "Something", buildDisjunction(agentName, grid_cells, conditions));
    return os;
  }

  std::string PrismFormulaPrinter::buildFormula(const std::string &formulaName, const std::string &formula) {
    return "formula " + formulaName + " = " + formula + ";\n";
  }

  std::string PrismFormulaPrinter::buildDisjunction(const AgentName &agentName, const cells &cells, const std::vector<std::string> &conditions) {
    bool first = true;
    std::string disjunction = "";
    if(!conditions.empty()) {
      for(uint index = 0; index < cells.size(); index++) {
        if(first) first = false;
        else disjunction += " | ";
        disjunction += "(" + cellToConjunction(agentName, cells.at(index)) + "&" + conditions.at(index) + ")";
      }

    } else {
      for(auto const cell : cells) {
        if(first) first = false;
        else disjunction += " | ";
        disjunction += "(" + cellToConjunction(agentName, cell) + ")";
      }
    }
    return disjunction;
  }
}
