#include "CbmEvent.h"

#include "CbmMatch.h"
#include "CbmVertex.h"
#include "CbmDetectorList.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

#include <vector>

 void compareEventDataMembers(CbmEvent& test, Int_t evnumber, Double_t starttime,
 Double_t endtime, Int_t numobjects, CbmMatch* match /*, CbmVertex& vertex*/)
{
  Int_t retValInt{-111};
  Double_t retValDouble{-111.};

  retValInt = test.GetNumber();
  EXPECT_EQ(evnumber, retValInt);

  retValDouble = test.GetStartTime();
  EXPECT_EQ(starttime, retValDouble);

  retValDouble = test.GetEndTime();
  EXPECT_EQ(endtime, retValDouble);

  retValInt = test.GetNofData();
  EXPECT_EQ(numobjects, retValInt);

  EXPECT_EQ(match, test.GetMatch());
}

void compareEventMap(CbmEvent& test, Int_t numobjects, Int_t numobjectstype, Cbm::DataType type, std::vector<UInt_t> indices)
{
  Int_t retValInt{-111};
  UInt_t retValUInt{111};

  retValInt = test.GetNofData();
  EXPECT_EQ(numobjects, retValInt);

  retValInt = test.GetNofData(type);
  EXPECT_EQ(numobjectstype, retValInt);

  if(numobjectstype > 0){
    for(Int_t i=0; i< numobjectstype; ++i){
      retValUInt=test.GetIndex(type, i);
      EXPECT_EQ(indices[i], retValUInt);
    }
  }

}

TEST(_GTestCbmCluster, CheckDefaultConstructor)
{
  CbmEvent test;
  {
    SCOPED_TRACE("CheckDefaultConstructor");
    compareEventDataMembers(test, -1, 0., 0., 0, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckStandardConstructor)
{
  CbmEvent test{-111};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    compareEventDataMembers(test, -111, 0., 0., 0, nullptr);
  }
}


TEST(_GTestCbmCluster, CheckSettersAndGetters)
{
  CbmEvent test{-111, 1., 2.};
  {
    SCOPED_TRACE("CheckSettersAndGetters: Initial Test");
    compareEventDataMembers(test, -111, 1., 2., 0, nullptr);
  }

  test.SetStartTime(-23.);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetStartTime");
    compareEventDataMembers(test, -111, -23., 2., 0, nullptr);
  }

  test.SetEndTime(-45.);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetEndTime");
    compareEventDataMembers(test, -111, -23., -45., 0, nullptr);
  }

  CbmMatch* testMatch = new CbmMatch();
  test.SetMatch(testMatch);
  {
    SCOPED_TRACE("CheckSettersAndGetters: SetMatch");
    compareEventDataMembers(test, -111, -23., -45., 0, testMatch);
  }
}

 TEST(_GTestCbmCluster, CheckAddData)
 {
   CbmEvent test{-111, 1., 2.};
   {
     SCOPED_TRACE("CheckAddData: Initial Test");
     compareEventDataMembers(test, -111, 1., 2., 0, nullptr);
   }

   std::vector<UInt_t> mctrack;
   std::vector<UInt_t> stspoint;
   std::vector<UInt_t> stsdigi;
   std::vector<UInt_t> stscluster;
   std::vector<UInt_t> stshit;
   std::vector<UInt_t> ststrack;


   mctrack.push_back(11);
   test.AddData(Cbm::kMCTrack, 11);
   {
     SCOPED_TRACE("CheckAddData: Add first MCTrack");
     compareEventDataMembers(test, -111, 1., 2., 1, nullptr);
     compareEventMap(test, 1,  1, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 1, -1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 1, -1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 1, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 1, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 1, -1, Cbm::kStsTrack,   ststrack);
   }

   mctrack.push_back(23);
   test.AddData(Cbm::kMCTrack, 23);
   {
     SCOPED_TRACE("CheckAddData: Add second MCTrack");
     compareEventDataMembers(test, -111, 1., 2., 2, nullptr);
     compareEventMap(test, 2,  2, Cbm::kMCTrack,    mctrack);
     compareEventMap(test, 2, -1, Cbm::kStsPoint,   stspoint);
     compareEventMap(test, 2, -1, Cbm::kStsDigi,    stsdigi);
     compareEventMap(test, 2, -1, Cbm::kStsCluster, stscluster);
     compareEventMap(test, 2, -1, Cbm::kStsHit,     stshit);
     compareEventMap(test, 2, -1, Cbm::kStsTrack,   ststrack);
   }


 }

    /*
    kMCTrack,
            kStsPoint,
            kStsDigi,
            kStsCluster,
            kStsHit,
            kStsTrack};
     */

/*
TEST(_GTestCbmCluster, CheckDefaultConstructor)
{
  CbmCluster test;
  std::vector<Int_t> testvector;
  {
    SCOPED_TRACE("CheckDefaultConstructor");
    compareClusterDataMembers(test, 0, testvector, 0, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckStandardConstructor)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckStandardConstructor");
    compareClusterDataMembers(test, 5, testvector, -11, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckSetDigis)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckSetDigis: InitialTest");
    compareClusterDataMembers(test, 5, testvector, -11, nullptr);
  }

  std::vector<Int_t> testvector1{11,21,31,41,51,61};
  test.SetDigis(testvector1);
  {
    SCOPED_TRACE("CheckSetDigis: SetDigis");
    compareClusterDataMembers(test, 6, testvector1, -11, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckAddDigi)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  std::vector<Int_t> testvector1{1,2,3,4,5,111};
  std::vector<Int_t> testvector2{1,2,3,4,5,111,222};

  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckAddDigi: InitialTest");
    compareClusterDataMembers(test, testvector.size(), testvector, -11, nullptr);
  }

  test.AddDigi(111);
  {
    SCOPED_TRACE("CheckAddDigis: AddDigi 1");
    compareClusterDataMembers(test, testvector.size()+1, testvector1, -11, nullptr);
  }

  test.AddDigi(222);
  {
    SCOPED_TRACE("CheckAddDigis: AddDigi 2");
    compareClusterDataMembers(test, testvector.size()+2, testvector2, -11, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckAddDigis)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  std::vector<Int_t> testvector1{11,21,31,41,51,61};
  std::vector<Int_t> testvector2{1,2,3,4,5,11,21,31,41,51,61};

  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckAddDigis: InitialTest");
    compareClusterDataMembers(test, testvector.size(), testvector, -11, nullptr);
  }

  test.AddDigis(testvector1);
  {
    SCOPED_TRACE("CheckAddDigis: AddDigis");
    compareClusterDataMembers(test, testvector2.size(), testvector2, -11, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckClearDigis)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  std::vector<Int_t> testvector1;

  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckClearDigis: InitialTest");
    compareClusterDataMembers(test, testvector.size(), testvector, -11, nullptr);
  }

  test.ClearDigis();
  {
    SCOPED_TRACE("CheckClearDigis: AddDigis");
    compareClusterDataMembers(test, 0, testvector1, -11, nullptr);
  }
}

TEST(_GTestCbmCluster, CheckSetters)
{
  std::vector<Int_t> testvector{1,2,3,4,5};

  CbmCluster test{testvector,-11};
  {
    SCOPED_TRACE("CheckSetters: InitialTest");
    compareClusterDataMembers(test, testvector.size(), testvector, -11, nullptr);
  }

  test.SetAddress(-111);
  {
    SCOPED_TRACE("CheckSetters: SetAddress");
    compareClusterDataMembers(test, testvector.size(), testvector, -111, nullptr);
  }

  CbmMatch testMatch;
  test.SetMatch(&testMatch);
  {
    SCOPED_TRACE("CheckSetters: SetAddress");
    compareClusterDataMembers(test, testvector.size(), testvector, -111, &testMatch);
  }

}

TEST(_GTestCbmCluster, CheckToString)
{
  std::vector<Int_t> testvector{1,2,3,4,5};
  CbmCluster test{testvector,-11};

  EXPECT_STREQ("CbmCluster: nofDigis=5 | 1 2 3 4 5  | address=-11\n", test.ToString().c_str());
}
*/
