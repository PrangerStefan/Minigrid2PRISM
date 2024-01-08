#include "PrismModulesPrinter.h"

#include <map>
#include <string>

#define NOFAULT -1
#define LEFT 0
#define RIGHT 1
#define FORWARD 2


std::string northUpdate(const AgentName &a) { return "(y"+a+"'=y"+a+"-1)"; }
std::string southUpdate(const AgentName &a) { return "(y"+a+"'=y"+a+"+1)"; }
std::string eastUpdate(const AgentName &a)  { return "(x"+a+"'=x"+a+"+1)"; }
std::string westUpdate(const AgentName &a)  { return "(x"+a+"'=x"+a+"-1)"; }

namespace prism {

  PrismModulesPrinter::PrismModulesPrinter(std::ostream& os, const ModelType &modelType, const coordinates &maxBoundaries, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const std::map<std::string, cells> &slipperyTiles, const AgentNameAndPositionMap &agentNameAndPositionMap, std::vector<Configuration> config, const float probIntended, const float faultyProbability)
    : os(os), modelType(modelType), maxBoundaries(maxBoundaries), boxes(boxes), balls(balls), lockedDoors(lockedDoors), unlockedDoors(unlockedDoors), keys(keys), slipperyTiles(slipperyTiles), agentNameAndPositionMap(agentNameAndPositionMap), configuration(config), probIntended(probIntended), faultyProbability(faultyProbability) {
      numberOfPlayer = agentNameAndPositionMap.size();
      size_t index = 0;
      for(auto begin = agentNameAndPositionMap.begin(); begin != agentNameAndPositionMap.end(); begin++, index++) {
        agentIndexMap[begin->first] = index;
      }
  }

  void PrismModulesPrinter::printModelType(const ModelType &modelType) {
    switch(modelType) {
      case(ModelType::MDP):
        os << "mdp";
        break;
      case(ModelType::SMG):
        os << "smg";
        break;
    }
    os << "\n\n";
  }

  std::ostream& PrismModulesPrinter::print() {
    for(const auto [agentName, initialPosition] : agentNameAndPositionMap)  {
      agentNameActionMap[agentName] = {};
    }

    for(const auto &key : keys) {
      printPortableObjectModule(key);
    }
    for(const auto &ball : balls) {
      printPortableObjectModule(ball);
    }
    for(const auto &box : boxes) {
      printPortableObjectModule(box);
    }
    for(const auto &door : unlockedDoors) {
      printDoorModule(door, true);
    }
    for(const auto &door : lockedDoors) {
      printDoorModule(door, false);
    }

    for(const auto [agentName, initialPosition] : agentNameAndPositionMap)  {
      printRobotModule(agentName, initialPosition);
    }

    if(faultyBehaviour()) {
      for(const auto [agentName, initialPosition] : agentNameAndPositionMap)  {
        printFaultyMovementModule(agentName);
      }
    }

    return os;
  }


  void PrismModulesPrinter::printConfiguration(const std::vector<Configuration>& configurations) {
    for (auto& configuration : configurations) {
      if (configuration.overwrite_ || configuration.type_ == ConfigType::Module) {
        continue;
      }
      os << configuration.expression_ << std::endl;
    }
  }

  void PrismModulesPrinter::printConstants(const std::vector<std::string> &constants) {
    for (auto& constant : constants) {
      os << constant << std::endl;
    }
  }

  void PrismModulesPrinter::printPortableObjectModule(const cell &object) {
    std::string identifier = capitalize(object.getColor()) + object.getType();
    os << "\nmodule " << identifier << std::endl;
    os << "\tx" << identifier << " : [-1.." << maxBoundaries.second  << "] init " << object.column << ";\n";
    os << "\ty" << identifier << " : [-1.." << maxBoundaries.first << "] init " << object.row << ";\n";
    os << "\t" << identifier << "PickedUp : bool;\n";
    os << "\n";

    for(const auto [name, position] : agentNameAndPositionMap) {
      printPortableObjectActions(name, identifier);
    }
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printPortableObjectActions(const std::string &agentName, const std::string &identifier) {
    std::string actionName = "[" + agentName + "_pickup_" + identifier + "]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " true -> (x" << identifier << "'=-1) & (y" << identifier << "'=-1) & (" << identifier << "PickedUp'=true);\n";
    actionName = "[" + agentName + "_drop_" + identifier + "_north]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " " << " true -> (x" << identifier << "'=x" << agentName << ")   & (y" << identifier << "'=y" << agentName << "-1) & (" << identifier << "PickedUp'=false);\n";
    actionName = "[" + agentName + "_drop_" + identifier + "_west]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " " << " true -> (x" << identifier << "'=x" << agentName << "-1) & (y" << identifier << "'=y" << agentName << ") & (" << identifier << "PickedUp'=false);\n";
    actionName = "[" + agentName + "_drop_" + identifier + "_south]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " " << " true -> (x" << identifier << "'=x" << agentName << ")   & (y" << identifier << "'=y" << agentName << "+1) & (" << identifier << "PickedUp'=false);\n";
    actionName = "[" + agentName + "_drop_" + identifier + "_east]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " " << " ttrue -> (x" << identifier << "'=x" << agentName << "+1) & (y" << identifier << "'=y" << agentName << ") & (" << identifier << "PickedUp'=false);\n";
  }

  void PrismModulesPrinter::printDoorModule(const cell &door, const bool &opened) {
    std::string identifier = capitalize(door.getColor()) + door.getType();
    os << "\nmodule " << identifier << std::endl;
    os << "\t" << identifier << "Open : bool init false;\n";
    os << "\n";

    if(opened) {
      for(const auto [name, position] : agentNameAndPositionMap) {
        printUnlockedDoorActions(name, identifier);
      }
    } else {
      for(const auto [name, position] : agentNameAndPositionMap) {
        printLockedDoorActions(name, identifier);
      }
    }
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printLockedDoorActions(const std::string &agentName, const std::string &identifier) {
    std::string actionName = "[" + agentName + "_unlock_" + identifier + "]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " !" << identifier << "Open -> (" << identifier << "Open'=true);\n";
    actionName = "[" + agentName + "_close_" + identifier + "]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << actionName << " " << identifier << "Open -> (" << identifier << "Open'=false);\n";
  }

  void PrismModulesPrinter::printUnlockedDoorActions(const std::string &agentName, const std::string &identifier) {
    std::string actionName = "[" + agentName + "_open_" + identifier + "]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t !" << identifier << "Open -> (" << identifier << "Open'=true);\n";
    actionName = "[" + agentName + "_close_" + identifier + "]";
    agentNameActionMap.at(agentName).insert({NOFAULT, actionName});
    os << "\t" << agentName << " " << identifier << "Open -> (" << identifier << "Open'=false);\n";
  }

  void PrismModulesPrinter::printRobotModule(const AgentName &agentName, const coordinates &initialPosition) {
    os << "\nmodule " << agentName << std::endl;
    os << "\tx"    << agentName << " : [0.." << maxBoundaries.second  << "] init " << initialPosition.second << ";\n";
    os << "\ty"    << agentName << " : [0.." << maxBoundaries.first << "] init " << initialPosition.first << ";\n";
    os << "\tview" << agentName << " : [0..3] init 1;\n";

    printTurnActionsForRobot(agentName);
    printMovementActionsForRobot(agentName);
    if(slipperyBehaviour()) printSlipperyMovementActionsForRobot(agentName);

    for(const auto &door : unlockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      printUnlockedDoorActionsForRobot(agentName, identifier);
    }

    for(const auto &door : lockedDoors) {
      std::string identifier = capitalize(door.getColor()) + door.getType();
      std::string key = capitalize(door.getColor()) + "Key";
      printLockedDoorActionsForRobot(agentName, identifier, key);
    }

    for(const auto &key : keys) {
      std::string identifier = capitalize(key.getColor()) + key.getType();
      os << "\t" << agentName << "Carrying" << identifier << " : bool init false;\n";
      printPortableObjectActionsForRobot(agentName, identifier);
    }

    for(const auto &ball : balls) {
      std::string identifier = capitalize(ball.getColor()) + ball.getType();
      os << "\t" << agentName << "Carrying" << identifier << " : bool init false;\n";
      printPortableObjectActionsForRobot(agentName, identifier);
    }

    for(const auto &box : boxes) {
      std::string identifier = capitalize(box.getColor()) + box.getType();
      os << "\t" << agentName << "Carrying" << identifier << " : bool init false;\n";
      printPortableObjectActionsForRobot(agentName, identifier);
    }

    os << "\n" << actionStream.str();
    actionStream.str(std::string());
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printPortableObjectActionsForRobot(const std::string &a, const std::string &i) {
    actionStream << "\t[" << a << "_pickup_" << i << "] "      << " !" << a << "IsCarrying & " <<  a << "CannotMove" << i << " -> (" << a << "Carrying" << i << "'=true);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_north]\t" << a << "Carrying" << i << " & view" << a << "=3 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveNorthWall -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_west] \t" << a << "Carrying" << i << " & view" << a << "=2 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveWestWall  -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_south]\t" << a << "Carrying" << i << " & view" << a << "=1 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveSouthWall -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_east] \t" << a << "Carrying" << i << " & view" << a << "=0 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveEastWall  -> (" << a << "Carrying" << i << "'=false);\n";
    actionStream << "\n";
  }

  void PrismModulesPrinter::printUnlockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier) {
    actionStream << "\t[" << agentName << "_open_" << identifier  << "] " << agentName << "CannotMove" << identifier << " -> true;\n";
    actionStream << "\t[" << agentName << "_close_" << identifier << "] " << agentName << "IsNextTo"     << identifier << " -> true;\n";
    actionStream << "\n";
  }

  void PrismModulesPrinter::printLockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier, const std::string &key) {
    actionStream << "\t[" << agentName << "_unlock_" << identifier  << "] " << agentName << "CannotMove" << identifier << " & " << agentName << "Carrying" << key << " -> true;\n";
    actionStream << "\t[" << agentName << "_close_" << identifier << "] "   << agentName << "IsNextTo"   << identifier << " & " << agentName << "Carrying" << key << " -> true;\n";
    actionStream << "\n";
  }

  void PrismModulesPrinter::printTurnActionsForRobot(const AgentName &a) {
    actionStream << printTurnGuard(a, "right", RIGHT, "true") << printTurnUpdate(a, {1.0, "(view"+a+"'=mod(view"+a+"+1,4))"}, RIGHT);
    actionStream << printTurnGuard(a, "left", LEFT, "view"+a+">0") << printTurnUpdate(a, {1.0, "(view"+a+"'=view"+a+"-1)"}, LEFT);
    actionStream << printTurnGuard(a, "left", LEFT, "view"+a+"=0") << printTurnUpdate(a, {1.0, "(view"+a+"'=3)"}, LEFT);
  }

  void PrismModulesPrinter::printMovementActionsForRobot(const AgentName &a) {
    actionStream << printMovementGuard(a, "North", 3) << printMovementUpdate(a, {1.0, "(y"+a+"'=y"+a+"-1)"});
    actionStream << printMovementGuard(a, "East",  0) << printMovementUpdate(a, {1.0, "(x"+a+"'=x"+a+"+1)"});
    actionStream << printMovementGuard(a, "South", 1) << printMovementUpdate(a, {1.0, "(y"+a+"'=y"+a+"+1)"});
    actionStream << printMovementGuard(a, "West",  2) << printMovementUpdate(a, {1.0, "(x"+a+"'=x"+a+"-1)"});
  }

  std::string PrismModulesPrinter::printMovementGuard(const AgentName &a, const std::string &direction, const size_t &viewDirection) {
    std::string actionName = "[" + a + "_move_" + direction + "]";
    agentNameActionMap.at(a).insert({FORWARD, actionName});
    return "\t" + actionName + " " + viewVariable(a, viewDirection) + " !" + a + "IsOnSlippery & !" + a + "IsOnLava & !" + a + "IsOnGoal & !" + a + "CannotMove" + direction + "Wall &!" + a + "CannotMoveConditionally -> ";
  }

  std::string PrismModulesPrinter::printMovementUpdate(const AgentName &a, const update &u) const {
    return updateToString(u) + ";\n";
  }

  std::string PrismModulesPrinter::printTurnGuard(const AgentName &a, const std::string &direction, const ActionId &actionId, const std::string &cond) {
    std::string actionName = "[" + a + "_turn_" + direction + "]";
    agentNameActionMap.at(a).insert({actionId, actionName});
    return "\t" + actionName + " " + cond + " -> ";
  }

  std::string PrismModulesPrinter::printTurnUpdate(const AgentName &a, const update &u, const ActionId &actionId) const {
    return updateToString(u) + ";\n";
  }

  void PrismModulesPrinter::printSlipperyMovementActionsForRobot(const AgentName &a) {
    if(!slipperyTiles.at("North").empty()) {
      printSlipperyMovementActionsForNorth(a);
      printSlipperyTurnActionsForNorth(a);
    }
    if(!slipperyTiles.at("East").empty()) {
      printSlipperyMovementActionsForEast(a) ;
      printSlipperyTurnActionsForEast(a);
    }
    if(!slipperyTiles.at("South").empty()) {
      printSlipperyMovementActionsForSouth(a);
      printSlipperyTurnActionsForSouth(a);
    }
    if(!slipperyTiles.at("West").empty()) {
      printSlipperyMovementActionsForWest(a) ;
      printSlipperyTurnActionsForWest(a);
    }
  }

  void PrismModulesPrinter::printSlipperyMovementActionsForNorth(const AgentName &a) {
    actionStream << printSlipperyMovementGuard(a, "North", 3, { "CanSlipNorth",  "CanSlipNorthEast",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, northUpdate(a)}, {(1 - probIntended) * 1/2, northUpdate(a)+"&"+eastUpdate(a)}, {(1 - probIntended) * 1/2, northUpdate(a)+"&"+westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, {"!CanSlipNorth",  "CanSlipNorthEast",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1/2, northUpdate(a)+"&"+eastUpdate(a)}, {1/2, northUpdate(a)+"&"+westUpdate(a)} });
    actionStream << printSlipperyMovementGuard(a, "North", 3, { "CanSlipNorth", "!CanSlipNorthEast",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, northUpdate(a)}, {(1 - probIntended), northUpdate(a)+"&"+westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, { "CanSlipNorth",  "CanSlipNorthEast", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, northUpdate(a)}, {(1 - probIntended), northUpdate(a)+"&"+eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, {"!CanSlipNorth", "!CanSlipNorthEast",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1, northUpdate(a)+"&"+westUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, { "CanSlipNorth", "!CanSlipNorthEast", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, {"!CanSlipNorth",  "CanSlipNorthEast", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1, northUpdate(a)+"&"+eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 3, {"!CanSlipNorth", "!CanSlipNorthEast", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "North", 2, { "CanSlipWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, westUpdate(a) }, {1 - probIntended, westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 2, {"!CanSlipWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1, westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 2, { "CanSlipWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", { {1, westUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 2, {"!CanSlipWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "North", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "North", 0, { "CanSlipEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, eastUpdate(a) }, {1 - probIntended, eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 0, {"!CanSlipEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "North", { {1, eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 0, { "CanSlipEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "North", { {1, eastUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 0, {"!CanSlipEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "North", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "North", 1, { "CanSlipSouth",  "CanSlipNorth"}) << printSlipperyMovementUpdate(a, "North", { {probIntended, southUpdate(a) }, {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 1, {"!CanSlipSouth",  "CanSlipNorth"}) << printSlipperyMovementUpdate(a, "North", { {1, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 1, { "CanSlipSouth", "!CanSlipNorth"}) << printSlipperyMovementUpdate(a, "North", { {1, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "North", 1, {"!CanSlipSouth", "!CanSlipNorth"}) << printSlipperyMovementUpdate(a, "North", {}) << "\n";
  }

  void PrismModulesPrinter::printSlipperyMovementActionsForEast(const AgentName &a) {
    actionStream << printSlipperyMovementGuard(a, "East", 0, { "CanSlipEast",  "CanSlipSouthEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, eastUpdate(a)}, {(1 - probIntended) * 1/2, eastUpdate(a)+"&"+southUpdate(a)}, {(1 - probIntended) * 1/2, eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, {"!CanSlipEast",  "CanSlipSouthEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1/2, eastUpdate(a)+"&"+southUpdate(a)}, {1/2, eastUpdate(a)+"&"+northUpdate(a)} });
    actionStream << printSlipperyMovementGuard(a, "East", 0, { "CanSlipEast", "!CanSlipSouthEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, eastUpdate(a)}, {(1 - probIntended), eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, { "CanSlipEast",  "CanSlipSouthEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, eastUpdate(a)}, {(1 - probIntended), eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, {"!CanSlipEast", "!CanSlipSouthEast",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)+"&"+northUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, { "CanSlipEast", "!CanSlipSouthEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, {"!CanSlipEast",  "CanSlipSouthEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 0, {"!CanSlipEast", "!CanSlipSouthEast", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "East", 3, { "CanSlipNorth",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, northUpdate(a) }, {1 - probIntended, eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 3, {"!CanSlipNorth",  "CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 3, { "CanSlipNorth", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, northUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 3, {"!CanSlipNorth", "!CanSlipNorthEast"}) << printSlipperyMovementUpdate(a, "East", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "East", 1, { "CanSlipSouth",  "CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, southUpdate(a) }, {1 - probIntended, eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 1, {"!CanSlipSouth",  "CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 1, { "CanSlipSouth", "!CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "East", { {1, southUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 1, {"!CanSlipSouth", "!CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "East", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "East", 2, { "CanSlipWest",  "CanSlipEast"}) << printSlipperyMovementUpdate(a, "East", { {probIntended, eastUpdate(a) }, {1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 2, {"!CanSlipWest",  "CanSlipEast"}) << printSlipperyMovementUpdate(a, "East", { {1, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 2, { "CanSlipWest", "!CanSlipEast"}) << printSlipperyMovementUpdate(a, "East", { {1, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "East", 2, {"!CanSlipWest", "!CanSlipEast"}) << printSlipperyMovementUpdate(a, "East", {}) << "\n";
  }

  void PrismModulesPrinter::printSlipperyMovementActionsForSouth(const AgentName &a) {
    actionStream << printSlipperyMovementGuard(a, "South", 1, { "CanSlipSouth",  "CanSlipSouthEast",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, southUpdate(a)}, {(1 - probIntended) * 1/2, southUpdate(a)+"&"+eastUpdate(a)}, {(1 - probIntended) * 1/2, southUpdate(a)+"&"+westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, {"!CanSlipSouth",  "CanSlipSouthEast",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1/2, southUpdate(a)+"&"+eastUpdate(a)}, {1/2, southUpdate(a)+"&"+westUpdate(a)} });
    actionStream << printSlipperyMovementGuard(a, "South", 1, { "CanSlipSouth", "!CanSlipSouthEast",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, southUpdate(a)}, {(1 - probIntended), southUpdate(a)+"&"+westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, { "CanSlipSouth",  "CanSlipSouthEast", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, southUpdate(a)}, {(1 - probIntended), southUpdate(a)+"&"+eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, {"!CanSlipSouth", "!CanSlipSouthEast",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1, southUpdate(a)+"&"+westUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, { "CanSlipSouth", "!CanSlipSouthEast", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, {"!CanSlipSouth",  "CanSlipSouthEast", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1, southUpdate(a)+"&"+eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 1, {"!CanSlipSouth", "!CanSlipSouthEast", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "South", 2, { "CanSlipWest",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, westUpdate(a) }, {1 - probIntended, westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 2, {"!CanSlipWest",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1, westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 2, { "CanSlipWest", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", { {1, westUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 2, {"!CanSlipWest", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "South", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "South", 0, { "CanSlipEast",  "CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, eastUpdate(a) }, {1 - probIntended, eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 0, {"!CanSlipEast",  "CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "South", { {1, eastUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 0, { "CanSlipEast", "!CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "South", { {1, eastUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 0, {"!CanSlipEast", "!CanSlipSouthEast"}) << printSlipperyMovementUpdate(a, "South", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "South", 3, { "CanSlipSouth",  "CanSlipNorth"}) << printSlipperyMovementUpdate(a, "South", { {probIntended, southUpdate(a) }, {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 3, {"!CanSlipSouth",  "CanSlipNorth"}) << printSlipperyMovementUpdate(a, "South", { {1, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 3, { "CanSlipSouth", "!CanSlipNorth"}) << printSlipperyMovementUpdate(a, "South", { {1, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "South", 3, {"!CanSlipSouth", "!CanSlipNorth"}) << printSlipperyMovementUpdate(a, "South", {}) << "\n";
  }

  void PrismModulesPrinter::printSlipperyMovementActionsForWest(const AgentName &a) {
    actionStream << printSlipperyMovementGuard(a, "West", 2, { "CanSlipWest",  "CanSlipSouthWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, westUpdate(a)}, {(1 - probIntended) * 1/2, westUpdate(a)+"&"+southUpdate(a)}, {(1 - probIntended) * 1/2, westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, {"!CanSlipWest",  "CanSlipSouthWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1/2, westUpdate(a)+"&"+southUpdate(a)}, {1/2, westUpdate(a)+"&"+northUpdate(a)} });
    actionStream << printSlipperyMovementGuard(a, "West", 2, { "CanSlipWest", "!CanSlipSouthWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, westUpdate(a)}, {(1 - probIntended), westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, { "CanSlipWest",  "CanSlipSouthWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, westUpdate(a)}, {(1 - probIntended), westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, {"!CanSlipWest", "!CanSlipSouthWest",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)+"&"+northUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, { "CanSlipWest", "!CanSlipSouthWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, {"!CanSlipWest",  "CanSlipSouthWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 2, {"!CanSlipWest", "!CanSlipSouthWest", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "West", 3, { "CanSlipNorth",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, northUpdate(a) }, {1 - probIntended, westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 3, {"!CanSlipNorth",  "CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)+"&"+northUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 3, { "CanSlipNorth", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 3, {"!CanSlipNorth", "!CanSlipNorthWest"}) << printSlipperyMovementUpdate(a, "West", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "West", 1, { "CanSlipSouth",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, southUpdate(a) }, {1 - probIntended, westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 1, {"!CanSlipSouth",  "CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)+"&"+southUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 1, { "CanSlipSouth", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "West", { {1, southUpdate(a) } }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 1, {"!CanSlipSouth", "!CanSlipSouthWest"}) << printSlipperyMovementUpdate(a, "West", {}) << "\n";

    actionStream << printSlipperyMovementGuard(a, "West", 0, { "CanSlipEast",  "CanSlipWest"}) << printSlipperyMovementUpdate(a, "West", { {probIntended, westUpdate(a) }, {1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 0, {"!CanSlipEast",  "CanSlipWest"}) << printSlipperyMovementUpdate(a, "West", { {1, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 0, { "CanSlipEast", "!CanSlipWest"}) << printSlipperyMovementUpdate(a, "West", { {1, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyMovementGuard(a, "West", 0, {"!CanSlipEast", "!CanSlipWest"}) << printSlipperyMovementUpdate(a, "West", {}) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForNorth(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, { "CanSlipNorth"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, {"!CanSlipNorth"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipNorth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipNorth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipNorth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipNorth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForEast(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, { "CanSlipEast"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, {"!CanSlipEast"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipEast"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipEast"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipEast"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipEast"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForSouth(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, { "CanSlipSouth"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, {"!CanSlipSouth"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipSouth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipSouth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipSouth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipSouth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForWest(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, { "CanSlipWest"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", RIGHT, {"!CanSlipWest"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipWest"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, { "CanSlipWest"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipWest"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", LEFT, {"!CanSlipWest"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  std::string PrismModulesPrinter::printSlipperyMovementGuard(const AgentName &a, const std::string &direction, const ViewDirection &viewDirection, const std::vector<std::string> &guards) {
    std::string actionName = "[" + a + "_move_" + direction + "]";
    agentNameActionMap.at(a).insert({FORWARD, actionName});
    return "\t" + actionName + " " + viewVariable(a, viewDirection) + a + "IsOnSlippery" + direction + " & " + buildConjunction(a, guards) + " -> ";
  }

  std::string PrismModulesPrinter::printSlipperyMovementUpdate(const AgentName &a, const std::string &direction, const updates &u) const {
    return updatesToString(u);
  }

  std::string PrismModulesPrinter::printSlipperyTurnGuard(const AgentName &a, const std::string &direction, const ActionId &actionId, const std::vector<std::string> &guards, const std::string &cond) {
    std::string actionName = "[" + a + "_turn_" + direction + "]";
    agentNameActionMap.at(a).insert({actionId, actionName});
    return "\t" + actionName + " " + buildConjunction(a, guards) + " & " + cond + " -> ";
  }

  std::string PrismModulesPrinter::printSlipperyTurnUpdate(const AgentName &a, const updates &u) {
    return updatesToString(u);
  }

  void PrismModulesPrinter::printFaultyMovementModule(const AgentName &a) {
    os << "\nmodule " << a << "FaultyBehaviour" << std::endl;
    os << "\tpreviousAction" << a << " : [-1..2] init -1;\n";

    for(const auto [actionId, actionName] : agentNameActionMap.at(a)) {
      os << "\t" << actionName << faultyBehaviourGuard(a, actionId) << " -> " << faultyBehaviourUpdate(a, actionId) << ";\n";
    }
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printMoveModule() {
    os << "\nmodule " << "Arbiter" << std::endl;
    os << "\tclock : [0.." << agentIndexMap.size() - 1 << "] init 0;\n";

    for(const auto [agentName, actions] : agentNameActionMap) {
      for(const auto [actionId, actionName] : actions) {
        os << "\t" << actionName << " " << moveGuard(agentName) << " -> " << moveUpdate(agentName) << ";\n";
      }
    }
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printConfiguredActions(const AgentName &agentName) {
    for (auto& config : configuration) {
      if (config.type_ == ConfigType::Module && !config.overwrite_ && agentName == config.module_) {
        os << config.expression_ ;
      }
    }

    os << "\n";

    return os;
  }

  std::ostream& PrismModulesPrinter::printDoneActions(std::ostream &os, const AgentName &agentName) {
    os << "\t[" << agentName << "_done]" << moveGuard(agentName) << agentName << "IsInGoal | " << agentName << "IsInLava -> (" << agentName << "Done'=true);\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printPlayerStruct(std::ostream &os, const AgentName &agentName, const bool agentWithView, const std::vector<float> &probabilities, const std::set<std::string> &slipperyActions) {
    os << "player " << agentName << "\n\t";
    bool first = true;
    std::list<std::string> allActions = { "_move_north", "_move_east", "_move_south", "_move_west" };
    std::list<std::string> movementActions = allActions;
    for(auto const& probability : probabilities) {
      std::string percentageString = std::to_string((int)(100 * probability));
      for(auto const& movement : movementActions) {
        allActions.push_back(movement + "_" + percentageString);
      }
    }
    if(agentWithView) {
      allActions.push_back("_turn_left");
      allActions.push_back("_turn_right");
    } else {
      allActions.push_back("_turns");
    }

    for(auto const& action : allActions) {
      if(first) first = false; else os << ", ";
      os << "[" << agentName << action << "]";
    }
    for(auto const& action : slipperyActions) {
      os << ", " << action;
    }

    os << ", [" << agentName << "_done]";
    os << "\nendplayer\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printGlobalMoveVariable(std::ostream &os, const size_t &numberOfPlayer) {
    os << "\nglobal move : [0.." << std::to_string(numberOfPlayer - 1) << "];\n\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printRewards(std::ostream &os, const AgentName &agentName, const std::map<coordinates, float> &stateRewards, const cells &lava, const cells &goals, const std::map<Color, cells> &backgroundTiles) {
    if(lava.size() != 0) {
      os << "rewards \"" << agentName << "SafetyNoBFS\"\n";
      os << "\t" <<agentName << "IsInLavaAndNotDone: -100;\n";
      os << "endrewards\n";
    }

    if (!goals.empty() || !lava.empty())  {
      os << "rewards \"" << agentName << "SafetyNoBFSAndGoal\"\n";
      if(goals.size() != 0) os << "\t" << agentName << "IsInGoalAndNotDone:  100;\n";
      if(lava.size() != 0) os << "\t" << agentName << "IsInLavaAndNotDone: -100;\n";
      os << "endrewards\n";
    }

    os << "rewards \"" << agentName << "Time\"\n";
    os << "\t!" << agentName << "IsInGoal : -1;\n";
    if(goals.size() != 0) os << "\t" << agentName << "IsInGoalAndNotDone:  100;\n";
    if(lava.size() != 0) os << "\t" << agentName << "IsInLavaAndNotDone: -100;\n";
    os << "endrewards\n";

    if(stateRewards.size() > 0) {
      os << "rewards \"" << agentName << "SafetyWithBFS\"\n";
      if(lava.size() != 0) os << "\t" << agentName << "IsInLavaAndNotDone: -100;\n";
      for(auto const [coordinates, reward] : stateRewards) {
        os << "\txAgent=" << coordinates.first << "&yAgent=" << coordinates.second << " : " << reward << ";\n";
      }
      os << "endrewards\n";
    }
    if(stateRewards.size() > 0) {
      os << "rewards \"" << agentName << "SafetyWithBFSAndGoal\"\n";
      if(goals.size() != 0) os << "\tAgentIsInGoalAndNotDone:  100;\n";
      if(lava.size() != 0) os << "\tAgentIsInLavaAndNotDone: -100;\n";
      for(auto const [coordinates, reward] : stateRewards) {
        os << "\txAgent=" << coordinates.first << "&yAgent=" << coordinates.second << " : " << reward << ";\n";
      }
      os << "endrewards\n";
    }

    for(auto const entry : backgroundTiles)
    {
      std::cout << getColor(entry.first) << " ";
      for(auto const cell : entry.second){
        std::cout << cell.getCoordinates().first << " " << cell.getCoordinates().second << std::endl;
      }
    }
    if(backgroundTiles.size() > 0) {
      os << "rewards \"TaxiReward\"\n";
      os << "\t!AgentIsInGoal : -1;\n";
      std::string allPassengersPickedUp = "";
      bool first = true;
      for(auto const [color, cells] : backgroundTiles) {
        if(cells.size() == 0) continue;
        if(first) first = false; else allPassengersPickedUp += "&";
        std::string c = getColor(color);
        c.at(0) = std::toupper(c.at(0));
        std::string visitedLabel = agentName + "_picked_up_" + c;
        allPassengersPickedUp += visitedLabel;
        os << "[" << agentName << "_pickup_" << c << "] true : 100;\n";
      }
      if(goals.size() != 0) os << "\tAgentIsInGoalAndNotDone & " << allPassengersPickedUp << " :  100;\n";
      if(goals.size() != 0) os << "\tAgentIsInGoalAndNotDone & !(" << allPassengersPickedUp << ") :  -100;\n";
      os << "endrewards";
    }
    return os;
  }

  std::string PrismModulesPrinter::faultyBehaviourGuard(const AgentName &agentName, const ActionId &actionId) const {
    if(faultyBehaviour()) {
      if(actionId == NOFAULT) {
        return "(previousAction" + agentName + "=" + std::to_string(NOFAULT) + ") ";
      } else {
        return "(previousAction" + agentName + "=" + std::to_string(NOFAULT) + " | previousAction" + agentName + "=" + std::to_string(actionId) + ") ";
      }
    } else {
      return "";
    }
  }

  std::string PrismModulesPrinter::faultyBehaviourUpdate(const AgentName &agentName, const ActionId &actionId) const {
    if(actionId != NOFAULT) {
      return updatesToString({ {1 - faultyProbability, "(previousAction" + agentName + "'=" + std::to_string(NOFAULT) + ")"},  {faultyProbability, "(previousAction" + agentName + "'=" + std::to_string(actionId) + ")" } });
    } else {
      return "true";
    }
  }

  std::string PrismModulesPrinter::moveGuard(const AgentName &agentName) const {
    return "clock=" + std::to_string(agentIndexMap.at(agentName));
  }

  std::string PrismModulesPrinter::moveUpdate(const AgentName &agentName) const {
    size_t agentIndex = agentIndexMap.at(agentName);
    return (agentIndex == numberOfPlayer - 1) ? " & (clock'=0) " : " & (clock'=" + std::to_string(agentIndex + 1) + ") ";

  }

  std::string PrismModulesPrinter::updatesToString(const updates &updates) const {
    if(updates.empty()) return "true";
    std::string updatesString = "";
    bool first = true;
    for(auto const update : updates) {
      if(first) first = false;
      else updatesString += " + ";
      updatesString += updateToString(update);
    }
    return updatesString;
  }

  std::string PrismModulesPrinter::updateToString(const update &u) const {
    return std::to_string(u.first) + ": " + u.second;
  }

  std::string PrismModulesPrinter::viewVariable(const AgentName &agentName, const size_t &agentDirection, const bool agentWithView) const {
    return agentWithView ? " view" + agentName + "=" + std::to_string(agentDirection) + " & " : " ";
  }

  bool PrismModulesPrinter::anyPortableObject() const {
    return !keys.empty() || !boxes.empty() || !balls.empty();
  }

  bool PrismModulesPrinter::faultyBehaviour() const {
    return faultyProbability > 0.0f;
  }

  bool PrismModulesPrinter::slipperyBehaviour() const {
    return !slipperyTiles.at("North").empty() || !slipperyTiles.at("East").empty() || !slipperyTiles.at("South").empty() || !slipperyTiles.at("West").empty();
  }

  bool PrismModulesPrinter::isGame() const {
    return modelType == ModelType::SMG;
  }

  std::string PrismModulesPrinter::buildConjunction(const AgentName &a, std::vector<std::string> formulae) const {
    if(formulae.empty()) return "true";
    std::string conjunction = "";
    bool first = true;
    for(auto const formula : formulae) {
      if(first) first = false;
      else conjunction += " & ";
      conjunction += formula;
    }
    return conjunction;
  }

}
