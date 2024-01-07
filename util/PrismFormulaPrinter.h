#pragma once

#include <iostream>
#include <functional>
#include "MinigridGrammar.h"
#include "PrismPrinter.h"
#include "ConfigYaml.h"


std::string oneOffToString(const int &offset);
std::string vectorToDisjunction(const std::vector<std::string> &formulae);
std::string cellToConjunction(const AgentName &agentName, const cell &c);
std::string coordinatesToConjunction(const AgentName &agentName, const coordinates &c, const ViewDirection viewDirection);
std::string objectPositionToConjunction(const AgentName &agentName, const std::string &identifier, const std::pair<int, int> &relativePosition, const ViewDirection viewDirection);
std::map<ViewDirection, coordinates> getSurroundingCells(const cell &c);
std::map<ViewDirection, std::pair<int, int>> getRelativeSurroundingCells();

namespace prism {
  class PrismFormulaPrinter {
    public:
      PrismFormulaPrinter(std::ostream &os, const std::map<std::string, cells> &restrictions, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const std::map<std::string, cells> &slipperyTiles, const cells &lava, const cells &goals);

      void print(const AgentName &agentName);

      void printRestrictionFormula(const AgentName &agentName, const std::string &direction, const cells &grid_cells);
      void printIsOnFormula(const AgentName &agentName, const std::string &type, const cells &grid_cells, const std::string &direction = "");
      void printIsNextToFormula(const AgentName &agentName, const std::string &type, const std::map<ViewDirection, coordinates> &coordinates);
      void printRestrictionFormulaWithCondition(const AgentName &agentName, const std::string &reason, const std::map<ViewDirection, coordinates> &coordinates, const std::string &condition);
      void printRelativeRestrictionFormulaWithCondition(const AgentName &agentName, const std::string &reason, const std::string &condition);
    private:
      std::string buildFormula(const std::string &formulaName, const std::string &formula);
      std::string buildLabel(const std::string &labelName, const std::string &label);
      std::string buildDisjunction(const AgentName &agentName, const std::map<ViewDirection, coordinates> &cells);
      std::string buildDisjunction(const AgentName &agentName, const cells &cells, const std::vector<std::string> &conditions = {});
      std::string buildDisjunction(const AgentName &agentName, const std::string &reason);

      std::ostream &os;
      std::map<std::string, cells> restrictions;
      cells boxes;
      cells balls;
      cells lockedDoors;
      cells unlockedDoors;
      cells keys;
      std::map<std::string, cells> slipperyTiles;
      cells lava;
      cells goals;

      std::vector<std::string> conditionalMovementRestrictions;
      std::vector<std::string> portableObjects;
  };
}
