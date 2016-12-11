#include "CbmConstants.h"

#include "Rtypes.h"

#include "gtest/gtest.h"
#include "gtest/gtest-spi.h"

TEST(_GTestCbmConstants , CheckParticleMasses)
{
  Double_t protonMass = 0.938272046;
  Double_t neutronMass = 0.939565379;
  Double_t electronMass = 0.000510998928;

  EXPECT_EQ(protonMass, Cbm::ProtonMass());
  EXPECT_EQ(neutronMass, Cbm::NeutronMass());
  EXPECT_EQ(electronMass, Cbm::ElectronMass());
}

