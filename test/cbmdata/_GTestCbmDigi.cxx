#include "CbmDigi.h"

#include "CbmMatch.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

#include <utility> // std::forward

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
//   CbmTestDigi(CbmTestDigi&& digi) : CbmDigi(std::forward<CbmTestDigi>(digi)) {;}

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
/*    CbmTestDigi& operator=(CbmTestDigi&& other)
    {
      if (this != &other) {
        CbmDigi::operator=(std::forward<CbmTestDigi>(other));
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

void compareDigiDataMembers(CbmTestDigi& test, Int_t address, Double_t charge, CbmMatch* match,
    Int_t systemid, Double_t time)
{
  Int_t retValInt{-222};
  Double_t retValDouble{-222.};

  retValInt = test.GetAddress();
  EXPECT_EQ(address, retValInt);

  retValDouble = test.GetCharge();
  EXPECT_FLOAT_EQ(charge, retValDouble);

  EXPECT_EQ(match, test.GetMatch());

  retValInt = test.GetSystemId();
  EXPECT_EQ(systemid, retValInt);

  retValDouble = test.GetTime();
  EXPECT_FLOAT_EQ(time, retValDouble);
}

void compareMatchDataMembers(CbmMatch* testMatch, Int_t noflinks, Double_t weight)
{
  Int_t linkLength{-111};
  Double_t totalWeight{-111.};

  linkLength = testMatch->GetNofLinks();
  EXPECT_EQ(0, linkLength);

  totalWeight = testMatch->GetTotalWeight();
  EXPECT_FLOAT_EQ(0., totalWeight);
}

TEST(_GTestCbmDigi , CheckDefaultConstructor)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);
}

TEST(_GTestCbmDigi , CheckSetMatch)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);

  // Test if we can add an empty CbmMatch, get it back, and extract
  // the expected values
  CbmMatch* testMatch = new CbmMatch();

  compareMatchDataMembers(testMatch, 0, 0.);

  test.SetMatch(testMatch);

  CbmMatch* testMatch1 = test.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    compareMatchDataMembers(testMatch1, 0, 0.);
  }

}


TEST(_GTestCbmDigi , CheckCopyConstructor)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  CbmMatch* testMatch = new CbmMatch();

  compareMatchDataMembers(testMatch, 0, 0.);

  test.SetMatch(testMatch);

  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);

  // Create object by copy constructing
  // test should be equal to test2 and test should be existing
  CbmTestDigi test2{test};

  // Test if the new object has the same values for all data members
  // as the original object
  CbmMatch* testMatch1 = test2.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    compareMatchDataMembers(testMatch1, 0, 0.);
  }

  compareDigiDataMembers(test2, -111, 0., testMatch1, -111, -111.);

  // Test if the original object wasn't changed
  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);
}

TEST(_GTestCbmDigi , CheckAssignmentOperator)
{
  // Create abstract base class via derived class
  CbmTestDigi test;

  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);

  // Create object by copy constructing
  // test should be equal to test2 and test should be existing
  CbmTestDigi test2;
  test2 = test;

  compareDigiDataMembers(test2, -111, 0., nullptr, -111, -111.);

  // Test if the original object wasn't changed
  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);

  CbmMatch* testMatch = new CbmMatch();

  compareMatchDataMembers(testMatch, 0, 0.);

  test.SetMatch(testMatch);

  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);

  // Create object by copy constructing
  // test should be equal to test2 and test should be existing
  CbmTestDigi test3;
  test3 = test;

  // Test if the new object has the same values for all data members
  // as the original object
  CbmMatch* testMatch1 = test3.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    compareMatchDataMembers(testMatch1, 0, 0.);
  }

  compareDigiDataMembers(test3, -111, 0., testMatch1, -111, -111.);

  // Test if the original object wasn't changed
  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);
}

/*
TEST(_GTestCbmDigi , CheckMoveConstructor)
{
  // Create abstract base class via derived class
  // After creation there is no CbmMatch added such
  // that the pointer is a nullptr
  CbmTestDigi test;
  EXPECT_EQ(nullptr, test.GetMatch());

  // After adding a new CbmMatch to CbmDigi
  // the GetMatch returns the pointer to the object
  CbmMatch* testMatch = new CbmMatch();

  compareMatchDataMembers(testMatch, 0, 0.);

  test.SetMatch(testMatch);

  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);

  // Create object by move constructing
  // test2 should now contain the pointer to the CbmMatch object and
  // test should contain a nullptr
  CbmTestDigi test2{std::move(test)};

  CbmMatch* testMatch1 = test2.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    compareMatchDataMembers(testMatch1, 0, 0.);
  }

  compareDigiDataMembers(test2, -111, 0., testMatch1, -111, -111.);

  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);

}

TEST(_GTestCbmDigi , CheckAssignmentMoveConstructor)
{
  // Create abstract base class via derived class
  // After creation there is no CbmMatch added such
  // that the pointer is a nullptr
  CbmTestDigi test;
  EXPECT_EQ(nullptr, test.GetMatch());

  // After adding a new CbmMatch to CbmDigi
  // the GetMatch returns the pointer to the object
  CbmMatch* testMatch = new CbmMatch();

  compareMatchDataMembers(testMatch, 0, 0.);

  test.SetMatch(testMatch);

  compareDigiDataMembers(test, -111, 0., testMatch, -111, -111.);

  // Create object by move constructing
  // test2 should now contain the pointer to the CbmMatch object and
  // test should contain a nullptr
  CbmTestDigi test2; // = std::move(test);
  test2 = std::move(test);

  CbmMatch* testMatch1 = test2.GetMatch();

  EXPECT_NE(nullptr, testMatch1);
  if (testMatch1) {
    compareMatchDataMembers(testMatch1, 0, 0.);
  }

  compareDigiDataMembers(test2, -111, 0., testMatch1, -111, -111.);

  compareDigiDataMembers(test, -111, 0., nullptr, -111, -111.);
}
*/
