#include "Grid.h"
#include <boost/algorithm/string/find.hpp>

#include <algorithm>

prism::ModelType GridOptions::getModelType() const
{
  if (agentsWithView.size() > 1) {
    return prism::ModelType::SMG;
  }
  return prism::ModelType::MDP;
}

Grid::Grid(cells gridCells, cells background, const GridOptions &gridOptions, const std::map<coordinates, float> &stateRewards, const float faultyProbability)
  : allGridCells(gridCells), background(background), gridOptions(gridOptions), stateRewards(stateRewards), faultyProbability(faultyProbability)
{
  cell max = allGridCells.at(allGridCells.size() - 1);
  maxBoundaries = std::make_pair(max.row - 1, max.column - 1);
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(walls),         [](cell c) { return c.type == Type::Wall; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(lava),          [](cell c) { return c.type == Type::Lava; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(floor),         [](cell c) { return c.type == Type::Floor; }); // TODO CHECK IF ALL AGENTS ARE ADDED TO FLOOR
  std::copy_if(background.begin(), background.end(), std::back_inserter(slipperyNorth), [](cell c) { return c.type == Type::SlipperyNorth; });
  std::copy_if(background.begin(), background.end(), std::back_inserter(slipperyEast),  [](cell c) { return c.type == Type::SlipperyEast; });
  std::copy_if(background.begin(), background.end(), std::back_inserter(slipperySouth), [](cell c) { return c.type == Type::SlipperySouth; });
  std::copy_if(background.begin(), background.end(), std::back_inserter(slipperyWest),  [](cell c) { return c.type == Type::SlipperyWest; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(lockedDoors),   [](cell c) { return c.type == Type::LockedDoor; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(unlockedDoors), [](cell c) { return c.type == Type::Door; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(goals),         [](cell c) { return c.type == Type::Goal; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(keys),          [](cell c) { return c.type == Type::Key; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(boxes),         [](cell c) { return c.type == Type::Box; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(balls),         [](cell c) { return c.type == Type::Ball; });
  std::copy_if(gridCells.begin(),  gridCells.end(),  std::back_inserter(adversaries),   [](cell c) { return c.type == Type::Adversary; });
  agent = *std::find_if(gridCells.begin(), gridCells.end(), [](cell c) { return c.type == Type::Agent; });
  floor.push_back(agent);

  agentNameAndPositionMap.insert({ "Agent", agent.getCoordinates() });
  for(auto const& adversary : adversaries) {
    std::string color = adversary.getColor();
    color.at(0) = std::toupper(color.at(0));
    try {
      if(gridOptions.agentsToBeConsidered.size() != 0 && std::find(gridOptions.agentsToBeConsidered.begin(), gridOptions.agentsToBeConsidered.end(), color) == gridOptions.agentsToBeConsidered.end()) continue;
      auto success = agentNameAndPositionMap.insert({ color, adversary.getCoordinates() });
      floor.push_back(adversary);
      if(!success.second) {
        throw std::logic_error("Agent with " + color + " already present\n");
      }
    } catch(const std::logic_error& e) {
      std::cerr << "Expected agents colors to be different. Agent with color : '" << color << "' already present." << std::endl;
      throw;
    }
  }
  for(auto const& key : keys) {
    std::string color = key.getColor();
    try {
      auto success = keyNameAndPositionMap.insert({color, key.getCoordinates() });
      if (!success.second) {
        throw std::logic_error("Multiple keys with same color not supported " + color + "\n");
      }
    } catch(const std::logic_error& e) {
      std::cerr << "Expected key colors to be different. Key with color : '" << color << "' already present." << std::endl;
      throw;
    }
  }
  for(auto const& color : allColors) {
    cells cellsOfColor;
    std::copy_if(background.begin(), background.end(), std::back_inserter(cellsOfColor), [&color](cell c) {
        return c.type == Type::Floor && c.color == color;
    });
    if(cellsOfColor.size() > 0) {
      backgroundTiles.emplace(color, cellsOfColor);
    }
  }
}

std::ostream& operator<<(std::ostream& os, const Grid& grid) {
  int lastRow = 1;
  for(auto const& cell : grid.allGridCells) {
    if(lastRow != cell.row)
      os << std::endl;
    os << static_cast<char>(cell.type) << static_cast<char>(cell.color);
    lastRow = cell.row;
  }
  return os;
}

cells Grid::getGridCells() {
  return allGridCells;
}

bool Grid::isBlocked(coordinates p) {
  return isWall(p);
}

bool Grid::isWall(coordinates p) {
  return std::find_if(walls.begin(), walls.end(),
      [p](cell cell) {
        return cell.row == p.second && cell.column == p.first;
      }) != walls.end();
}


void Grid::applyOverwrites(std::string& str, std::vector<Configuration>& configuration) {
  for (auto& config : configuration) {
    if (!config.overwrite_) {
      continue;
    }
      size_t start_pos;

      if (config.type_ == ConfigType::Formula) {
        start_pos = str.find("formula " + config.identifier_);
      } else if (config.type_ == ConfigType::Label) {
        start_pos = str.find("label " + config.identifier_);
      } else if (config.type_ == ConfigType::Module) {
        auto iter = boost::find_nth(str, config.identifier_, config.index_);
        start_pos = std::distance(str.begin(), iter.begin());
      }
       else if (config.type_ == ConfigType::Constant) {
        start_pos = str.find(config.identifier_);

        if (start_pos == std::string::npos) {
          std::cout << "Couldn't find overwrite:" << config.expression_ << std::endl;
        }
      }

      size_t end_pos = str.find(';', start_pos) + 1;

      std::string expression = config.expression_;

      str.replace(start_pos, end_pos - start_pos , expression);
  }
}
void Grid::printToPrism(std::ostream& os, std::vector<Configuration>& configuration ,const prism::ModelType& modelType) {
  cells northRestriction, eastRestriction, southRestriction, westRestriction;

  cells walkable = floor;
  walkable.insert(walkable.end(), goals.begin(), goals.end());
  walkable.insert(walkable.end(), boxes.begin(), boxes.end());
  walkable.insert(walkable.end(), adversaries.begin(), adversaries.end());
  walkable.insert(walkable.end(), lava.begin(), lava.end());
  walkable.insert(walkable.end(), keys.begin(), keys.end());
  walkable.insert(walkable.end(), balls.begin(), balls.end());

  for(auto const& c : walkable) {
    std::cout << "checking: " << c << " south of c: " << c.getSouth().first << " " << c.getSouth().second << std::endl;
    if(isWall(c.getNorth())) northRestriction.push_back(c);
    if(isWall(c.getEast()))   eastRestriction.push_back(c);
    if(isWall(c.getSouth())) southRestriction.push_back(c);
    if(isWall(c.getWest()))   westRestriction.push_back(c);
  }


  std::map<std::string, cells> wallRestrictions = {{"North", northRestriction}, {"East", eastRestriction}, {"South", southRestriction}, {"West", westRestriction}};
  std::map<std::string, cells> slipperyTiles    = {{"North", slipperyNorth}, {"East", slipperyEast}, {"South", slipperySouth}, {"West", slipperyWest}};

  std::vector<AgentName> agentNames;
  std::transform(agentNameAndPositionMap.begin(),
                 agentNameAndPositionMap.end(),
                 std::back_inserter(agentNames),
                 [](const std::map<AgentNameAndPosition::first_type,AgentNameAndPosition::second_type>::value_type &pair){return pair.first;});
  std::string agentName = agentNames.at(0);

  prism::PrismFormulaPrinter formulas(os, wallRestrictions, boxes, balls, lockedDoors, unlockedDoors, keys, slipperyTiles, lava, goals);
  prism::PrismModulesPrinter modules(os, modelType, maxBoundaries, boxes, balls, lockedDoors, unlockedDoors, keys, slipperyTiles, agentNameAndPositionMap, configuration, faultyProbability);

  modules.printModelType(modelType);
  for(const auto &agentName : agentNames) {
    formulas.print(agentName);
  }
  if(modelType == prism::ModelType::SMG) modules.printGlobalMoveVariable(os, agentNameAndPositionMap.size());
  std::vector<std::string> constants {"const double prop_zero = 0/9;",
                                      "const double prop_intended = 6/9;",
                                      "const double prop_turn_intended = 6/9;",
                                      "const double prop_displacement = 3/9;",
                                      "const double prop_turn_displacement = 3/9;",
                                      "const int width = " + std::to_string(maxBoundaries.first) + ";",
                                      "const int height = " + std::to_string(maxBoundaries.second) + ";"
                                      };
  modules.printConstants(os, constants);
  modules.print();




  //modules.printInitStruct(os, agentNameAndPositionMap, keyNameAndPositionMap, lockedDoors, unlockedDoors, modelType);


  //size_t agentIndex  = 0;
  //for(auto agentNameAndPosition = agentNameAndPositionMap.begin(); agentNameAndPosition != agentNameAndPositionMap.end(); ++agentNameAndPosition, agentIndex++) {
  //  AgentName agentName = agentNameAndPosition->first;
  //  //std::cout << "Agent Name: " << agentName << std::endl;
  //  bool agentWithView = std::find(gridOptions.agentsWithView.begin(), gridOptions.agentsWithView.end(), agentName) != gridOptions.agentsWithView.end();
  //  bool agentWithProbabilisticBehaviour = std::find(gridOptions.agentsWithProbabilisticBehaviour.begin(), gridOptions.agentsWithProbabilisticBehaviour.end(), agentName) != gridOptions.agentsWithProbabilisticBehaviour.end();
  //  std::set<std::string> slipperyActions; // TODO AGENT POSITION INITIALIZATIN
  //  if(agentWithProbabilisticBehaviour) modules.printModule(os, agentName, agentIndex, maxBoundaries, agentNameAndPosition->second, keys, backgroundTiles, agentWithView, gridOptions.probabilitiesForActions);
  //  else                                modules.printModule(os, agentName, agentIndex, maxBoundaries, agentNameAndPosition->second, keys, backgroundTiles, agentWithView, {} ,faultyProbability);
  //  for(auto const& c : slipperyNorth) {
  //    modules.printSlipperyMove(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::North);
  //    if(!gridOptions.enforceOneWays) modules.printSlipperyTurn(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::North);

  //  }
  //  for(auto const& c : slipperyEast) {
  //    modules.printSlipperyMove(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::East);
  //    if(!gridOptions.enforceOneWays) modules.printSlipperyTurn(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::East);
  //  }
  //  for(auto const& c : slipperySouth) {
  //    modules.printSlipperyMove(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::South);
  //    if(!gridOptions.enforceOneWays) modules.printSlipperyTurn(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::South);
  //  }
  //  for(auto const& c : slipperyWest) {
  //    modules.printSlipperyMove(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::West);
  //    if(!gridOptions.enforceOneWays) modules.printSlipperyTurn(os, agentName, agentIndex, c.getCoordinates(), slipperyActions, getWalkableDirOf8Neighborhood(c), prism::PrismModulesPrinter::SlipperyType::West);
  //  }

  //  modules.printEndmodule(os);

  //  if(modelType == prism::ModelType::SMG) {
  //    if(agentWithProbabilisticBehaviour) modules.printPlayerStruct(os, agentNameAndPosition->first, agentWithView, gridOptions.probabilitiesForActions, slipperyActions);
  //    else                                modules.printPlayerStruct(os, agentNameAndPosition->first, agentWithView, {}, slipperyActions);
  //  }
  //if(!stateRewards.empty()) {
  //  modules.printRewards(os, agentName, stateRewards, lava, goals, backgroundTiles);
  //}

  //}

  if (!configuration.empty()) {
    modules.printConfiguration(os, configuration);
  }
}


std::array<bool, 8> Grid::getWalkableDirOf8Neighborhood(cell c) /* const */ {
  return (std::array<bool, 8>)
         {
           !isBlocked(c.getNorth()),
           !isBlocked(c.getNorth(allGridCells).getEast()),
           !isBlocked(c.getEast()),
           !isBlocked(c.getSouth(allGridCells).getEast()),
           !isBlocked(c.getSouth()),
           !isBlocked(c.getSouth(allGridCells).getWest()),
           !isBlocked(c.getWest()),
           !isBlocked(c.getNorth(allGridCells).getWest())
         };
}
