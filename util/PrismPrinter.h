#pragma once

#include <string>

#include "cell.h"

typedef std::string AgentName;
typedef size_t ViewDirection;
typedef std::pair<std::string, coordinates> AgentNameAndPosition;
typedef AgentNameAndPosition KeyNameAndPosition;
typedef std::map<AgentNameAndPosition::first_type, AgentNameAndPosition::second_type> AgentNameAndPositionMap;
typedef std::map<KeyNameAndPosition::first_type, KeyNameAndPosition::second_type> KeyNameAndPositionMap;
typedef std::pair<cell, std::string> CellAndCondition;

namespace prism {
  enum class ModelType {
    MDP, SMG
  };
}
