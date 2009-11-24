//*************************************************************************
//* ========================
//*  EXTPCKalDetector Class
//* ========================
//*
//* (Description)
//*   User defined detector class
//* (Requires)
//*     EXVKalDetector
//* (Provides)
//*     class EXTPCKalDetector
//* (Update Recored)
//*   2009/11/23  K.Ikematsu   Derived from KalTest/examples/kaltest/
//*                                         hybrid/tpc/EXTPCKalDetector.cxx
//*
//* $Id: EXTPCKalDetector.cxx,v 1.1.1.1 2009-11-24 00:13:59 ikematsu Exp $
//*************************************************************************
//
// STL
#include <vector>

// GEAR
#include "gear/GEAR.h"
#include "gear/TPCParameters.h"
#include "gear/PadRowLayout2D.h"
#include "gearimpl/TPCModuleImpl.h"
#include "gearxml/GearXML.h"

// global constants from Marlin, used for the global pointer to the GearMgr
#include <marlin/Global.h>

#include "EXTPCKalDetector.h"
#include "EXTPCMeasLayer.h"
#include "EXTPCHit.h"
#include "TRandom.h"
#include "TMath.h"

#include "TTUBE.h"
#include "TNode.h"
#include "TVirtualPad.h"

Double_t EXTPCKalDetector::fgVdrift = 7.6e-3;
ClassImp(EXTPCKalDetector)

EXTPCKalDetector::EXTPCKalDetector(Int_t m)
                : EXVKalDetector(m),
                  fNodePtr(0)
{
  Double_t A, Z, density, radlen;

#if 0
  A       = 14.00674 * 0.7 + 15.9994 * 0.3;
  Z       = 7.3;
  density = 1.205e-3;
  radlen  = 3.42e4;
  TMaterial &air = *new TMaterial("TPCAir", "", A, Z, density, radlen, 0.);
#endif

  A       = 39.948 * 0.9 + (12.011 * 0.2 + 1.00794 * 0.8) * 0.1;
  Z       = 16.4;
  density = 0.749e-3;
  radlen  = 1.196e4 * 2;
  TMaterial &gas = *new TMaterial("TPCGas", "", A, Z, density, radlen, 0.);

#if 0
  A       = 12.0107;
  Z       = 6.;
  density = 0.1317;
  radlen  = 42.7 / density;
  TMaterial &cfrp = *new TMaterial("TPCCFRP", "", A, Z, density, radlen, 0.);
#endif

  gear::TPCParameters const &theTPCParameters
    = marlin::Global::GEAR->getTPCParameters();

  Int_t nmodules = theTPCParameters.getNModules();

  std::vector<const gear::TPCModule *> modules;

  for (Int_t i = 0; i < nmodules; i++) {
    modules.push_back(&theTPCParameters.getModule(i));
  }

  static const Double_t kmm2cm = 0.1;

  static const Double_t lhalf
    = theTPCParameters.getMaxDriftLength() * kmm2cm;     // half length
  static const Int_t    nrows = modules[0]->getNRows();  // # of pad rows
  ///// FIXME: temporary treatment /////////////////////////
  static const Int_t    nlayers = nrows * 3;  // # of layers
  //////////////////////////////////////////////////////////
  static const Double_t neff    = 22.7;
  static const Double_t sigmax0 = 38.3e-4;
  static const Double_t sigmax1 = 101.5e-4 / TMath::Sqrt(10.) / TMath::Sqrt(neff);
  static const Double_t sigmaz0 = 500.e-4;
  static const Double_t sigmaz1 = 154.e-4  / TMath::Sqrt(10.) / TMath::Sqrt(neff);

  Bool_t active = EXTPCMeasLayer::kActive;
  Bool_t dummy  = EXTPCMeasLayer::kDummy;

  // create measurement layers of central tracker
  for (Int_t layer = 0; layer < nlayers; layer++) {

    Int_t    superlayer = layer / nrows;
    Int_t    rownum     = layer % nrows;

    ///// FIXME: temporary treatment //////////////////////////////////////
    Int_t    module     = superlayer < 1 ? 1 : (superlayer < 2 ? 3 : 6);
    ///////////////////////////////////////////////////////////////////////

    TVector3 xc(modules[module]->getOffset()[0] * kmm2cm,
                modules[module]->getOffset()[1] * kmm2cm,
                0.);

    Double_t phimin = modules[module]->getLocalModuleExtent()[2] + modules[module]->getAngle();
    Double_t phimax = modules[module]->getLocalModuleExtent()[3] + modules[module]->getAngle();

    Int_t    padrow = layer % nrows;
    Int_t    idpad  = modules[module]->getPadIndex(rownum, 0);

    Double_t r      = modules[module]->getLocalPadLayout().getPadCenter(idpad)[0] * kmm2cm;
    // this returns local coordinate value!!

#if 0
    std::cerr << "(module, layer, r, xc1, xc2, xc3, phimin, phimax, lhalf) = ("
              << module << ", "
              << padrow << ", "
              << r      << ", "
              << xc.X() << ", "
              << xc.Y() << ", "
              << xc.Z() << ", "
              << phimin << ", "
              << phimax << ", "
              << lhalf  << ")" << std::endl;
#endif
    // r is curvature (local r)
    Add(new EXTPCMeasLayer(gas, gas, r, xc, phimin, phimax, lhalf, sigmax0, sigmax1, sigmaz0, sigmaz1, active, module, padrow));
  }
  SetOwner();
}

EXTPCKalDetector::~EXTPCKalDetector()
{
}

//_________________________________________________________________________
//  --------------
//  Utility Method
//  --------------
//_________________________________________________________________________
//  --------------------------------------
//  Draw: Drawing method for event display
//  --------------------------------------
//
void EXTPCKalDetector::Draw(Int_t color, const Char_t *opt)
{
  if (! gPad) return;
  TNode *nodep = GetNodePtr();
  nodep->cd();

  if (! fNodePtr) {
    EXTPCMeasLayer *inp  = static_cast<EXTPCMeasLayer *>(First());
    EXTPCMeasLayer *outp = static_cast<EXTPCMeasLayer *>(Last());
    Double_t rin  = inp->GetR();
    Double_t rout = outp->GetR();
    Double_t hlen = outp->GetZmax();
    const Char_t *name  = "TPC";
    const Char_t *nname = "TPCNode";
    TTUBE *tubep = new TTUBE(name, name, "void", rin, rout, hlen);
    tubep->SetBit(kCanDelete);
    fNodePtr = new TNode(nname, nname, name);
    fNodePtr->SetLineColor(color);
    fNodePtr->SetLineWidth(0.01);
  }
  EXVKalDetector::Draw(color, opt);
}
