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
    : os(os), modelType(modelType), maxBoundaries(maxBoundaries), boxes(boxes), balls(balls), lockedDoors(lockedDoors), unlockedDoors(unlockedDoors), keys(keys), slipperyTiles(slipperyTiles), agentNameAndPositionMap(agentNameAndPositionMap), configuration(config), probIntended(probIntended), faultyProbability(faultyProbability), viewDirectionMapping({{0, "East"}, {1, "South"}, {2, "West"}, {3, "North"}}) {
      numberOfPlayer = agentNameAndPositionMap.size();
      size_t index = 0;
      for(auto begin = agentNameAndPositionMap.begin(); begin != agentNameAndPositionMap.end(); begin++, index++) {
        agentIndexMap[begin->first] = index;
      }
  }

  std::ostream& PrismModulesPrinter::printModelType(const ModelType &modelType) {
    switch(modelType) {
      case(ModelType::MDP):
        os << "mdp";
        break;
      case(ModelType::SMG):
        os << "smg";
        break;
    }
    os << "\n\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::print() {
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
    return os;
  }


  std::ostream& PrismModulesPrinter::printConfiguration(std::ostream& os, const std::vector<Configuration>& configurations) {
    for (auto& configuration : configurations) {
      if (configuration.overwrite_ || configuration.type_ == ConfigType::Module) {
        continue;
      }
      os << configuration.expression_ << std::endl;
    }
    return os;
  }

  std::ostream& PrismModulesPrinter::printConstants(std::ostream &os, const std::vector<std::string> &constants) {
    for (auto& constant : constants) {
      os << constant << std::endl;
    }
    return os;
  }

  std::ostream& PrismModulesPrinter::printInitStruct(std::ostream &os, const AgentNameAndPositionMap &agents, const KeyNameAndPositionMap &keys, const cells &lockedDoors, const cells &unlockedDoors, prism::ModelType modelType) {
    /*
    os << "init\n";
    os << "\t";

    bool first = true;
    for (auto const& agent : agents) {
      if (first) first = false;
      else os << " & ";
      os << "(!" << agent.first << "IsInGoal & !" << agent.first << "IsInLava & !" << agent.first << "Done & !" << agent.first << "IsOnWall & ";
      os << "x" << agent.first << "=" << agent.second.second << " & y" << agent.first << "=" << agent.second.first << ")";
      os << " & !" << agent.first << "_is_carrying_object";
        // os << " & ( !AgentIsOnSlippery ) ";
      }

      for (auto const& key : keys) {
         os << " & ( !" << agent.first << "_has_" << key.first << "_key )";
      }
    }

    for (auto const& key : keys) {
      os << " & ( xKey" << key.first << "="<< key.second.second<< ")";
      os << " & ( yKey" << key.first << "=" << key.second.first << ")";
    }

    for (auto const& locked : lockedDoors) {
      os << " & (Door" << locked.getColor() << "locked & !Door" << locked.getColor() << "open)";
    }

    for (auto const& unlocked : unlockedDoors) {
      os << " & (!Door" << unlocked.getColor() << "locked & !Door" << unlocked.getColor() << "open)";
    }

    if (modelType == ModelType::SMG) {
      os << " & move=0";
    }


    os << "\nendinit\n\n";

    */
    return os;
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
	  os << "\t[" << agentName << "_pickup_" << identifier << "]\ttrue -> (x" << identifier << "'=-1) & (y" << identifier << "'=-1) & (" << identifier << "PickedUp'=true);\n";
	  os << "\t[" << agentName << "_drop_" << identifier << "_north]\ttrue -> (x" << identifier << "'=x" << agentName << ")   & (y" << identifier << "'=y" << agentName << "-1) & (" << identifier << "PickedUp'=false);\n";
	  os << "\t[" << agentName << "_drop_" << identifier << "_west]\ttrue -> (x" << identifier << "'=x" << agentName << "-1) & (y" << identifier << "'=y" << agentName << ") & (" << identifier << "PickedUp'=false);\n";
	  os << "\t[" << agentName << "_drop_" << identifier << "_south]\ttrue -> (x" << identifier << "'=x" << agentName << ")   & (y" << identifier << "'=y" << agentName << "+1) & (" << identifier << "PickedUp'=false);\n";
	  os << "\t[" << agentName << "_drop_" << identifier << "_east]\ttrue -> (x" << identifier << "'=x" << agentName << "+1) & (y" << identifier << "'=y" << agentName << ") & (" << identifier << "PickedUp'=false);\n";
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
    os << "\t[" << agentName << "_unlock_" << identifier << "] !" << identifier << "Open -> (" << identifier << "Open'=true);\n";
    os << "\t[" << agentName << "_close_" << identifier << "] " << identifier << "Open -> (" << identifier << "Open'=false);\n";
  }

  void PrismModulesPrinter::printUnlockedDoorActions(const std::string &agentName, const std::string &identifier) {
    os << "\t[" << agentName << "_open_" << identifier << "] !" << identifier << "Open -> (" << identifier << "Open'=true);\n";
    os << "\t[" << agentName << "_close_" << identifier << "] " << identifier << "Open -> (" << identifier << "Open'=false);\n";
  }

  void PrismModulesPrinter::printRobotModule(const AgentName &agentName, const coordinates &initialPosition) {
    os << "\nmodule " << agentName << std::endl;
    os << "\tx"    << agentName << " : [0.." << maxBoundaries.second  << "] init " << initialPosition.second << ";\n";
    os << "\ty"    << agentName << " : [0.." << maxBoundaries.first << "] init " << initialPosition.first << ";\n";
    os << "\tview" << agentName << " : [0..3] init 1;\n";

    if(faultyBehaviour()) os << "\tpreviousAction" << agentName << " : [-1..2] init -1;\n";

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
    actionStream << "\t[" << a << "_pickup_" << i << "] "      << faultyBehaviourGuard(a, NOFAULT) << moveGuard(a) << " !" << a << "IsCarrying & " <<  a << "CannotMove" << i << " -> (" << a << "Carrying" << i << "'=true);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_north]\t" << faultyBehaviourGuard(a, NOFAULT) << moveGuard(a) << a << "Carrying" << i << " & view" << a << "=3 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveNorthWall -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_west] \t" << faultyBehaviourGuard(a, NOFAULT) << moveGuard(a) << a << "Carrying" << i << " & view" << a << "=2 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveWestWall  -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_south]\t" << faultyBehaviourGuard(a, NOFAULT) << moveGuard(a) << a << "Carrying" << i << " & view" << a << "=1 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveSouthWall -> (" << a << "Carrying" << i << "'=false);\n";
	  actionStream << "\t[" << a << "_drop_" << i << "_east] \t" << faultyBehaviourGuard(a, NOFAULT) << moveGuard(a) << a << "Carrying" << i << " & view" << a << "=0 & !" << a << "CannotMoveConditionally & !" << a << "CannotMoveEastWall  -> (" << a << "Carrying" << i << "'=false);\n";
    actionStream << "\n";
  }

  void PrismModulesPrinter::printUnlockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier) {
    actionStream << "\t[" << agentName << "_open_" << identifier  << "] " << faultyBehaviourGuard(agentName, NOFAULT) << moveGuard(agentName) << agentName << "CannotMove" << identifier << " -> true;\n";
    actionStream << "\t[" << agentName << "_close_" << identifier << "] " << faultyBehaviourGuard(agentName, NOFAULT) << moveGuard(agentName) << agentName << "IsNextTo"     << identifier << " -> true;\n";
    actionStream << "\n";
  }

  void PrismModulesPrinter::printLockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier, const std::string &key) {
    actionStream << "\t[" << agentName << "_unlock_" << identifier  << "] " << faultyBehaviourGuard(agentName, NOFAULT) << moveGuard(agentName) << agentName << "CannotMove" << identifier << " & " << agentName << "Carrying" << key << " -> true;\n";
    actionStream << "\t[" << agentName << "_close_" << identifier << "] "   << faultyBehaviourGuard(agentName, NOFAULT) << moveGuard(agentName) << agentName << "IsNextTo"   << identifier << " & " << agentName << "Carrying" << key << " -> true;\n";
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

  std::string PrismModulesPrinter::printMovementGuard(const AgentName &a, const std::string &direction, const size_t &viewDirection) const {
    return "\t[" + a + "_move_" + direction + "]" + moveGuard(a) + viewVariable(a, viewDirection) + faultyBehaviourGuard(a, FORWARD) + " !" + a + "IsOnSlippery & !" + a + "IsOnLava & !" + a + "IsOnGoal & !" + a + "CannotMove" + direction + "Wall &!" + a + "CannotMoveConditionally -> ";
  }

  std::string PrismModulesPrinter::printMovementUpdate(const AgentName &a, const update &u) const {
    if(!faultyBehaviour()) {
      return updateToString(u) + ";\n";
    } else {
      update nonFaultyUpdate = {u.first - faultyProbability, u.second + " & " + faultyBehaviourUpdate(a, NOFAULT)};
      update faultyUpdate = {faultyProbability, u.second + " & " + faultyBehaviourUpdate(a, FORWARD)};
      return updateToString(nonFaultyUpdate) + " + " + updateToString(faultyUpdate) + ";\n";
    }
  }

  std::string PrismModulesPrinter::printTurnGuard(const AgentName &a, const std::string &direction, const ActionId &actionId, const std::string &cond) const {
    return "\t[" + a + "_turn_" + direction + "]" + moveGuard(a) + faultyBehaviourGuard(a, actionId) + cond + " -> ";
  }

  std::string PrismModulesPrinter::printTurnUpdate(const AgentName &a, const update &u, const ActionId &actionId) const {
    if(!faultyBehaviour()) {
      return updateToString(u) + ";\n";
    } else {
      update nonFaultyUpdate = {u.first - faultyProbability, u.second + " & " + faultyBehaviourUpdate(a, NOFAULT)};
      update faultyUpdate = {faultyProbability, u.second + " & " + faultyBehaviourUpdate(a, actionId)};
      return updateToString(nonFaultyUpdate) + " + " + updateToString(faultyUpdate) + ";\n";
    }
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
    actionStream << printSlipperyTurnGuard(a, "right", { "CanSlipNorth"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", {"!CanSlipNorth"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipNorth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipNorth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, northUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipNorth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipNorth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForEast(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", { "CanSlipEast"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", {"!CanSlipEast"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipEast"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipEast"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, eastUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipEast"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipEast"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForSouth(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", { "CanSlipSouth"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", {"!CanSlipSouth"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipSouth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipSouth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, southUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipSouth"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipSouth"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  void PrismModulesPrinter::printSlipperyTurnActionsForWest(const AgentName &a) {
    actionStream << printSlipperyTurnGuard(a, "right", { "CanSlipWest"},  "true") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"+1,4))"}, { 1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "right", {"!CanSlipWest"}, "true") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";

    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipWest"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=mod(view"+a+"-1)"}, {1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", { "CanSlipWest"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {probIntended, "(view"+a+"'=view"+a+"=3)"},     {1 - probIntended, westUpdate(a)} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipWest"}, "view"+a+">0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=mod(view"+a+"+1,4))"} }) << "\n";
    actionStream << printSlipperyTurnGuard(a, "left", {"!CanSlipWest"}, "view"+a+"=0") << printSlipperyTurnUpdate(a, { {1, "(view"+a+"'=view"+a+"=3)"} }) << "\n";
  }

  std::string PrismModulesPrinter::printSlipperyMovementGuard(const AgentName &a, const std::string &direction, const ViewDirection &viewDirection, const std::vector<std::string> &guards) const {
    return "\t[" + a + "_move_" + direction + "] " + moveGuard(a) + viewVariable(a, viewDirection) + faultyBehaviourGuard(a, FORWARD) + a + "IsOnSlippery" + direction + " & " + buildConjunction(a, guards) + " -> ";
  }

  std::string PrismModulesPrinter::printSlipperyMovementUpdate(const AgentName &a, const std::string &direction, const updates &u) const {
    return updatesToString(u, a, FORWARD);
  }

  std::string PrismModulesPrinter::printSlipperyTurnGuard(const AgentName &a, const std::string &direction, const std::vector<std::string> &guards, const std::string &cond) const {
    return "\t[" + a + "_turn_" + direction + "] " + buildConjunction(a, guards) + " & " + cond + " -> ";
  }

  std::string PrismModulesPrinter::printSlipperyTurnUpdate(const AgentName &a, const updates &u) {
    return updatesToString(u, a, NOFAULT);
  }

  std::ostream& PrismModulesPrinter::printConfiguredActions(std::ostream &os, const AgentName &agentName) {
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

   std::ostream& PrismModulesPrinter::printSlipperyTurn(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation) {
   // constexpr std::size_t PROB_PIECES = 9, ALL_POSS_DIRECTIONS = 9;

   // std::array<std::string, ALL_POSS_DIRECTIONS> positionTransition = {
   //   /* north      */                                                "(y" + agentName + "'=y" + agentName + "-1)",
   //   /* north east */   "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "-1)",
   //   /* east       */   "(x" + agentName + "'=x" + agentName + "+1)",
   //   /* east south */   "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "+1)",
   //   /* south      */                                                "(y" + agentName + "'=y" + agentName + "+1)",
   //   /* south west */   "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "+1)",
   //   /* west       */   "(x" + agentName + "'=x" + agentName + "-1)",
   //   /* north west */   "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "-1)",
   //   /* own position */ "(x" + agentName + "'=x" + agentName + ") & (y" + agentName + "'=y" + agentName + ")"
   // };

   // // view transition appdx in form (guard, update part)
   // // IMPORTANT: No mod() usage for turn left due to bug in mod() function for decrement


   // std::array<std::tuple<std::string, std::string, std::string>, 3> viewTransition = {
   //     std::make_tuple(" & " + agentName + "SlipperyTurnRightAllowed ", " & (view" + agentName + "'=mod(view" + agentName + " + 1, 4))", "_right]"),
   //     std::make_tuple(" & " + agentName + "SlipperyTurnLeftAllowed & view" + agentName + ">0", " & (view" + agentName + "'=view" + agentName + " - 1)", "_left]"),
   //     std::make_tuple(" & " + agentName + "SlipperyTurnLeftAllowed & view" + agentName + "=0", " & (view" + agentName + "'=3)", "_left]")
   // };

   // // direction specifics

   // std::string actionName;
   // std::string positionGuard;
   // std::size_t remainPosIndex = 8;
   // std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, e, se, s, sw, w, nw, CURRENT POS }
   // std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants;

   // switch (orientation)
   // {
   //   case SlipperyType::North:
   //     actionName = "\t[" + agentName + "turn_at_slip_north";
   //     positionGuard = "\t" + agentName + "IsOnSlipperyNorth";
   //     prob_piece_dir = { 0, 0, 0, 0, 1, 0, 0, 0, 0 /* <- R */ };
   //     prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_turn_displacement" /* <- R */, "prop_zero", "prop_zero", "prop_zero","prop_zero" };
   //     break;

   //   case SlipperyType::South:
   //     actionName = "\t[" + agentName + "turn_at_slip_south";
   //     positionGuard = "\t" + agentName + "IsOnSlipperySouth";
   //     prob_piece_dir = { 1, 0, 0, 0, 0, 0, 0, 0, 0 /* <- R */ };
   //     prob_piece_dir_constants = { "prop_turn_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" };
   //     break;

   //   case SlipperyType::East:
   //     actionName = "\t[" + agentName + "turn_at_slip_east";
   //     positionGuard = "\t" + agentName + "IsOnSlipperyEast";
   //     prob_piece_dir = { 0, 0, 0, 0, 0, 0, 1, 0, 0 /* <- R */ };
   //     prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_turn_displacement", "prop_zero", "prop_zero" };
   //     break;

   //   case SlipperyType::West:
   //     actionName = "\t[" + agentName + "turn_at_slip_west";
   //     positionGuard = "\t" + agentName + "IsOnSlipperyWest";
   //     prob_piece_dir = { 0, 0, 1, 0, 0, 0, 0, 0, 0 /* <- R */ };
   //     prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_turn_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" };
   //     break;
   // }

   // slipperyActions.insert(actionName + "_left]");
   // slipperyActions.insert(actionName + "_right]");

   // // override probability to 0 if corresp. direction is blocked

   // for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS - 1; i++) {
   //   if (!neighborhood.at(i))
   //     prob_piece_dir.at(i) = 0;
   // }

   // // determine residual probability (R) by replacing 0 with (1 - overall sum)

   // prob_piece_dir.at(remainPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);
   // prob_piece_dir_constants.at(remainPosIndex) = "prop_turn_intended";
   // // <DEBUG_AREA>
   // {
   //   assert(prob_piece_dir.at(remainPosIndex) <= 9 && prob_piece_dir.at(remainPosIndex) >= 6 && "Value not in Range!");
   //   assert(std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0) == PROB_PIECES && "Does not sum up to 1!");
   // }
   // // </DEBUG_AREA>

   // // generic output (for every view transition)
   // for (std::size_t v = 0; v < viewTransition.size(); v++) {
   //     os << actionName << std::get<2>(viewTransition.at(v)) << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << std::get<0>(viewTransition.at(v));
   //     for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
   //       if (i == remainPosIndex) {
   //       os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants.at(i) << " : " << positionTransition.at(i) << std::get<1>(viewTransition.at(v)) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
   //       } else {
   //         os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
   //       }
   //     }
   // }

    return os;
  }

  std::ostream& PrismModulesPrinter::printSlipperyMove(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation) {
    //constexpr std::size_t PROB_PIECES = 9, ALL_POSS_DIRECTIONS = 8;

    //std::array<std::string, ALL_POSS_DIRECTIONS> positionTransition = {
    //  /* north      */                                              "(y" + agentName + "'=y" + agentName + "-1)",
    //  /* north east */ "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "-1)",
    //  /* east       */ "(x" + agentName + "'=x" + agentName + "+1)",
    //  /* east south */ "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "+1)",
    //  /* south      */                                              "(y" + agentName + "'=y" + agentName + "+1)",
    //  /* south west */ "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "+1)",
    //  /* west       */ "(x" + agentName + "'=x" + agentName + "-1)",
    //  /* north west */ "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "-1)"
    //};

    //// direction specifics

    //std::size_t straightPosIndex, straightPosIndex_east, straightPosIndex_south, straightPosIndex_west, straightPosIndex_north;
    //std::string actionName, specialTransition; // if straight ahead is blocked
    //std::string positionGuard;
    //std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, e, se, s, sw, w, nw }
    //std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_north; // { n, ne, e, se, s, sw, w, nw }
    //std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_east; // { n, ne, e, se, s, sw, w, nw }
    //std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_south; // { n, ne, e, se, s, sw, w, nw }
    //std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_west; // { n, ne, e, se, s, sw, w, nw }

    //std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants;
    //std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_north;
    //std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_east;
    //std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_south;
    //std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_west;

    //switch (orientation)
    //{
    //  case SlipperyType::North:
    //    actionName = "\t[" + agentName + "move_on_slip_north]";
    //    positionGuard = "\t" + agentName + "IsOnSlipperyNorth";
    //    prob_piece_dir = { 0, 0, 1, 2, 0 /* <- R */, 2, 1, 0 };

    //    prob_piece_dir_agent_south =       { 0 , 0, 0, 1, 0 /*s <- R */, 1, 0, 0};
    //    prob_piece_dir_agent_east =     { 0,  0, 0  /*e <- R */, 2, 0, 0, 0, 0 };
    //    prob_piece_dir_agent_north =  { 0 /*n <- R */, 0, 0, 0, 2  , 0, 0, 0 };
    //    prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 2, 0  /* <- R */, 0   };

    //    prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero" };

    //    prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_zero", "prop_zero" };
    //    prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_zero", "prop_zero", "prop_displacement", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
    //    prob_piece_dir_constants_agent_south = { "prop_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
    //    prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement", "prop_zero", "prop_zero" } ;


    //    straightPosIndex = 4;
    //    straightPosIndex_east = 2;
    //    straightPosIndex_south = 4;
    //    straightPosIndex_west = 6;
    //    straightPosIndex_north = 0;

    //    specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "+1)");
    //    break;

    //  case SlipperyType::South:
    //    actionName = "\t[" + agentName + "move_on_slip_south]";
    //    positionGuard = "\t" + agentName + "IsOnSlipperySouth";

    //    prob_piece_dir = { 0 /* <- R */, 2, 1, 0, 0, 0, 1, 2 };
    //                                  // { n, ne, e, se, s, sw, w, nw }
    //    prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
    //    prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
    //    prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
    //    prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

    //    prob_piece_dir_constants =  { "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement" };

    //    prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_displacement * 1/2" };
    //    prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_displacement", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
    //    prob_piece_dir_constants_agent_south = { "prop_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
    //    prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_displacement" } ;


    //    straightPosIndex = 0; // always north
    //    straightPosIndex_east = 2;
    //    straightPosIndex_south = 4;
    //    straightPosIndex_west = 6;
    //    straightPosIndex_north = 0;
    //    specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
    //    break;

    //  case SlipperyType::East:
    //    actionName = "\t[" + agentName + "move_on_slip_east]";
    //    positionGuard = "\t" + agentName + "IsOnSlipperyEast";
    //     // { n, ne, e, se, s, sw, w, nw }

    //    prob_piece_dir = { 1, 0, 0, 0, 1, 2, 0 /* <- R */, 2 };
    //    // TODO
    //    prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
    //    prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
    //    prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
    //    prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

    //    prob_piece_dir_constants = { "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement" };

    //    prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2" };
    //    prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_displacement", "prop_zero" };
    //    prob_piece_dir_constants_agent_south = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero" } ;
    //    prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_zero", "prop_displacement * 1/2" } ;


    //    straightPosIndex = 6;
    //    straightPosIndex_east = 2;
    //    straightPosIndex_south = 4;
    //    straightPosIndex_west = 6;
    //    straightPosIndex_north = 0;

    //    specialTransition = "(x" + agentName + "'=x" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
    //    break;

    //  case SlipperyType::West:
    //    actionName = "\t[" + agentName + "move_on_slip_west]";
    //    positionGuard = "\t" + agentName + "IsOnSlipperyWest";
    //    prob_piece_dir = { 1, 2, 0 /* <- R */, 2, 1, 0, 0, 0 };
    //    // TODO
    //    // { n, ne, e, se, s, sw, w, nw }

    //    prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
    //    prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
    //    prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
    //    prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

    //    prob_piece_dir_constants = {"prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero","prop_zero", "prop_zero" };

    //    prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
    //    prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_displacement * 1/2", "prop_zero", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
    //    prob_piece_dir_constants_agent_south = { "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
    //    prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_displacement", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;


    //    straightPosIndex = 2;
    //    straightPosIndex_east = 2;
    //    straightPosIndex_south = 4;
    //    straightPosIndex_west = 6;
    //    straightPosIndex_north = 0;
    //    specialTransition = "(x" + agentName + "'=x" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "+1)");
    //    break;
    //}

    //slipperyActions.insert(actionName);

    //// override probability to 0 if corresp. direction is blocked

    //for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
    //  if (!neighborhood.at(i))
    //    prob_piece_dir.at(i) = 0;
    //}

    //// determine residual probability (R) by replacing 0 with (1 - overall sum)
    //prob_piece_dir.at(straightPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);
    //prob_piece_dir_constants.at(straightPosIndex) = "prop_intended";
    //prob_piece_dir_constants_agent_east.at(straightPosIndex_east) = "prop_intended";
    //prob_piece_dir_constants_agent_south.at(straightPosIndex_south) = "prop_intended";
    //prob_piece_dir_constants_agent_west.at(straightPosIndex_west) = "prop_intended";
    //prob_piece_dir_constants_agent_north.at(straightPosIndex_north) = "prop_intended";
    //// <DEBUG_AREA>
    //{
    //  assert(prob_piece_dir.at(straightPosIndex) <= 9 && prob_piece_dir.at(straightPosIndex) >= 3 && "Value not in Range!");
    //  assert(std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0) == PROB_PIECES && "Does not sum up to 1!");
    //  assert(orientation != SlipperyType::North || (prob_piece_dir.at(7) == 0 && prob_piece_dir.at(0) == 0 && prob_piece_dir.at(1) == 0 && "Slippery up should be impossible!"));
    //  assert(orientation != SlipperyType::South || (prob_piece_dir.at(3) == 0 && prob_piece_dir.at(4) == 0 && prob_piece_dir.at(5) == 0 && "Slippery down should be impossible!"));
    //  assert(orientation != SlipperyType::East  || (prob_piece_dir.at(1) == 0 && prob_piece_dir.at(2) == 0 && prob_piece_dir.at(3) == 0 && "Slippery right should be impossible!"));
    //  assert(orientation != SlipperyType::West  || (prob_piece_dir.at(5) == 0 && prob_piece_dir.at(6) == 0 && prob_piece_dir.at(7) == 0 && "Slippery left should be impossible!"));
    //}
    //// </DEBUG_AREA>

    //// special case: straight forward is blocked (then remain in same position)

    //positionTransition.at(straightPosIndex) = specialTransition;

    //// generic output (for every view and every possible view direction)
    //size_t current_dir = 0; // East
    //os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;

    //for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
    //  os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_east.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    //}

    //current_dir = 1; // South
    //os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;

    //for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
    //  os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_south.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    //}

    //current_dir = 2; // West

    //os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;
    //for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
    //  os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_west.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    //}


    //current_dir = 3; // North

    //os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;
    //for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
    //  os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_north.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    //}

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

  std::string PrismModulesPrinter::moveGuard(const AgentName &agentName) const {
    return isGame() ? " move=" + std::to_string(agentIndexMap.at(agentName)) + " & " : " ";
  }

  std::string PrismModulesPrinter::faultyBehaviourGuard(const AgentName &agentName, const ActionId &actionId) const {
    if(faultyBehaviour()) {
      if(actionId == NOFAULT) {
        return "(previousAction" + agentName + "=" + std::to_string(NOFAULT) + ") & ";
      } else {
        return "(previousAction" + agentName + "=" + std::to_string(NOFAULT) + " | previousAction" + agentName + "=" + std::to_string(actionId) + ") & ";
      }
    } else {
      return "";
    }
  }

  std::string PrismModulesPrinter::faultyBehaviourUpdate(const AgentName &agentName, const ActionId &actionId) const {
    return "(previousAction" + agentName + "'=" + std::to_string(actionId) + ")";
  }

  std::string PrismModulesPrinter::moveUpdate(const AgentName &agentName) const {
    size_t agentIndex = agentIndexMap.at(agentName);
    return isGame() ?
             (agentIndex == numberOfPlayer - 1) ?
               " & (move'=0) " :
               " & (move'=" + std::to_string(agentIndex + 1) + ") " :
               "";
  }

  std::string PrismModulesPrinter::updatesToString(const updates &updates, const AgentName &a, const ActionId &actionId) const {
    if(updates.empty()) return "true";
    std::string updatesString = "";
    bool first = true;
    for(auto const update : updates) {
      if(first) first = false;
      else updatesString += " + ";
      updatesString += updateToString(update);
      if(faultyBehaviour()) updatesString += "&" + faultyBehaviourUpdate(a, actionId);
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
