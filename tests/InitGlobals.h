#pragma once

#include <gtest/gtest.h>

#include "Init.h"

class GlobalInit : public ::testing::Test {
public:
  static void SetUpTestSuite() {
    SandalBot::initGlobals();
  }
};