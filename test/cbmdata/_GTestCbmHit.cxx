#include "CbmHit.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(_GTestCbmHit , CheckDefaultConstructor)
{
  CbmHit test;
  // should initialize the Object with the following values
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(0.,test.GetZ());
  EXPECT_FLOAT_EQ(0.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());
}

TEST(_GTestCbmHit , CheckStandardConstructor)
{
  CbmHit test{kHIT, 0., 0., -1, -1};
  // should initialize the Object with the following values
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(0.,test.GetZ());
  EXPECT_FLOAT_EQ(0.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());
}

TEST(_GTestCbmHit , CheckStandardConstructorWithTime)
{
  CbmHit test{kHIT, 0., 0., -1, -1, -2.,-2.};
  // should initialize the Object with the following values
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(0.,test.GetZ());
  EXPECT_FLOAT_EQ(0.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-2, test.GetTime());
  EXPECT_FLOAT_EQ(-2, test.GetTimeError());
}

TEST(_GTestCbmHit , TestSettersAndGetters)
{
  CbmHit test;
  // should initialize the Object with the following values
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(0.,test.GetZ());
  EXPECT_FLOAT_EQ(0.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());

  test.SetZ(-2.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(0.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());

  test.SetDz(-2.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-1, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());

  test.SetRefId(-2);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-2, test.GetRefId());
  EXPECT_EQ(-1, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());

  test.SetAddress(-2.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-2, test.GetRefId());
  EXPECT_EQ(-2, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-1, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());

  test.SetTime(-2.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-2, test.GetRefId());
  EXPECT_EQ(-2, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-2, test.GetTime());
  EXPECT_FLOAT_EQ(-1, test.GetTimeError());


  test.SetTimeError(-2.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-2, test.GetRefId());
  EXPECT_EQ(-2, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-2, test.GetTime());
  EXPECT_FLOAT_EQ(-2, test.GetTimeError());

  test.SetTime(-3., -3.);
  EXPECT_EQ(kHIT, test.GetType());
  EXPECT_FLOAT_EQ(-2.,test.GetZ());
  EXPECT_FLOAT_EQ(-2.,test.GetDz());
  EXPECT_EQ(-2, test.GetRefId());
  EXPECT_EQ(-2, test.GetAddress());
  EXPECT_EQ(nullptr, test.GetMatch());
  EXPECT_FLOAT_EQ(-3, test.GetTime());
  EXPECT_FLOAT_EQ(-3, test.GetTimeError());

  EXPECT_EQ(-1, test.GetPlaneId());

  //TODO: Test setter for fMatch
}

TEST(_GTestCbmHit , ToString)
{
  CbmHit test{kHIT, 0., 0., -1, -1};
  EXPECT_STREQ("Has to be implemented in derrived class", test.ToString().c_str());
}
