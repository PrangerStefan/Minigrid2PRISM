#include "PrismFormulaPrinter.h"

#include <map>
#include <string>
#include <algorithm>

std::string oneOffToString(const int &offset) {
  return offset != 0 ? ( offset == 1 ? "+1" : "-1" )  : "";
}

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

std::string objectPositionToConjunction(const AgentName &agentName, const std::string &identifier, const std::pair<int, int> &relativePosition, const ViewDirection viewDirection) {
  std::string xOffset = oneOffToString(relativePosition.first);
  std::string yOffset = oneOffToString(relativePosition.second);
  return "x" + agentName + xOffset + "=x" + identifier + "&y" + agentName + yOffset + "=y" + identifier + "&view" + agentName + "=" + std::to_string(viewDirection);
}

std::map<ViewDirection, coordinates> getSurroundingCells(const cell &c) {
  return {{1, c.getNorth()}, {2, c.getEast()}, {3, c.getSouth()}, {0, c.getWest()}};
}

std::map<ViewDirection, std::pair<int, int>> getRelativeSurroundingCells() {
  return { {1, {0,+1}}, {2, {-1,0}}, {3, {0,-1}}, {0, {+1,0}} };
}

namespace prism {
  PrismFormulaPrinter::PrismFormulaPrinter(std::ostream &os, const std::map<std::string, cells> &restrictions, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const std::map<std::string, cells> &slipperyTiles, const cells &lava, const cells &goals)
    : os(os),  restrictions(restrictions), boxes(boxes), balls(balls), lockedDoors(lockedDoors), unlockedDoors(unlockedDoors), keys(keys), slipperyTiles(slipperyTiles), lava(lava), goals(goals)
  { }

  void PrismFormulaPrinter::print(const AgentName &agentName) {
    for(const auto& [direction, cells] : restrictions) {
      printRestrictionFormula(agentName, direction, cells);
    }

    for(const auto& [direction, cells] : slipperyTiles) {
      printIsOnFormula(agentName, "Slippery", cells, direction);
    }
    std::vector<std::string> allSlipperyDirections = {agentName + "IsOnSlipperyNorth", agentName + "IsOnSlipperyEast", agentName + "IsOnSlipperySouth", agentName + "IsOnSlipperyWest"};
    os << buildFormula(agentName + "IsOnSlippery", vectorToDisjunction(allSlipperyDirections));
    printIsOnFormula(agentName, "Lava", lava);
    printIsOnFormula(agentName, "Goal", goals);

    for(const auto& ball : balls) {
      std::string identifier = capitalize(ball.getColor()) + ball.getType();
      printRelativeRestrictionFormulaWithCondition(agentName, identifier, "!" + identifier + "PickedUp");
      portableObjects.push_back(agentName + "Carrying" + identifier);
      for(auto const c : getSurroundingCells(ball)) {
        std::cout << ball << std::endl;
        std::cout << "dir:" << c.first << " column" << c.second.first << " row" << c.second.second << std::endl;

      }
    }

    for(const auto& box : boxes) {
      std::string identifier = capitalize(box.getColor()) + box.getType();
      printRelativeRestrictionFormulaWithCondition(agentName, identifier, "!" + identifier + "PickedUp");
      portableObjects.push_back(agentName + "Carrying" + identifier);
    }

    for(const auto& key : keys) {
      std::string identifier = capitalize(key.getColor()) + key.getType();
      printRelativeRestrictionFormulaWithCondition(agentName, identifier, "!" + identifier + "PickedUp");
      portableObjects.push_back(agentName + "Carrying" + identifier);
    }

    for(const auto& door : unlockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      printRestrictionFormulaWithCondition(agentName, identifier, getSurroundingCells(door), "!" + identifier + "Open");
      printIsNextToFormula(agentName, identifier, getSurroundingCells(door));
    }

    for(const auto& door : lockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      printRestrictionFormulaWithCondition(agentName, identifier, getSurroundingCells(door), "!" + identifier + "Open");
      printIsNextToFormula(agentName, identifier, getSurroundingCells(door));
    }

    if(conditionalMovementRestrictions.size() > 0) {
      os << buildFormula(agentName + "CannotMoveConditionally", vectorToDisjunction(conditionalMovementRestrictions));
      os << buildFormula(agentName + "IsCarrying", vectorToDisjunction(portableObjects));
    }
  }

  void PrismFormulaPrinter::printRestrictionFormula(const AgentName &agentName, const std::string &direction, const cells &grid_cells) {
    os << buildFormula(agentName + "CannotMove" + direction + "Wall", buildDisjunction(agentName, grid_cells));
  }

  void PrismFormulaPrinter::printIsOnFormula(const AgentName &agentName, const std::string &type, const cells &grid_cells, const std::string &direction) {
    os << buildFormula(agentName + "IsOn" + type + direction, buildDisjunction(agentName, grid_cells));
  }

  void PrismFormulaPrinter::printIsNextToFormula(const AgentName &agentName, const std::string &type, const std::map<ViewDirection, coordinates> &coordinates) {
    os << buildFormula(agentName + "IsNextTo" + type, buildDisjunction(agentName, coordinates));
  }

  void PrismFormulaPrinter::printRestrictionFormulaWithCondition(const AgentName &agentName, const std::string &reason, const std::map<ViewDirection, coordinates> &coordinates, const std::string &condition) {
    os << buildFormula(agentName + "CannotMove" + reason, "(" + buildDisjunction(agentName, coordinates) + ") & " + condition);
    conditionalMovementRestrictions.push_back(agentName + "CannotMove" + reason);
  }

  void PrismFormulaPrinter::printRelativeRestrictionFormulaWithCondition(const AgentName &agentName, const std::string &reason, const std::string &condition) {
    os << buildFormula(agentName + "CannotMove" + reason, "(" + buildDisjunction(agentName, reason) + ") & " + condition);
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

  std::string PrismFormulaPrinter::buildDisjunction(const AgentName &agentName, const std::string &reason) {
    std::string disjunction = "";
    bool first = true;
    for(auto const [viewDirection, relativePosition] : getRelativeSurroundingCells()) {
      if(first) first = false;
      else disjunction += " | ";
      disjunction += "(" + objectPositionToConjunction(agentName, reason, relativePosition, viewDirection) + ")";
    }
    return disjunction;
  }
}
