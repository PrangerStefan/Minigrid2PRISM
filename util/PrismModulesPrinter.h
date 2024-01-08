#pragma once

#include <iostream>
#include <functional>
#include "MinigridGrammar.h"
#include "PrismPrinter.h"
#include "ConfigYaml.h"


std::string northUpdate(const AgentName &a);
std::string southUpdate(const AgentName &a);
std::string eastUpdate(const AgentName &a);
std::string westUpdate(const AgentName &a);

namespace prism {
  class PrismModulesPrinter {
    public:
      PrismModulesPrinter(std::ostream& os, const ModelType &modelType, const coordinates &maxBoundaries, const cells &boxes, const cells &balls, const cells &lockedDoors, const cells &unlockedDoors, const cells &keys, const std::map<std::string, cells> &slipperyTiles, const AgentNameAndPositionMap &agentNameAndPositionMap, std::vector<Configuration> config, const float probIntended, const float faultyProbability);

      std::ostream& print();

      std::ostream& printModelType(const ModelType &modelType);


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

      std::ostream& printInitStruct(std::ostream &os, const AgentNameAndPositionMap &agents, const KeyNameAndPositionMap &keys, const cells &lockedDoors, const cells &unlockedDoors, prism::ModelType modelType);
      std::ostream& printDoneActions(std::ostream &os, const AgentName &agentName);
      std::ostream& printPlayerStruct(std::ostream &os, const AgentName &agentName, const bool agentWithView, const std::vector<float> &probabilities = {}, const std::set<std::string> &slipperyActions = {});
      std::ostream& printGlobalMoveVariable(std::ostream &os, const size_t &numberOfPlayer);
      std::ostream& printRewards(std::ostream &os, const AgentName &agentName, const std::map<coordinates, float> &stateRewards, const cells &lava, const cells &goals, const std::map<Color, cells> &backgroundTiles);

      std::ostream& printConfiguration(std::ostream &os, const std::vector<Configuration>& configurations);
      std::ostream& printConfiguredActions(std::ostream &os, const AgentName &agentName);


      bool isGame() const;
    private:
      void printPortableObjectModule(const cell &object);
      void printPortableObjectActions(const std::string &agentName, const std::string &identifier);

      void printDoorModule(const cell &object, const bool &opened);
      void printLockedDoorActions(const std::string &agentName, const std::string &identifier);
      void printUnlockedDoorActions(const std::string &agentName, const std::string &identifier);

      void printRobotModule(const AgentName &agentName, const coordinates &initialPosition);
      void printPortableObjectActionsForRobot(const std::string &agentName, const std::string &identifier);
      void printUnlockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier);
      void printLockedDoorActionsForRobot(const std::string &agentName, const std::string &identifier, const std::string &key);
      void printMovementActionsForRobot(const std::string &a);
      void printTurnActionsForRobot(const std::string &a);
      void printSlipperyMovementActionsForRobot(const AgentName &a);
      void printSlipperyMovementActionsForNorth(const AgentName &a);
      void printSlipperyMovementActionsForEast(const AgentName &a);
      void printSlipperyMovementActionsForSouth(const AgentName &a);
      void printSlipperyMovementActionsForWest(const AgentName &a);

      std::string printMovementGuard(const AgentName &a, const std::string &direction, const size_t &viewDirection) const;
      std::string printMovementUpdate(const AgentName &a, const update &update) const;
      std::string printTurnGuard(const AgentName &a, const std::string &direction, const ActionId &actionId, const std::string &cond = "") const;
      std::string printTurnUpdate(const AgentName &a, const update &u, const ActionId &actionId) const;
      std::string printSlipperyMovementGuard(const AgentName &a, const std::string &direction, const ViewDirection &viewDirection, const std::vector<std::string> &guards) const;
      std::string printSlipperyMovementUpdate(const AgentName &a, const std::string &direction, const updates &u) const;

      bool anyPortableObject() const;
      bool faultyBehaviour() const;
      bool slipperyBehaviour() const;
      std::string moveGuard(const AgentName &agentName) const;
      std::string faultyBehaviourGuard(const AgentName &agentName, const ActionId &actionId) const;
      std::string faultyBehaviourUpdate(const AgentName &agentName, const ActionId &actionId) const;
      std::string moveUpdate(const AgentName &agentName) const;
      std::string updatesToString(const updates &updates, const AgentName &a, const ActionId &actionId) const;
      std::string updateToString(const update &u) const;

      std::string viewVariable(const AgentName &agentName, const size_t &agentDirection, const bool agentWithView = true) const;


      std::string buildConjunction(const AgentName &a, std::vector<std::string> formulae) const;


      std::ostream &os;
      std::stringstream actionStream;

      ModelType const &modelType;
      coordinates const &maxBoundaries;
      AgentName agentName;
      cells boxes;
      cells balls;
      cells lockedDoors;
      cells unlockedDoors;
      cells keys;
      std::map<std::string, cells> slipperyTiles;

      AgentNameAndPositionMap agentNameAndPositionMap;
      std::map<AgentName, size_t> agentIndexMap;
      size_t numberOfPlayer;
      float const faultyProbability;
      float const probIntended;
      std::vector<Configuration> configuration;
      std::map<int, std::string> viewDirectionMapping;
      std::vector<ViewDirection> viewDirections = {0, 1, 2, 3};
  };
}
