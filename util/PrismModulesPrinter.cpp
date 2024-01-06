#include "PrismModulesPrinter.h"

#include <map>
#include <string>

namespace prism {

  PrismModulesPrinter::PrismModulesPrinter(std::ostream& os, const ModelType &modelType, const coordinates &maxBoundaries, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const AgentNameAndPositionMap &agentNameAndPositionMap, std::vector<Configuration> config, const bool enforceOneWays)
    : os(os), modelType(modelType), maxBoundaries(maxBoundaries), boxes(boxes), balls(balls), lockedDoors(lockedDoors), unlockedDoors(unlockedDoors), keys(keys), agentNameAndPositionMap(agentNameAndPositionMap), enforceOneWays(enforceOneWays), configuration(config), viewDirectionMapping({{0, "East"}, {1, "South"}, {2, "West"}, {3, "North"}}) {
      numberOfPlayer = agentNameAndPositionMap.size();
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


  std::ostream& PrismModulesPrinter::printBooleansForKeys(std::ostream &os, const AgentName &agentName, const cells &keys) {
    for(auto const& key : keys) {
      os << "\t" << agentName << "_has_"<< key.getColor() << "_key : bool;\n";//init false;\n";
    }
    os << "\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printBooleansForBackground(std::ostream &os, const AgentName &agentName, const std::map<Color, cells> &backgroundTiles) {
    for(auto const& [color, cells] : backgroundTiles) {
      if(cells.size() == 0) continue;
      std::string c = getColor(color);
      c.at(0) = std::toupper(c.at(0));
      os << "\t" << agentName << "_picked_up_" << c << " : bool init false;\n";
    }
    os << "\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printActionsForKeys(std::ostream &os, const AgentName &agentName, const cells &keys) {
    for(auto const& key : keys) { // TODO ADD Drop action and enforce that pickup only possible if pockets empty (nothing picked up already)
      os << "\n";
      std::string keyColor = key.getColor();
      os << "\t[pickup_" << keyColor << "_key]\t" << pickupGuard(agentName, keyColor) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=true) & (" << agentName << "_is_carrying_object'=true);\n";
      os << "\n";

      os << "\t[drop_" << keyColor << "_key_north]\t" << dropGuard(agentName, keyColor, 3) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=false) & (" << agentName << "_is_carrying_object'=false);\n";

      os << "\t[drop_" << keyColor << "_key_west]\t" << dropGuard(agentName, keyColor, 2) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=false) & (" << agentName << "_is_carrying_object'=false);\n";

      os << "\t[drop_" << keyColor << "_key_south]\t" << dropGuard(agentName, keyColor, 1) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=false) & (" << agentName << "_is_carrying_object'=false);\n";

      os << "\t[drop_" << keyColor << "_key_east]\t" << dropGuard(agentName, keyColor, 0) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=false) & (" << agentName << "_is_carrying_object'=false);\n";
    }

    return os;
  }

  std::string PrismModulesPrinter::pickupGuard(const AgentName &agentName, const std::string keyColor ) {
    return "!" + agentName + "_is_carrying_object &\t" + agentName + "CanPickUp" + keyColor + "Key ";
  }

  std::string PrismModulesPrinter::dropGuard(const AgentName &agentName, const std::string keyColor, size_t view) {
    return viewVariable(agentName, view, true) + "\t!" + agentName + "CannotMove" + viewDirectionMapping.at(view) + "\t&\t" + agentName + "_has_" + keyColor + "_key\t";
  }

  std::ostream& PrismModulesPrinter::printActionsForBackground(std::ostream &os, const AgentName &agentName, const std::map<Color, cells> &backgroundTiles) {
    for(auto const& [color, cells] : backgroundTiles) {
      if(cells.size() == 0) continue;
      std::string c = getColor(color);
      c.at(0) = std::toupper(c.at(0));
      os << "\t[" << agentName << "_pickup_" << c << "] " << agentName << "On" << c << " & !" << agentName << "_picked_up_" << c << " -> ";
      os << "(" << agentName << "_picked_up_" << c << "'=true);\n";
    }
    os << "\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printInitStruct(std::ostream &os, const AgentNameAndPositionMap &agents, const KeyNameAndPositionMap &keys, const cells &lockedDoors, const cells &unlockedDoors, prism::ModelType modelType) {
    os << "init\n";
    os << "\t";

    bool first = true;
    for (auto const& agent : agents) {
      if (first) first = false;
      else os << " & ";
      os << "(!" << agent.first << "IsInGoal & !" << agent.first << "IsInLava & !" << agent.first << "Done & !" << agent.first << "IsOnWall & ";
      os << "x" << agent.first << "=" << agent.second.second << " & y" << agent.first << "=" << agent.second.first << ")";
      os << " & !" << agent.first << "_is_carrying_object";
      if(enforceOneWays) {
        os << " & ( !AgentCannotTurn ) ";
      } else {
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


    return os;
  }

  std::ostream& PrismModulesPrinter::printModule(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &boundaries, const coordinates& initialPosition, const cells &keys, const std::map<Color, cells> &backgroundTiles, const bool agentWithView, const std::vector<float> &probabilities, const double faultyProbability) {
    os << "module " << agentName << "\n";
    os << "\tx" << agentName << " : [1.." << boundaries.second  << "];\n";
    os << "\ty" << agentName << " : [1.." << boundaries.first << "];\n";
    os << "\t" << agentName << "_is_carrying_object : bool;\n";

    printBooleansForKeys(os, agentName, keys);
    printBooleansForBackground(os, agentName, backgroundTiles);
    os << "\t" << agentName << "Done : bool;\n";
    if(agentWithView) {
      os << "\tview" << agentName << " : [0..3];\n";
    os << "\n";
    if (faultyProbability != 0.0) {
      os << "\t[" << agentName << "_turn_right] " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery -> " << 100 - faultyProbability <<  "/100:" << "(view" << agentName << "'=mod(view" << agentName << " + 1, 4))" << moveUpdate(agentIndex) << "\n + " << faultyProbability <<  "/100:" << "(view" << agentName << "'=mod(view" << agentName << " + 2, 4))" << ";\n";
      os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << ">1 -> " << 100 - faultyProbability <<  "/100:" << "(view" << agentName << "'=mod(view" << agentName << " - 1, 4))" << moveUpdate(agentIndex) << "\n + " << faultyProbability <<  "/100:" << "(view" << agentName << "'=mod(view" << agentName << " - 2, 4))" << ";\n";
      os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << "=0 -> " << 100 - faultyProbability <<  "/100:" << "(view" << agentName << "'=3)" << moveUpdate(agentIndex) << "\n + " << faultyProbability <<  "/100:" << "(view" << agentName << "'=2)" << ";\n";
      os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << "=1 -> " << 100 - faultyProbability <<  "/100:" << "(view" << agentName << "'=0)" << moveUpdate(agentIndex) << "\n + " << faultyProbability <<  "/100:" << "(view" << agentName << "'=3)" << ";\n";
    } else {
      os << "\t[" << agentName << "_turn_right] " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery -> (view" << agentName << "'=mod(view" << agentName << " + 1, 4)) " << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << ">0 -> (view" << agentName << "'=view" << agentName << " - 1) " << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << "=0 -> (view" << agentName << "'=3) " << moveUpdate(agentIndex) << ";\n";
    }
      if(enforceOneWays) {
        os << "\t[" << agentName << "_stuck]        !" << agentName << "IsFixed & " << agentName << "CannotTurn & view" << agentName << " = 0 & " << agentName << "CannotMoveEast -> true;\n";
        os << "\t[" << agentName << "_stuck]        !" << agentName << "IsFixed & " << agentName << "CannotTurn & view" << agentName << " = 1 & " << agentName << "CannotMoveSouth -> true;\n";
        os << "\t[" << agentName << "_stuck]        !" << agentName << "IsFixed & " << agentName << "CannotTurn & view" << agentName << " = 2 & " << agentName << "CannotMoveWest -> true;\n";
        os << "\t[" << agentName << "_stuck]        !" << agentName << "IsFixed & " << agentName << "CannotTurn & view" << agentName << " = 3 & " << agentName << "CannotMoveNorth -> true;\n";
      }
    } else {
      os << "\t[" << agentName << "_turns]  " << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " <<  moveGuard(agentIndex) << " true -> (x" << agentName << "'=x" << agentName << ")" << moveUpdate(agentIndex) << ";\n";
    }
    printActionsForKeys(os, agentName, keys);
    printActionsForBackground(os, agentName, backgroundTiles);
    os << "\n";

    printMovementActions(os, agentName, agentIndex, agentWithView, 1.0, faultyProbability);
    for(auto const& probability : probabilities) {
      printMovementActions(os, agentName, agentIndex, agentWithView, probability);
    }
    printDoneActions(os, agentName, agentIndex);

    printConfiguredActions(os, agentName);

    os << "\n";
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

    os << "\n" << actionStream.str();
    os << "endmodule\n\n";
  }

  void PrismModulesPrinter::printPortableObjectActionsForRobot(const std::string &a, const std::string &i) {
    actionStream << "\t[" << a << "_pickup_" << i << "] " << a << "CannotMove" << i << " -> (" << a << "Carrying" << i << "'=true);\n";
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
    actionStream << "\t[" << agentName << "_close_" << identifier << "] " << agentName << "IsNextTo"   << identifier << " & " << agentName << "Carrying" << key << " -> true;\n";
    actionStream << "\n";
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

  std::ostream& PrismModulesPrinter::printMovementActions(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const bool agentWithView, const float &probability, const double &stickyProbability) {
    if(stickyProbability != 0.0) {
      os << "\t[" << agentName << "_move_north]" << moveGuard(agentIndex) << viewVariable(agentName, 3, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveNorth -> " << (100 - stickyProbability) << "/100:" << "(y" << agentName << "'=y" << agentName << "-1)" << moveUpdate(agentIndex) << "\n+ " << (stickyProbability) << "/100:" << "(y" << agentName << "'=max(y" << agentName << "-2, 1 ))" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_east] " << moveGuard(agentIndex) << viewVariable(agentName, 0, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveEast  -> " << (100 - stickyProbability) << "/100:" << "(x" << agentName << "'=x" << agentName << "+1)" << moveUpdate(agentIndex) << "\n+ " << (stickyProbability) << "/100:" << "(x" << agentName << "'=min(x" << agentName << "+2, width))" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_south]" << moveGuard(agentIndex) << viewVariable(agentName, 1, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveSouth -> " << (100 - stickyProbability) << "/100:" << "(y" << agentName << "'=y" << agentName << "+1)" << moveUpdate(agentIndex) << "\n+ " << (stickyProbability) << "/100:" << "(y" << agentName << "'=min(y" << agentName << "+2, height))" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_west] " << moveGuard(agentIndex) << viewVariable(agentName, 2, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveWest  -> " << (100 - stickyProbability) << "/100:" << "(x" << agentName << "'=x" << agentName << "-1)" << moveUpdate(agentIndex) << "\n+ " << (stickyProbability) << "/100:" << "(x" << agentName << "'=max(x" << agentName << "-1, 1))" << moveUpdate(agentIndex) << ";\n";
    }
    else if(probability >= 1) {
      os << "\t[" << agentName << "_move_north]" << moveGuard(agentIndex) << viewVariable(agentName, 3, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveNorth -> (y" << agentName << "'=y" << agentName << "-1)" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_east] " << moveGuard(agentIndex) << viewVariable(agentName, 0, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveEast  -> (x" << agentName << "'=x" << agentName << "+1)" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_south]" << moveGuard(agentIndex) << viewVariable(agentName, 1, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveSouth -> (y" << agentName << "'=y" << agentName << "+1)" << moveUpdate(agentIndex) << ";\n";
      os << "\t[" << agentName << "_move_west] " << moveGuard(agentIndex) << viewVariable(agentName, 2, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava &!" << agentName << "IsInGoal &  !" << agentName << "CannotMoveWest  -> (x" << agentName << "'=x" << agentName << "-1)" << moveUpdate(agentIndex) << ";\n";
    } else {
      std::string probabilityString = std::to_string(probability);
      std::string percentageString = std::to_string((int)(100 * probability));
      std::string complementProbabilityString = std::to_string(1 - probability);
      os << "\t[" << agentName << "_move_north_" << percentageString << "] ";
      os << moveGuard(agentIndex) << viewVariable(agentName, 3, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava & !" << agentName << "CannotMoveNorth -> ";
      os << probabilityString << ": (y" << agentName << "'=y" << agentName << "-1)" << moveUpdate(agentIndex) << " + ";
      os << complementProbabilityString << ": (y" << agentName << "'=y" << agentName << ") " << moveUpdate(agentIndex) << ";\n";

      os << "\t[" << agentName << "_move_east_" << percentageString << "] ";
      os << moveGuard(agentIndex) << viewVariable(agentName, 0, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava & !" << agentName << "CannotMoveEast  -> ";
      os << probabilityString << ": (x" << agentName << "'=x" << agentName << "+1)" << moveUpdate(agentIndex) << " + ";
      os << complementProbabilityString << ": (x" << agentName << "'=x" << agentName << ") " << moveUpdate(agentIndex) << ";\n";

      os << "\t[" << agentName << "_move_south_" << percentageString << "] ";
      os << moveGuard(agentIndex) << viewVariable(agentName, 1, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava & !" << agentName << "CannotMoveSouth -> ";
      os << probabilityString << ": (y" << agentName << "'=y" << agentName << "+1)" << moveUpdate(agentIndex) << " + ";
      os << complementProbabilityString << ": (y" << agentName << "'=y" << agentName << ") " << moveUpdate(agentIndex) << ";\n";

      os << "\t[" << agentName << "_move_west_" << percentageString << "] ";
      os << moveGuard(agentIndex) << viewVariable(agentName, 2, agentWithView) << " !" << agentName << "IsFixed & " << " !" << agentName << "IsOnSlippery & !" << agentName << "IsInLava & !" << agentName << "CannotMoveWest  -> ";
      os << probabilityString << ": (x" << agentName << "'=x" << agentName << "-1)" << moveUpdate(agentIndex) << " + ";
      os << complementProbabilityString << ": (x" << agentName << "'=x" << agentName << ") " << moveUpdate(agentIndex) << ";\n";
    }
    return os;
  }

  std::ostream& PrismModulesPrinter::printDoneActions(std::ostream &os, const AgentName &agentName, const size_t &agentIndex) {
    os << "\t[" << agentName << "_done]" << moveGuard(agentIndex) << agentName << "IsInGoal | " << agentName << "IsInLava -> (" << agentName << "Done'=true);\n";
    return os;
  }

   std::ostream& PrismModulesPrinter::printSlipperyTurn(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation) {
    constexpr std::size_t PROB_PIECES = 9, ALL_POSS_DIRECTIONS = 9;

    std::array<std::string, ALL_POSS_DIRECTIONS> positionTransition = {
      /* north      */                                                "(y" + agentName + "'=y" + agentName + "-1)",
      /* north east */   "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "-1)",
      /* east       */   "(x" + agentName + "'=x" + agentName + "+1)",
      /* east south */   "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "+1)",
      /* south      */                                                "(y" + agentName + "'=y" + agentName + "+1)",
      /* south west */   "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "+1)",
      /* west       */   "(x" + agentName + "'=x" + agentName + "-1)",
      /* north west */   "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "-1)",
      /* own position */ "(x" + agentName + "'=x" + agentName + ") & (y" + agentName + "'=y" + agentName + ")"
    };

    // view transition appdx in form (guard, update part)
    // IMPORTANT: No mod() usage for turn left due to bug in mod() function for decrement


    std::array<std::tuple<std::string, std::string, std::string>, 3> viewTransition = {
        std::make_tuple(" & " + agentName + "SlipperyTurnRightAllowed ", " & (view" + agentName + "'=mod(view" + agentName + " + 1, 4))", "_right]"),
        std::make_tuple(" & " + agentName + "SlipperyTurnLeftAllowed & view" + agentName + ">0", " & (view" + agentName + "'=view" + agentName + " - 1)", "_left]"),
        std::make_tuple(" & " + agentName + "SlipperyTurnLeftAllowed & view" + agentName + "=0", " & (view" + agentName + "'=3)", "_left]")
    };

    // direction specifics

    std::string actionName;
    std::string positionGuard;
    std::size_t remainPosIndex = 8;
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, e, se, s, sw, w, nw, CURRENT POS }
    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants;

    switch (orientation)
    {
      case SlipperyType::North:
        actionName = "\t[" + agentName + "turn_at_slip_north";
        positionGuard = "\t" + agentName + "IsOnSlipperyNorth";
        prob_piece_dir = { 0, 0, 0, 0, 1, 0, 0, 0, 0 /* <- R */ };
        prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_turn_displacement" /* <- R */, "prop_zero", "prop_zero", "prop_zero","prop_zero" };
        break;

      case SlipperyType::South:
        actionName = "\t[" + agentName + "turn_at_slip_south";
        positionGuard = "\t" + agentName + "IsOnSlipperySouth";
        prob_piece_dir = { 1, 0, 0, 0, 0, 0, 0, 0, 0 /* <- R */ };
        prob_piece_dir_constants = { "prop_turn_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" };
        break;

      case SlipperyType::East:
        actionName = "\t[" + agentName + "turn_at_slip_east";
        positionGuard = "\t" + agentName + "IsOnSlipperyEast";
        prob_piece_dir = { 0, 0, 0, 0, 0, 0, 1, 0, 0 /* <- R */ };
        prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_turn_displacement", "prop_zero", "prop_zero" };
        break;

      case SlipperyType::West:
        actionName = "\t[" + agentName + "turn_at_slip_west";
        positionGuard = "\t" + agentName + "IsOnSlipperyWest";
        prob_piece_dir = { 0, 0, 1, 0, 0, 0, 0, 0, 0 /* <- R */ };
        prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_turn_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" };
        break;
    }

    slipperyActions.insert(actionName + "_left]");
    slipperyActions.insert(actionName + "_right]");

    // override probability to 0 if corresp. direction is blocked

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS - 1; i++) {
      if (!neighborhood.at(i))
        prob_piece_dir.at(i) = 0;
    }

    // determine residual probability (R) by replacing 0 with (1 - overall sum)

    prob_piece_dir.at(remainPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);
    prob_piece_dir_constants.at(remainPosIndex) = "prop_turn_intended";
    // <DEBUG_AREA>
    {
      assert(prob_piece_dir.at(remainPosIndex) <= 9 && prob_piece_dir.at(remainPosIndex) >= 6 && "Value not in Range!");
      assert(std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0) == PROB_PIECES && "Does not sum up to 1!");
    }
    // </DEBUG_AREA>

    // generic output (for every view transition)
    for (std::size_t v = 0; v < viewTransition.size(); v++) {
        os << actionName << std::get<2>(viewTransition.at(v)) << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << std::get<0>(viewTransition.at(v));
        for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
          if (i == remainPosIndex) {
          os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants.at(i) << " : " << positionTransition.at(i) << std::get<1>(viewTransition.at(v)) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
          } else {
            os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
          }
        }
    }

    return os;
  }

  std::ostream& PrismModulesPrinter::printSlipperyMove(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation) {
    constexpr std::size_t PROB_PIECES = 9, ALL_POSS_DIRECTIONS = 8;

    std::array<std::string, ALL_POSS_DIRECTIONS> positionTransition = {
      /* north      */                                              "(y" + agentName + "'=y" + agentName + "-1)",
      /* north east */ "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "-1)",
      /* east       */ "(x" + agentName + "'=x" + agentName + "+1)",
      /* east south */ "(x" + agentName + "'=x" + agentName + "+1) & (y" + agentName + "'=y" + agentName + "+1)",
      /* south      */                                              "(y" + agentName + "'=y" + agentName + "+1)",
      /* south west */ "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "+1)",
      /* west       */ "(x" + agentName + "'=x" + agentName + "-1)",
      /* north west */ "(x" + agentName + "'=x" + agentName + "-1) & (y" + agentName + "'=y" + agentName + "-1)"
    };

    // direction specifics

    std::size_t straightPosIndex, straightPosIndex_east, straightPosIndex_south, straightPosIndex_west, straightPosIndex_north;
    std::string actionName, specialTransition; // if straight ahead is blocked
    std::string positionGuard;
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, e, se, s, sw, w, nw }
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_north; // { n, ne, e, se, s, sw, w, nw }
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_east; // { n, ne, e, se, s, sw, w, nw }
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_south; // { n, ne, e, se, s, sw, w, nw }
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir_agent_west; // { n, ne, e, se, s, sw, w, nw }

    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants;
    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_north;
    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_east;
    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_south;
    std::array<std::string, ALL_POSS_DIRECTIONS> prob_piece_dir_constants_agent_west;

    switch (orientation)
    {
      case SlipperyType::North:
        actionName = "\t[" + agentName + "move_on_slip_north]";
        positionGuard = "\t" + agentName + "IsOnSlipperyNorth";
        prob_piece_dir = { 0, 0, 1, 2, 0 /* <- R */, 2, 1, 0 };

        prob_piece_dir_agent_south =       { 0 , 0, 0, 1, 0 /*s <- R */, 1, 0, 0};
        prob_piece_dir_agent_east =     { 0,  0, 0  /*e <- R */, 2, 0, 0, 0, 0 };
        prob_piece_dir_agent_north =  { 0 /*n <- R */, 0, 0, 0, 2  , 0, 0, 0 };
        prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 2, 0  /* <- R */, 0   };

        prob_piece_dir_constants = { "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero" };

        prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_zero", "prop_zero" };
        prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_zero", "prop_zero", "prop_displacement", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
        prob_piece_dir_constants_agent_south = { "prop_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
        prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement", "prop_zero", "prop_zero" } ;


        straightPosIndex = 4;
        straightPosIndex_east = 2;
        straightPosIndex_south = 4;
        straightPosIndex_west = 6;
        straightPosIndex_north = 0;

        specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "+1)");
        break;

      case SlipperyType::South:
        actionName = "\t[" + agentName + "move_on_slip_south]";
        positionGuard = "\t" + agentName + "IsOnSlipperySouth";

        prob_piece_dir = { 0 /* <- R */, 2, 1, 0, 0, 0, 1, 2 };
                                      // { n, ne, e, se, s, sw, w, nw }
        prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
        prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
        prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
        prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

        prob_piece_dir_constants =  { "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement" };

        prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_displacement * 1/2" };
        prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_displacement", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
        prob_piece_dir_constants_agent_south = { "prop_displacement", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
        prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_displacement" } ;


        straightPosIndex = 0; // always north
        straightPosIndex_east = 2;
        straightPosIndex_south = 4;
        straightPosIndex_west = 6;
        straightPosIndex_north = 0;
        specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
        break;

      case SlipperyType::East:
        actionName = "\t[" + agentName + "move_on_slip_east]";
        positionGuard = "\t" + agentName + "IsOnSlipperyEast";
         // { n, ne, e, se, s, sw, w, nw }

        prob_piece_dir = { 1, 0, 0, 0, 1, 2, 0 /* <- R */, 2 };
        // TODO
        prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
        prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
        prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
        prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

        prob_piece_dir_constants = { "prop_displacement * 1/2", "prop_zero", "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement" };

        prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2" };
        prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_displacement", "prop_zero" };
        prob_piece_dir_constants_agent_south = { "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero" } ;
        prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_zero", "prop_zero", "prop_zero" /* <- R */, "prop_displacement * 1/2", "prop_zero", "prop_displacement * 1/2" } ;


        straightPosIndex = 6;
        straightPosIndex_east = 2;
        straightPosIndex_south = 4;
        straightPosIndex_west = 6;
        straightPosIndex_north = 0;

        specialTransition = "(x" + agentName + "'=x" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
        break;

      case SlipperyType::West:
        actionName = "\t[" + agentName + "move_on_slip_west]";
        positionGuard = "\t" + agentName + "IsOnSlipperyWest";
        prob_piece_dir = { 1, 2, 0 /* <- R */, 2, 1, 0, 0, 0 };
        // TODO
        // { n, ne, e, se, s, sw, w, nw }

        prob_piece_dir_agent_north =       { 0 /*n <- R */, 1, 0, 0, 0, 0, 0, 1};
        prob_piece_dir_agent_east =     { 0, 2, 0  /*e <- R */, 0, 0, 0, 0, 0 };
        prob_piece_dir_agent_south =  { 2, 0, 0, 0, 0  /*s <- R */, 0, 0, 0 };
        prob_piece_dir_agent_west = { 0, 0, 0, 0, 0, 0, 0 /* <- R */, 2   };

        prob_piece_dir_constants = {"prop_displacement * 1/2", "prop_displacement", "prop_zero" /* <- R */, "prop_displacement", "prop_displacement * 1/2", "prop_zero","prop_zero", "prop_zero" };

        prob_piece_dir_constants_agent_north =      { "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
        prob_piece_dir_constants_agent_east =    { "prop_zero", "prop_displacement * 1/2", "prop_zero", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" };
        prob_piece_dir_constants_agent_south = { "prop_zero", "prop_zero", "prop_displacement * 1/2", "prop_displacement * 1/2", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;
        prob_piece_dir_constants_agent_west ={ "prop_zero", "prop_zero", "prop_displacement", "prop_zero", "prop_zero" /* <- R */, "prop_zero", "prop_zero", "prop_zero" } ;


        straightPosIndex = 2;
        straightPosIndex_east = 2;
        straightPosIndex_south = 4;
        straightPosIndex_west = 6;
        straightPosIndex_north = 0;
        specialTransition = "(x" + agentName + "'=x" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "+1)");
        break;
    }

    slipperyActions.insert(actionName);

    // override probability to 0 if corresp. direction is blocked

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      if (!neighborhood.at(i))
        prob_piece_dir.at(i) = 0;
    }

    // determine residual probability (R) by replacing 0 with (1 - overall sum)
    if(enforceOneWays) {
      prob_piece_dir = {0,0,0,0,0,0,0,0};
      prob_piece_dir_constants = {"zero","zero","zero","zero","zero","zero","zero","zero"};
    }
    prob_piece_dir.at(straightPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);
    prob_piece_dir_constants.at(straightPosIndex) = "prop_intended";
    prob_piece_dir_constants_agent_east.at(straightPosIndex_east) = "prop_intended";
    prob_piece_dir_constants_agent_south.at(straightPosIndex_south) = "prop_intended";
    prob_piece_dir_constants_agent_west.at(straightPosIndex_west) = "prop_intended";
    prob_piece_dir_constants_agent_north.at(straightPosIndex_north) = "prop_intended";
    // <DEBUG_AREA>
    {
      assert(prob_piece_dir.at(straightPosIndex) <= 9 && prob_piece_dir.at(straightPosIndex) >= 3 && "Value not in Range!");
      assert(std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0) == PROB_PIECES && "Does not sum up to 1!");
      assert(orientation != SlipperyType::North || (prob_piece_dir.at(7) == 0 && prob_piece_dir.at(0) == 0 && prob_piece_dir.at(1) == 0 && "Slippery up should be impossible!"));
      assert(orientation != SlipperyType::South || (prob_piece_dir.at(3) == 0 && prob_piece_dir.at(4) == 0 && prob_piece_dir.at(5) == 0 && "Slippery down should be impossible!"));
      assert(orientation != SlipperyType::East  || (prob_piece_dir.at(1) == 0 && prob_piece_dir.at(2) == 0 && prob_piece_dir.at(3) == 0 && "Slippery right should be impossible!"));
      assert(orientation != SlipperyType::West  || (prob_piece_dir.at(5) == 0 && prob_piece_dir.at(6) == 0 && prob_piece_dir.at(7) == 0 && "Slippery left should be impossible!"));
    }
    // </DEBUG_AREA>

    // special case: straight forward is blocked (then remain in same position)

    positionTransition.at(straightPosIndex) = specialTransition;

    // generic output (for every view and every possible view direction)
    size_t current_dir = 0; // East
    os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_east.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    }

    current_dir = 1; // South
    os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_south.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    }

    current_dir = 2; // West

    os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;
    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_west.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    }


    current_dir = 3; // North

    os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed " << "& " << "view" << agentName << "=" << current_dir;
    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      os << (i == 0 ? " -> " : " + ") << prob_piece_dir_constants_agent_north.at(i) << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
    }

    return os;
  }

  std::ostream& PrismModulesPrinter::printEndmodule(std::ostream &os) {
    os << "endmodule\n";
    os << "\n";
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

  std::string PrismModulesPrinter::moveGuard(const size_t &agentIndex) {
    return isGame() ? " move=" + std::to_string(agentIndex) + " & " : " ";
  }

  std::string PrismModulesPrinter::moveUpdate(const size_t &agentIndex) {
    return isGame() ?
             (agentIndex == numberOfPlayer - 1) ?
               " & (move'=0) " :
               " & (move'=" + std::to_string(agentIndex + 1) + ") " :
               "";
  }

  std::string PrismModulesPrinter::viewVariable(const AgentName &agentName, const size_t &agentDirection, const bool agentWithView) {
    return agentWithView ? " view" + agentName + "=" + std::to_string(agentDirection) + " & " : " ";
  }

  bool PrismModulesPrinter::isGame() const {
    return modelType == ModelType::SMG;
  }
}
