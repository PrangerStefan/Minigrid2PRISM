#pragma once

#include <iostream>
#include <fstream>
#include <map>
#include <utility>

#include "MinigridGrammar.h"
#include "PrismModulesPrinter.h"
#include "PrismFormulaPrinter.h"
#include "ConfigYaml.h"

struct GridOptions {
  std::vector<AgentName> agentsToBeConsidered;
  std::vector<AgentName> agentsWithView;
  std::vector<AgentName> agentsWithProbabilisticBehaviour;
  std::vector<float>     probabilitiesForActions;
  bool                   enforceOneWays;

  prism::ModelType getModelType() const;
};

class Grid {
  public:
    Grid(cells gridCells, cells background, const GridOptions &gridOptions, const std::map<coordinates, float> &stateRewards = {}, const float faultyProbability = 0);

    cells getGridCells();

    bool isBlocked(coordinates p);
    bool isWall(coordinates p);
    void printToPrism(std::ostream &os, std::vector<Configuration>& configuration, const prism::ModelType& modelType);
    void applyOverwrites(std::string& str, std::vector<Configuration>& configuration);

    std::array<bool, 8> getWalkableDirOf8Neighborhood(cell c);

    friend std::ostream& operator<<(std::ostream& os, const Grid &grid);

  private:
    GridOptions gridOptions;

    cells allGridCells;
    cells background;
    coordinates maxBoundaries;

    cell agent;
    cells adversaries;
    AgentNameAndPositionMap agentNameAndPositionMap;
    KeyNameAndPositionMap keyNameAndPositionMap;

    cells walls;
    cells floor;
    cells slipperyNorth;
    cells slipperyEast;
    cells slipperySouth;
    cells slipperyWest;
    cells lockedDoors;
    cells unlockedDoors;
    cells boxes;
    cells balls;
    cells lava;

    cells goals;
    cells keys;

    std::map<Color, cells> backgroundTiles;

    std::map<coordinates, float> stateRewards;
    float faultyProbability;
};
