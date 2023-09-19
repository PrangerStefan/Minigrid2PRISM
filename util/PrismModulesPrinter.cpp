#include "PrismModulesPrinter.h"

#include <map>
#include <string>

namespace prism {

  PrismModulesPrinter::PrismModulesPrinter(const ModelType &modelType, const size_t &numberOfPlayer, std::vector<Configuration> config, const bool enforceOneWays)
    : modelType(modelType), numberOfPlayer(numberOfPlayer), enforceOneWays(enforceOneWays), configuration(config), viewDirectionMapping({{0, "East"}, {1, "South"}, {2, "West"}, {3, "North"}}) {
  }

  std::ostream& PrismModulesPrinter::printModel(std::ostream &os, const ModelType &modelType) {
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

  std::ostream& PrismModulesPrinter::printBackgroundLabels(std::ostream &os, const AgentName &agentName, const std::pair<Color, cells> &backgroundTiles) {
    if(backgroundTiles.second.size() == 0) return os;

    bool first = true;
    std::string color = getColor(backgroundTiles.first);
    color.at(0) = std::toupper(color.at(0));
    os << "formula " << agentName << "On" << color << " = ";
    for(auto const& cell : backgroundTiles.second) {
      if(first) first = false; else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << ";\n";
    os << "label \"" << agentName << "On" << color << "\" = " << agentName << "On" << color << ";\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printRestrictionFormula(std::ostream& os, const AgentName &agentName, const std::string &direction,  const cells &cells) {
    bool first = true;
    os << "formula " << agentName << "CannotMove" << direction << " = " ;
    for(auto const& cell : cells) {
      if(first) first = false;
      else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << " | " << agentName << "CannotMove" << direction << "BecauseOfKey";
    os << ";\n";
    return os;
  }
  
  std::ostream& PrismModulesPrinter::printKeyRestrictionFormula(std::ostream& os, const AgentName &agentName, const std::string &direction, const cells &keys) {
    bool first = true;
    os << "formula " << agentName << "CannotMove" << direction << "BecauseOfKey" << " = ";
    for (auto const& key : keys) {
      if(first) first = false;
      else os << " | ";
      std::string keyColor = key.getColor();
      std::string xKey = "xKey" + keyColor;
      std::string yKey = "yKey" + keyColor;
      coordinates coords;
      
      os << "!" << agentName << "_has_" << keyColor << "_key & ";

      if (direction == "North") {
        os << " (x" << agentName << "   = " << xKey << "&y" << agentName << "-1 = " << yKey << ")";
      } else if (direction == "South") {
        os << " (x" << agentName << "   = " << xKey << "&y" << agentName << "+1 = " << yKey << ")";
      } else if (direction == "East") {
        os << " (x" << agentName << "+1 = " << xKey << "&y" << agentName << "   = " << yKey << ")";
      } else if (direction == "West") {
        os << " (x" << agentName << "-1 = " << xKey << "&y" << agentName << "   = " << yKey << ")";
      } else {
        os << "Invalid Direction! in Key Restriction";
      }
    }

    os << ";\n";
    return os;
  }



  std::ostream& PrismModulesPrinter::printIsOnSlipperyFormula(std::ostream& os, const AgentName &agentName, const std::vector<std::reference_wrapper<cells>> &slipperyCollection, const cells &slipperyNorth, const cells &slipperyEast, const cells &slipperySouth, const cells &slipperyWest) {
    if(std::find_if(slipperyCollection.cbegin(), slipperyCollection.cend(), [=](const std::reference_wrapper<cells>& c) { return !c.get().empty(); }) == slipperyCollection.cend()) {
      os << "formula " << agentName << "IsOnSlippery = false;\n";
      return os;
    }

    bool first = true;
    os << "formula " << agentName << "IsOnSlippery = ";

    for (const auto& slippery: slipperyCollection) {
      for(const auto& cell : slippery.get()) {
        if(first) first = false; else os << " | ";
        os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
      }
    }
    os << ";\n";
    if(enforceOneWays) {
      first = true;
      os << "formula " << agentName << "IsOnSlipperyNorth = ";

      for (const auto& cell: slipperyNorth) {
        if(first) first = false; else os << " | ";
        os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
      }
      os << ";\n";
      first = true;
      os << "formula " << agentName << "IsOnSlipperyEast = ";

      for (const auto& cell: slipperyEast) {
        if(first) first = false; else os << " | ";
        os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
      }
      os << ";\n";
      first = true;
      os << "formula " << agentName << "IsOnSlipperySouth = ";

      for (const auto& cell: slipperySouth) {
        if(first) first = false; else os << " | ";
        os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
      }
      os << ";\n";
      first = true;
      os << "formula " << agentName << "IsOnSlipperyWest = ";
      for (const auto& cell: slipperyWest) {
        if(first) first = false; else os << " | ";
        os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
      }
      os << ";\n";
    }
    return os;
  }

  std::ostream& PrismModulesPrinter::printIsInLavaFormula(std::ostream& os, const AgentName &agentName, const cells &lava) {
    if(lava.size() == 0) {
      os << "formula " << agentName << "IsInLava = false;\n";
      return os;
    }
    bool first = true;
    os << "formula " << agentName << "IsInLava = ";
    for(auto const& cell : lava) {
      if(first) first = false; else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << ";\n";
    os << "formula " << agentName << "IsInLavaAndNotDone = " << agentName << "IsInLava & !" << agentName << "Done;\n";
    os << "label \"" << agentName << "IsInLavaAndNotDone\" = " << agentName << "IsInLava & !" << agentName << "Done;\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printTurningNotAllowedFormulas(std::ostream& os, const AgentName &agentName, const cells &noTurnFloor) {
    if( (!enforceOneWays or noTurnFloor.size() == 0) or (noTurnFloor.size() == 0) ) {
      os << "formula " << agentName << "CannotTurn = false;\n";
      return os;
    }
    bool first = true;
    os << "formula " << agentName << "CannotTurn = ";
    for(auto const& cell : noTurnFloor) {
      if(first) first = false; else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << " | " << agentName << "IsOnSlippery;\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printIsFixedFormulas(std::ostream& os, const AgentName &agentName) {
    os << "formula " << agentName << "IsFixed = false;\n";
    os << "formula " << agentName << "SlipperyTurnLeftAllowed = true;\n";
    os << "formula " << agentName << "SlipperyTurnRightAllowed = true;\n";
    os << "formula " << agentName << "SlipperyMoveForwardAllowed = true;\n";
    os << "label \"FixedStates\" = " << agentName << "IsFixed | !" << agentName << "SlipperyTurnRightAllowed | !" << agentName << "SlipperyTurnLeftAllowed | !" << agentName << "SlipperyMoveForwardAllowed | " << agentName << "IsInGoal | " << agentName << "IsInLava";
    if(enforceOneWays) {
      os << " | " << agentName << "CannotTurn";
    }
    os << ";\n";
    //os << "label \"FixedStates\" = " << agentName << "IsFixed | " << agentName << "IsOnSlippery  | " << agentName << "IsInGoal | " << agentName << "IsInLava;\n";

    return os;
  }


  std::ostream& PrismModulesPrinter::printWallFormula(std::ostream& os, const AgentName &agentName, const cells &walls) {
    os << "formula " << agentName << "IsOnWall = ";
    bool first = true;
    for(auto const& cell : walls) {
      if(first) first = false; else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << ";\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printFormulas(std::ostream& os,
                              const AgentName &agentName,
                              const cells &restrictionNorth,
                              const cells &restrictionEast,
                              const cells &restrictionSouth,
                              const cells &restrictionWest,
                              const std::vector<std::reference_wrapper<cells>> &slipperyCollection,
                              const cells &lava,
                              const cells &walls,
                              const cells &noTurnFloor,
                              const cells &slipperyNorth,
                              const cells &slipperyEast,
                              const cells &slipperySouth,
                              const cells &slipperyWest,
                              const cells &keys) {
    printRestrictionFormula(os, agentName, "North", restrictionNorth);
    printRestrictionFormula(os, agentName, "East", restrictionEast);
    printRestrictionFormula(os, agentName, "South", restrictionSouth);
    printRestrictionFormula(os, agentName, "West", restrictionWest);
    
    if (!keys.empty()) {
      printKeyRestrictionFormula(os, agentName, "North", keys);
      printKeyRestrictionFormula(os, agentName, "East", keys);
      printKeyRestrictionFormula(os, agentName, "South", keys);
      printKeyRestrictionFormula(os, agentName, "West", keys);
    }
    
    printIsOnSlipperyFormula(os, agentName, slipperyCollection, slipperyNorth, slipperyEast, slipperySouth, slipperyWest);
    printIsInLavaFormula(os, agentName, lava);
    printWallFormula(os, agentName, walls);
    printTurningNotAllowedFormulas(os, agentName, noTurnFloor);
    printIsFixedFormulas(os, agentName);
    os << "\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printGoalLabel(std::ostream& os, const AgentName &agentName, const cells &goals) {
    if(goals.size() == 0) {
      os << "formula " << agentName << "IsInGoal = false;\n";
      return os;
    }

    bool first = true;
    os << "formula " << agentName << "IsInGoal = ";
    for(auto const& cell : goals) {
      if(first) first = false; else os << " | ";
      os << "(x" << agentName << "=" << cell.column << "&y" << agentName << "=" << cell.row << ")";
    }
    os << ";\n";
    os << "formula " << agentName << "IsInGoalAndNotDone = " << agentName << "IsInGoal & !" << agentName << "Done;\n";
    os << "label \"" << agentName << "IsInGoalAndNotDone\" = " << agentName << "IsInGoal & !" << agentName << "Done;\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printCrashLabel(std::ostream &os, const std::vector<AgentName> agentNames) {
    os << "label crash = ";
    bool first = true;
    for(auto const& agentName : agentNames) {
      if(agentName == "Agent") continue;
      if(first) first = false; else os << " | ";
      os << "(xAgent=x" << agentName << ")&(yAgent=y" << agentName << ")";
    }
    os << ";\n\n";
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

  std::ostream& PrismModulesPrinter::printAvoidanceLabel(std::ostream &os, const std::vector<AgentName> agentNames, const int &distance) {
    os << "label avoidance = ";
    bool first = true;
    for(auto const& agentName : agentNames) {
      if(agentName == "Agent") continue;
      if(first) first = false; else os << " | ";
      os << "max(xAgent-x" << agentName << ",x" << agentName << "-xAgent)+";
      os << "max(yAgent-y" << agentName << ",y" << agentName << "-yAgent) ";
    }
    os << ";\n\n";
    return os;
  }

  // TODO this does not account for multiple agents yet, i.e. key can be picked up multiple times
  std::ostream& PrismModulesPrinter::printKeysLabels(std::ostream& os, const AgentName &agentName, const cells &keys) {
    if(keys.size() == 0) return os;

    for(auto const& key : keys) {
      std::string keyColor = key.getColor();
      std::string xKey = "xKey" + keyColor;
      std::string yKey = "yKey" + keyColor;      
      os << "label \"" << agentName << "PickedUp" << keyColor << "Key\" = " << agentName << "_has_" << keyColor << "_key = true;\n";
      os << "formula " << agentName << "CanPickUp" << keyColor << "Key = ";
      os << "((x" << agentName << "-1 = " << xKey << "&y" << agentName << "   = " << yKey << "&view" << agentName << " = 2) |";
      os << " (x" << agentName << "+1 = " << xKey << "&y" << agentName << "   = " << yKey << "&view" << agentName << " = 0) |";
      os << " (x" << agentName << "   = " << xKey << "&y" << agentName << "-1 = " << yKey << "&view" << agentName << " = 3) |";
      os << " (x" << agentName << "   = " << xKey << "&y" << agentName << "+1 = " << yKey << "&view" << agentName << " = 1) ) &";
      os << "!" << agentName << "_has_" << keyColor << "_key;";
    }
    os << "\n";
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
      std::string keyColor = key.getColor();
      os << "\t[pickup_" << keyColor << "_key]\t" << pickupGuard(agentName, keyColor) << "-> ";
      os << "(" << agentName << "_has_" << keyColor << "_key'=true) & (" << agentName << "_is_carrying_object'=true) ;";
      os << "\n";
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

  std::ostream& PrismModulesPrinter::printInitStruct(std::ostream &os, const AgentName &agentName, const cells &keys) {
    os << "init\n";
    os << "\t(!AgentIsInGoal & !AgentIsInLava & !AgentDone & !AgentIsOnWall)";
    if(enforceOneWays) {
      os << " & ( !AgentCannotTurn ) ";
    } else {
      os << " & ( !AgentIsOnSlippery ) ";
    }

    for (auto const& key : keys) {
      os << " & ( !" << agentName << "_has_" << key.getColor() << "_key )";
      os << " & ( xKey" << key.getColor() << "="<< key.column << ")";
      os << " & ( yKey" << key.getColor() << "=" << key.row << ")";
    }

    os << " & ( !" << agentName << "_is_carrying_object" << ")";

    os << "\nendinit\n\n";

  
    return os;
  }

  std::ostream& PrismModulesPrinter::printModule(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &boundaries, const coordinates& initialPosition, const cells &keys, const std::map<Color, cells> &backgroundTiles, const bool agentWithView, const std::vector<float> &probabilities) {
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
    os << "\t[" << agentName << "_turn_right] " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery -> (view" << agentName << "'=mod(view" << agentName << " + 1, 4)) " << moveUpdate(agentIndex) << ";\n";
    os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << ">0 -> (view" << agentName << "'=view" << agentName << " - 1) " << moveUpdate(agentIndex) << ";\n";
    os << "\t[" << agentName << "_turn_left]  " << moveGuard(agentIndex) << " !" << agentName << "CannotTurn & " << " !" << agentName << "IsFixed & " << " !" << agentName << "IsInGoal & !" << agentName << "IsInLava & !" << agentName << "IsOnSlippery & view" << agentName << "=0 -> (view" << agentName << "'=3) " << moveUpdate(agentIndex) << ";\n";
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
    printMovementActions(os, agentName, agentIndex, agentWithView);
    for(auto const& probability : probabilities) {
      printMovementActions(os, agentName, agentIndex, agentWithView, probability);
    }
    printDoneActions(os, agentName, agentIndex);

    printConfiguredActions(os, agentName);

    os << "\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printKeyModule(std::ostream &os, const cell &key, const coordinates &boundaries) {
    std::string keyIdentifier = "Key" + key.getColor();

    os << "module " << keyIdentifier << "\n";

    os << "\tx" << keyIdentifier << " : [1.." << boundaries.second  << "];\n";
    os << "\ty" << keyIdentifier << " : [1.." << boundaries.first << "];\n";
    os << "\n";
    printKeyActions(os, key ,keyIdentifier);

    os << "\n";
    return os;
  }
  
  std::ostream& PrismModulesPrinter::printKeyActions(std::ostream &os, const cell &key ,const std::string &keyIdentifier) {
    std::string keyColor = key.getColor();
    std::string agentName = "Agent";
 
    os << "\t[drop_" << keyColor << "_key_north]\t" << dropGuard(agentName, keyColor, 3) << "-> ";
    os << "(xKey" << keyColor << "'=x" << agentName << ") & (yKey" << keyColor << "'=y" <<agentName << "-1) ;\n"; 

    os << "\t[drop_" << keyColor << "_key_west]\t" << dropGuard(agentName, keyColor, 2) << "-> ";
    os << "(xKey" << keyColor << "'=x" << agentName << "-1) & (yKey" << keyColor << "'=y" <<agentName << ") ;\n"; 

    os << "\t[drop_" << keyColor << "_key_south]\t" << dropGuard(agentName, keyColor, 1) << "-> ";
    os << "(xKey" << keyColor << "'=x" << agentName << ") & (yKey" << keyColor << "'=y" <<agentName << "+1) ;\n"; 

    os << "\t[drop_" << keyColor << "_key_east]\t" << dropGuard(agentName, keyColor, 0) << "-> ";
    os << "(xKey" << keyColor << "'=x" << agentName << "+1) & (yKey" << keyColor << "'=y" <<agentName << ") ;\n"; 

    return os;
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

  std::ostream& PrismModulesPrinter::printMovementActions(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const bool agentWithView, const float &probability) {
    if(probability >= 1) {
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
    std::size_t remainPosIndex = 8;
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, w, se, s, sw, w, nw, CURRENT POS }

    switch (orientation)
    {
      case SlipperyType::North:
        actionName = "\t[" + agentName + "turn_at_slip_north";
        prob_piece_dir = { 0, 0, 0, 1, 1, 1, 0, 0, 0 /* <- R */ };
        break;

      case SlipperyType::South:
        actionName = "\t[" + agentName + "turn_at_slip_south";
        prob_piece_dir = { 1, 1, 0, 0, 0, 0, 0, 1, 0 /* <- R */ };
        break;

      case SlipperyType::East:
        actionName = "\t[" + agentName + "turn_at_slip_east";
        prob_piece_dir = { 0, 0, 0, 0, 0, 1, 1, 1, 0 /* <- R */ };
        break;

      case SlipperyType::West:
        actionName = "\t[" + agentName + "turn_at_slip_west";
        prob_piece_dir = { 0, 1, 1, 1, 0, 0, 0, 0, 0 /* <- R */ };
        break;
    }

    slipperyActions.insert(actionName);

    // override probability to 0 if corresp. direction is blocked

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS - 1; i++) {
      if (!neighborhood.at(i))
        prob_piece_dir.at(i) = 0;
    }

    // determine residual probability (R) by replacing 0 with (1 - overall sum)

    prob_piece_dir.at(remainPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);

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
          os << (i == 0 ? " -> " : " + ") << prob_piece_dir.at(i) << "/" << PROB_PIECES << " : " << positionTransition.at(i) << std::get<1>(viewTransition.at(v)) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
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

    std::size_t straightPosIndex;
    std::string actionName, specialTransition; // if straight ahead is blocked
    std::array<std::size_t, ALL_POSS_DIRECTIONS> prob_piece_dir; // { n, ne, w, se, s, sw, w, nw }

    switch (orientation)
    {
      case SlipperyType::North:
        actionName = "\t[" + agentName + "move_on_slip_north]";
        prob_piece_dir = { 0, 0, 1, 2, 0 /* <- R */, 2, 1, 0 };
        straightPosIndex = 4;
        specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "+1)");
        break;

      case SlipperyType::South:
        actionName = "\t[" + agentName + "move_on_slip_south]";
        prob_piece_dir = { 0 /* <- R */, 2, 1, 0, 0, 0, 1, 2 };
        straightPosIndex = 0; // always north
        specialTransition = "(y" + agentName + "'=y" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
        break;

      case SlipperyType::East:
        actionName = "\t[" + agentName + "move_on_slip_east]";
        prob_piece_dir = { 1, 0, 0, 0, 1, 2, 0 /* <- R */, 2 };
        straightPosIndex = 6;
        specialTransition = "(x" + agentName + "'=x" + agentName + (!neighborhood.at(straightPosIndex) ? ")" : "-1)");
        break;

      case SlipperyType::West:
        actionName = "\t[" + agentName + "move_on_slip_west]";
        prob_piece_dir = { 1, 2, 0 /* <- R */, 2, 1, 0, 0, 0 };
        straightPosIndex = 2;
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
    }
    prob_piece_dir.at(straightPosIndex) = PROB_PIECES - std::accumulate(prob_piece_dir.begin(), prob_piece_dir.end(), 0);

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

    os << actionName << moveGuard(agentIndex) << " x" << agentName << "=" << c.second << " & y" << agentName << "=" << c.first << " & " << agentName << "SlipperyMoveForwardAllowed ";

    for (std::size_t i = 0; i < ALL_POSS_DIRECTIONS; i++) {
      os << (i == 0 ? " -> " : " + ") << prob_piece_dir.at(i) << "/" << PROB_PIECES << " : " << positionTransition.at(i) << moveUpdate(agentIndex) << (i == ALL_POSS_DIRECTIONS - 1 ? ";\n" : "\n");
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
    os << "\nendplayer\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printGlobalMoveVariable(std::ostream &os, const size_t &numberOfPlayer) {
    os << "\nglobal move : [0.." << std::to_string(numberOfPlayer - 1) << "] init 0;\n\n";
    return os;
  }

  std::ostream& PrismModulesPrinter::printRewards(std::ostream &os, const AgentName &agentName, const std::map<coordinates, float> &stateRewards, const cells &lava, const cells &goals, const std::map<Color, cells> &backgroundTiles) {
    if(lava.size() != 0) {
      os << "rewards \"SafetyNoBFS\"\n";
      os << "\tAgentIsInLavaAndNotDone: -100;\n";
      os << "endrewards\n";
    }

    if (!goals.empty() || !lava.empty())  {
      os << "rewards \"SafetyNoBFSAndGoal\"\n";
      if(goals.size() != 0) os << "\tAgentIsInGoalAndNotDone:  100;\n";
      if(lava.size() != 0) os << "\tAgentIsInLavaAndNotDone: -100;\n";
      os << "endrewards\n";
    }

    os << "rewards \"Time\"\n";
    os << "\t!AgentIsInGoal : -1;\n";
    if(goals.size() != 0) os << "\tAgentIsInGoalAndNotDone:  100;\n";
    if(lava.size() != 0) os << "\tAgentIsInLavaAndNotDone: -100;\n";
    os << "endrewards\n";

    if(stateRewards.size() > 0) {
      os << "rewards \"SafetyWithBFS\"\n";
      if(lava.size() != 0) os << "\tAgentIsInLavaAndNotDone: -100;\n";
      for(auto const [coordinates, reward] : stateRewards) {
        os << "\txAgent=" << coordinates.first << "&yAgent=" << coordinates.second << " : " << reward << ";\n";
      }
      os << "endrewards\n";
    }
    if(stateRewards.size() > 0) {
      os << "rewards \"SafetyWithBFSAndGoal\"\n";
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
