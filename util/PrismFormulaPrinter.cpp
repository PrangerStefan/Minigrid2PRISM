#include "PrismFormulaPrinter.h"

#include <map>
#include <string>
#include <algorithm>

std::string vectorToDisjunction(const std::vector<std::string> &formulae) {
  bool first = true;
  std::string disjunction = "";
  for(const auto &formula : formulae) {
    if(first) first = false;
    else disjunction += " | ";
    disjunction += formula;
  }
  return disjunction;
}

std::string cellToConjunction(const AgentName &agentName, const cell &c) {
  return "x" + agentName + "=" + std::to_string(c.column) + "&y" + agentName + "=" + std::to_string(c.row);
}

std::string coordinatesToConjunction(const AgentName &agentName, const coordinates &c, const ViewDirection viewDirection) {
  return "x" + agentName + "=" + std::to_string(c.first) + "&y" + agentName + "=" + std::to_string(c.second) + "&view" + agentName + "=" + std::to_string(viewDirection);
}

std::map<ViewDirection, coordinates> getSurroundingCells(const cell &c) {
  return {{1, c.getNorth()}, {2, c.getEast()}, {3, c.getSouth()}, {0, c.getWest()}};
}

namespace prism {
  PrismFormulaPrinter::PrismFormulaPrinter(std::ostream &os, const AgentName &agentName, const std::map<std::string, cells> &restrictions, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const std::map<std::string, cells> &slipperyTiles, const cells &lava)
    : os(os), agentName(agentName), restrictions(restrictions), boxes(boxes), balls(balls), lockedDoors(lockedDoors), unlockedDoors(unlockedDoors), keys(keys), slipperyTiles(slipperyTiles), lava(lava)
  { }

  void PrismFormulaPrinter::print() {
    for(const auto& [direction, cells] : restrictions) {
      printRestrictionFormula(direction, cells);
    }

    for(const auto& [direction, cells] : slipperyTiles) {
      printIsOnFormula("Slippery", cells, direction);
    }
    printIsOnFormula("Lava", lava);

    for(const auto& ball : balls) {
      std::string color = capitalize(ball.getColor());
      printRestrictionFormulaWithCondition(color + "Ball", getSurroundingCells(ball), "!" + color + "BallPickedUp");
    }

    for(const auto& box : boxes) {
      std::string color = capitalize(box.getColor());
      printRestrictionFormulaWithCondition(color + "Box", getSurroundingCells(box), "!" + color + "BoxPickedUp");
    }

    for(const auto& key : keys) {
      std::string color = capitalize(key.getColor());
      printRestrictionFormulaWithCondition(color + "Key", getSurroundingCells(key), "!" + color + "KeyPickedUp");
    }

    for(const auto& door : unlockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      printRestrictionFormulaWithCondition(identifier, getSurroundingCells(door), "!" + identifier + "Open");
      printIsNextToFormula(identifier, getSurroundingCells(door));
    }

    for(const auto& door : lockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      printRestrictionFormulaWithCondition(identifier, getSurroundingCells(door), "!" + identifier + "Open");
      printIsNextToFormula(identifier, getSurroundingCells(door));
    }

    if(conditionalMovementRestrictions.size() > 0) {
      os << buildFormula(agentName + "CannotMoveConditionally", vectorToDisjunction(conditionalMovementRestrictions));
    }
  }

  void PrismFormulaPrinter::printRestrictionFormula(const std::string &direction, const cells &grid_cells) {
    os << buildFormula(agentName + "CannotMove" + direction + "Wall", buildDisjunction(agentName, grid_cells));
  }

  void PrismFormulaPrinter::printIsOnFormula(const std::string &type, const cells &grid_cells, const std::string &direction) {
    os << buildFormula(agentName + "IsOn" + type + direction, buildDisjunction(agentName, grid_cells));
  }

  void PrismFormulaPrinter::printIsNextToFormula(const std::string &type, const std::map<ViewDirection, coordinates> &coordinates) {
    os << buildFormula(agentName + "IsNextTo" + type, buildDisjunction(agentName, coordinates));
  }

  void PrismFormulaPrinter::printRestrictionFormulaWithCondition(const std::string &reason, const std::map<ViewDirection, coordinates> &coordinates, const std::string &condition) {
    os << buildFormula(agentName + "CannotMove" + reason, "(" + buildDisjunction(agentName, coordinates) + ") & " + condition);
    conditionalMovementRestrictions.push_back(agentName + "CannotMove" + reason);
  }

  std::string PrismFormulaPrinter::buildFormula(const std::string &formulaName, const std::string &formula) {
    return "formula " + formulaName + " = " + formula + ";\n";
  }

  std::string PrismFormulaPrinter::buildDisjunction(const AgentName &agentName, const std::map<ViewDirection, coordinates> &cells) {
    if(cells.size() == 0) return "false";
    bool first = true;
    std::string disjunction = "";
    for(const auto [viewDirection, coordinates] : cells) {
      if(first) first = false;
      else disjunction += " | ";
      disjunction += "(" + coordinatesToConjunction(agentName, coordinates, viewDirection) + ")";
    }
    return disjunction;
  }

  std::string PrismFormulaPrinter::buildDisjunction(const AgentName &agentName, const cells &cells, const std::vector<std::string> &conditions) {
    if(cells.size() == 0) return "false";
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
