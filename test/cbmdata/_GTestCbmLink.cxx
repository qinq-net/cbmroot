#include "CbmLink.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(_GTestCbmLink , CheckDefaultConstructor)
{
  CbmLink test;
  EXPECT_EQ(-1, test.GetFile());
  EXPECT_EQ(-1,test.GetEntry());
  EXPECT_EQ(-1,test.GetIndex());
  EXPECT_FLOAT_EQ(-1, test.GetWeight());
}

TEST(_GTestCbmLink , CheckStandardConstructor1)
{
  CbmLink test{-2., -2};
  EXPECT_EQ(-1, test.GetFile());
  EXPECT_EQ(-1,test.GetEntry());
  EXPECT_EQ(-2,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());
}

TEST(_GTestCbmLink , CheckStandardConstructor2)
{
  CbmLink test{-2., -2, -2, -2};
  EXPECT_EQ(-2, test.GetFile());
  EXPECT_EQ(-2,test.GetEntry());
  EXPECT_EQ(-2,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());
}

TEST(_GTestCbmLink , TestSettersAndGetters)
{
  CbmLink test{-2., -2, -2, -2};
  EXPECT_EQ(-2, test.GetFile());
  EXPECT_EQ(-2,test.GetEntry());
  EXPECT_EQ(-2,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());

  test.SetFile(-3);
  EXPECT_EQ(-3, test.GetFile());
  EXPECT_EQ(-2,test.GetEntry());
  EXPECT_EQ(-2,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());

  test.SetEntry(-3);
  EXPECT_EQ(-3, test.GetFile());
  EXPECT_EQ(-3,test.GetEntry());
  EXPECT_EQ(-2,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());

  test.SetIndex(-3);
  EXPECT_EQ(-3, test.GetFile());
  EXPECT_EQ(-3,test.GetEntry());
  EXPECT_EQ(-3,test.GetIndex());
  EXPECT_FLOAT_EQ(-2, test.GetWeight());

  test.SetWeight(-3.);
  EXPECT_EQ(-3, test.GetFile());
  EXPECT_EQ(-3,test.GetEntry());
  EXPECT_EQ(-3,test.GetIndex());
  EXPECT_FLOAT_EQ(-3, test.GetWeight());

  test.AddWeight(-3);
  EXPECT_EQ(-3, test.GetFile());
  EXPECT_EQ(-3,test.GetEntry());
  EXPECT_EQ(-3,test.GetIndex());
  EXPECT_FLOAT_EQ(-6, test.GetWeight());
}

TEST(_GTestCbmLink , ToString)
{
  CbmLink test{-2., -2, -2, -2};
  EXPECT_STREQ("CbmLink: weight=-2 index=-2 entry=-2 file=-2\n", test.ToString().c_str());
}


TEST(_GTestCbmLink , CompareEqual)
{
  CbmLink test{-2., -2, -2, -2};
  CbmLink test1{-2., -2, -2, -2};
  CbmLink test2{-1., -2, -2, -2};
  CbmLink test3{-2., -1, -2, -2};
  CbmLink test4{-2., -2, -1, -2};
  CbmLink test5{-2., -2, -2, -1};

  EXPECT_TRUE(test==test1);
  EXPECT_TRUE(test==test2); //equals only check file, entry, and index ids but not the weight
  EXPECT_FALSE(test==test3);
  EXPECT_FALSE(test==test4);
  EXPECT_FALSE(test==test5);
}

TEST(_GTestCbmLink , CompareGreater)
{
  CbmLink test{2., 2, 2, 2};
  CbmLink test1{2., 2, 2, 2};
  CbmLink test2{2., 1, 2, 2};
  CbmLink test3{2., 2, 1, 2};
  CbmLink test4{2., 2, 2, 1};

  EXPECT_FALSE(test>test1);
  EXPECT_TRUE(test>test2);
  EXPECT_FALSE(test2>test);
  EXPECT_TRUE(test>test3);
  EXPECT_FALSE(test3>test);
  EXPECT_TRUE(test>test4);
  EXPECT_FALSE(test4>test);
}

TEST(_GTestCbmLink , CompareSmaller)
{
  CbmLink test{2., 2, 2, 2};
  CbmLink test1{2., 2, 2, 2};
  CbmLink test2{2., 1, 2, 2};
  CbmLink test3{2., 2, 1, 2};
  CbmLink test4{2., 2, 2, 1};

  EXPECT_FALSE(test<test1);
  EXPECT_FALSE(test<test2);
  EXPECT_TRUE(test2<test);
  EXPECT_FALSE(test<test3);
  EXPECT_TRUE(test3<test);
  EXPECT_FALSE(test<test4);
  EXPECT_TRUE(test4<test);
}
