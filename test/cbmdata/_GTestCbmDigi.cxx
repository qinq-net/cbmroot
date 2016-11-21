#include "CbmDigi.h"

#include "CbmMatch.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

#include <utility> // std::move
#include <iostream>

// Since CbmDigi is an abstract base class which can't be instantiated directly we have
// to create a derived class without any data members which simply forwards the function
// calls to the abstract base class

class CbmTestDigi : public CbmDigi
{

  public:
    CbmTestDigi() : CbmDigi() {;}

    /** Copy constructor  **/
    CbmTestDigi(const CbmTestDigi& digi) : CbmDigi(digi) {;}

    /** Move constructor  **/
//    CbmTestDigi(CbmTestDigi&& digi) : CbmDigi(std::move(digi)) {;}

    /** Destructor  **/
    virtual ~CbmTestDigi() {;}


    /** Assignment operator  **/
    CbmTestDigi& operator=(const CbmTestDigi& other)
    {
      if (this != &other) {
        CbmDigi::operator=(other);
      }
      return *this;
    }

    /** Move Assignment operator  **/
/*     CbmTestDigi& operator=(CbmTestDigi&& other)
    {
      if (this != &other) {
        CbmDigi::operator=(std::move(other));
      }
      return *this;
    }
*/

    /** Unique channel address  **/
    Int_t GetAddress() const {return CbmDigi::GetAddress();}


    /** System (enum DetectorId) **/
    Int_t  GetSystemId() const {return CbmDigi::GetSystemId();};


    /** Absolute time [ns]  **/
    Double_t GetTime() const {return CbmDigi::GetTime();}
};

TEST(_GTestCbmDigi , CheckDefaultConstructor)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);
}

TEST(_GTestCbmDigi , CheckCopyConstructor)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);

  // Create object by copy constructing
  // test should be equal to test2 and test should be existing
  CbmTestDigi test2{test};

  // Test if the new object has the same values for all data members
  // as the original object
  retValInt = test2.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test2.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test2.GetMatch());

  retValInt = test2.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test2.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);

  // Test if the original object wasn't changed
  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);
}

TEST(_GTestCbmDigi , CheckAssignmentConstructor)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);

  // Create object by copy constructing
  // test should be equal to test2 and test should be existing
  CbmTestDigi test2 = test;

  // Test if the new object has the same values for all data members
  // as the original object
  retValInt = test2.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test2.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test2.GetMatch());

  retValInt = test2.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test2.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);

  // Test if the original object wasn't changed
  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);
}

TEST(_GTestCbmDigi , CheckSetMatch)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(0., retValDouble);

  EXPECT_EQ(nullptr, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(-111, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(-111., retValDouble);

  // Test if we can add an empty CbmMatch, get it back, and extract
  // the expected values
  CbmMatch* testMatch = new CbmMatch();

  Int_t linkLength{-111};
  Double_t totalWeight{-111};

  linkLength = testMatch->GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = testMatch->GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);

  test.SetMatch(testMatch);

  CbmMatch* testMatch1 = test.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    linkLength = testMatch1->GetNofLinks();
    EXPECT_EQ(0, linkLength);

    totalWeight = testMatch1->GetTotalWeight();
    EXPECT_FLOAT_EQ(0., totalWeight);
  }

}

