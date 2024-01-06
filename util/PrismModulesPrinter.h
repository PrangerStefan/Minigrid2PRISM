#pragma once

#include <iostream>
#include <functional>
#include "MinigridGrammar.h"
#include "PrismPrinter.h"
#include "ConfigYaml.h"

namespace prism {
  class PrismModulesPrinter {
    public:
      PrismModulesPrinter(std::ostream &os, const ModelType &modelType, const coordinates &maxBoundaries, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const AgentNameAndPositionMap &agentNameAndPositionMap, std::vector<Configuration> config, const bool enforceOneWays = false);

      std::ostream& print();

      std::ostream& printModelType(const ModelType &modelType);

      void printPortableObjectModule(const cell &object);
      void printPortableObjectActions(const std::string &agentName, const std::string &identifier);

      void printDoorModule(const cell &object, const bool &opened);
      void printLockedDoorActions(const std::string &agentName, const std::string &identifier);
      void printUnlockedDoorActions(const std::string &agentName, const std::string &identifier);

      void printRobotModule(const AgentName &agentName, const coordinates &initialPosition);
      void printPortableObjectActionsForRobot(const std::string &agentName, const std::string &identifier);

      void printUnlockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier);
      void printLockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier, const std::string &key);

      std::ostream& printConstants(std::ostream &os, const std::vector<std::string> &constants);
       /*
        * Representation for Slippery Tile.
        *  -) North: Slips from North to South
        *  -) East: Slips from East to West
        *  -) South: Slips from South to North
        *  -) West: Slips from West to East
        */
      enum class SlipperyType { North, East, South, West };

      /*
       * Prints Slippery on move action.
       *
       * @param neighborhood: Information of wall-blocks in 8-neighborhood { n, nw, e, se, s, sw, w, nw }. If entry is false, then corresponding neighboorhood position is a wall.
       * @param orientation: Information of slippery type (either north, south, east, west).
       */
      std::ostream& printSlipperyMove(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation);

      /*
       * Prints Slippery on turn action.
       *
       * @param neighborhood: Information of wall-blocks in 8-neighborhood { n, nw, e, se, s, sw, w, nw }. If entry is false, then corresponding neighboorhood position is a wall.
       * @param orientation: Information of slippery type (either north, south, east, west).
       */
      std::ostream& printSlipperyTurn(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const coordinates &c, std::set<std::string> &slipperyActions, const std::array<bool, 8>& neighborhood, SlipperyType orientation);

      std::ostream& printBooleansForKeys(std::ostream &os, const AgentName &agentName, const cells &keys);
      std::ostream& printActionsForKeys(std::ostream &os, const AgentName &agentName, const cells &keys);
      std::ostream& printBooleansForBackground(std::ostream &os, const AgentName &agentName, const std::map<Color, cells> &backgroundTiles);
      std::ostream& printActionsForBackground(std::ostream &os, const AgentName &agentName, const std::map<Color, cells> &backgroundTiles);
      std::ostream& printInitStruct(std::ostream &os, const AgentNameAndPositionMap &agents, const KeyNameAndPositionMap &keys, const cells &lockedDoors, const cells &unlockedDoors, prism::ModelType modelType);
      std::ostream& printModule(std::ostream &os,
                                const AgentName &agentName,
                                const size_t &agentIndex,
                                const coordinates &boundaries,
                                const coordinates& initialPosition,
                                const cells &keys,
                                const std::map<Color, cells> &backgroundTiles,
                                const bool agentWithView,
                                const std::vector<float> &probabilities = {},
                                const double faultyProbability = 0);
      std::ostream& printMovementActions(std::ostream &os, const AgentName &agentName, const size_t &agentIndex, const bool agentWithView, const float &probability = 1.0, const double &stickyProbability = 0.0);
      std::ostream& printDoneActions(std::ostream &os, const AgentName &agentName, const size_t &agentIndex);
      std::ostream& printEndmodule(std::ostream &os);
      std::ostream& printPlayerStruct(std::ostream &os, const AgentName &agentName, const bool agentWithView, const std::vector<float> &probabilities = {}, const std::set<std::string> &slipperyActions = {});
      std::ostream& printGlobalMoveVariable(std::ostream &os, const size_t &numberOfPlayer);
      std::ostream& printRewards(std::ostream &os, const AgentName &agentName, const std::map<coordinates, float> &stateRewards, const cells &lava, const cells &goals, const std::map<Color, cells> &backgroundTiles);

      std::ostream& printConfiguration(std::ostream &os, const std::vector<Configuration>& configurations);
      std::ostream& printConfiguredActions(std::ostream &os, const AgentName &agentName);

      std::string moveGuard(const size_t &agentIndex);
      std::string pickupGuard(const AgentName &agentName, const std::string keyColor);
      std::string dropGuard(const AgentName &agentName, const std::string keyColor, size_t view);
      std::string moveUpdate(const size_t &agentIndex);

      std::string viewVariable(const AgentName &agentName, const size_t &agentDirection, const bool agentWithView);

      bool isGame() const;
    private:
      std::ostream &os;
      std::stringstream actionStream;

      ModelType const& modelType;
      coordinates const& maxBoundaries;
      AgentName agentName;
      cells boxes;
      cells balls;
      cells lockedDoors;
      cells unlockedDoors;
      cells keys;

      AgentNameAndPositionMap agentNameAndPositionMap;
      size_t numberOfPlayer;
      bool enforceOneWays;
      std::vector<Configuration> configuration;
      std::map<int, std::string> viewDirectionMapping;
  };
}
