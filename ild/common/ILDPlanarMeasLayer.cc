//*************************************************************************
//* ===================
//*  ILDPlanarMeasLayer Class
//* ===================
//*
//* (Description)
//*   Sample measurement layer class used by EXVTXHit.
//* (Requires)
//*     ILDVMeasLayer
//* (Provides)
//*     class ILDPlanarMeasLayer
//* (Update Recored)
//*   2003/09/30  Y.Nakashima       Original version.
//*
//*   2011/06/17  D.Kamai           Modified to handle ladder structure.
//*************************************************************************
//
#include <iostream>

#include "ILDPlanarMeasLayer.h"
#include "ILDPlanarHit.h"

#include "TVTrack.h"
#include "TVector3.h"
#include "TMath.h"
#include "TRotMatrix.h"
#include "TBRIK.h"
#include "TNode.h"
#include "TString.h"

                                                                                
ILDPlanarMeasLayer::ILDPlanarMeasLayer(TMaterial &min,
				       TMaterial &mout,
				       const TVector3  &center,
				       const TVector3  &normal,
				       Double_t   Bz,
				       Double_t SortingPolicy,
				       Double_t   xiwidth,
				       Double_t   zetawidth,
				       Double_t   xioffset,
				       Bool_t     is_active,
				       Int_t      layerID,
				       const Char_t    *name)
  : ILDVMeasLayer(min, mout, Bz, is_active, layerID, name),
    TPlane(center, normal),
    fSortingPolicy(SortingPolicy),
    fXiwidth(xiwidth),
    fZetawidth(zetawidth),
    fXioffset(xioffset)

{
}

ILDPlanarMeasLayer::~ILDPlanarMeasLayer()
{
}

TKalMatrix ILDPlanarMeasLayer::XvToMv(const TVector3 &xv) const
{
   // Calculate hit coordinate information:
   //	mv(0,0) = xi 
   //     (1,0) = zeta

   TKalMatrix mv(kMdim,1);

   mv(0,0) = (xv.Y() - GetXc().Y())*GetNormal().X()/GetNormal().Perp() - (xv.X() - GetXc().X())*GetNormal().Y()/GetNormal().Perp() ;
   mv(1,0) = xv.Z();

   return mv;
}

TKalMatrix ILDPlanarMeasLayer::XvToMv(const TVTrackHit &,
                               const TVector3   &xv) const
{
   return XvToMv(xv);
}

TVector3 ILDPlanarMeasLayer::HitToXv(const TVTrackHit &vht) const
{
   const ILDPlanarHit &ht = dynamic_cast<const ILDPlanarHit &>(vht);

   Double_t z =  ht(1,0);
   Double_t x = -ht(0,0)*GetNormal().Y()/GetNormal().Perp() + GetXc().X();
   Double_t y =  ht(0,0)*GetNormal().X()/GetNormal().Perp() + GetXc().Y();
   
   return TVector3(x,y,z);
}

void ILDPlanarMeasLayer::CalcDhDa(const TVTrackHit &vht,
                           const TVector3   &xxv,
                           const TKalMatrix &dxphiada,
                                 TKalMatrix &H)  const
{
   // Calculate
   //    H = (@h/@a) = (@phi/@a, @z/@a)^t
   // where
   //        h(a) = (phi, z)^t: expected meas vector
   //        a = (drho, phi0, kappa, dz, tanl, t0)
   //

   Int_t sdim = H.GetNcols();
   Int_t hdim = TMath::Max(5,sdim-1);

   // Set H = (@h/@a) = (@d/@a, @z/@a)^t
   
   for (Int_t i=0; i<hdim; i++) {
     H(0,i) = (GetNormal().X() / GetNormal().Perp()) * dxphiada(1,i)
             -(GetNormal().Y() / GetNormal().Perp()) * dxphiada(0,i);   
     H(1,i) =  dxphiada(2,i);
   }
   if (sdim == 6) {
      H(0,sdim-1) = 0.;
      H(1,sdim-1) = 0.;
   }

}



Bool_t ILDPlanarMeasLayer::IsOnSurface(const TVector3 &xx) const
{
  
  //  std::cout << "IsOnSurface " << std::endl;  

  Double_t xi   = (xx.Y()-GetXc().Y())*GetNormal().X()/GetNormal().Perp() - (xx.X()-GetXc().X())*GetNormal().Y()/GetNormal().Perp() ;
  Double_t zeta = xx.Z();

//  std::cout << "GetNormal().X() " <<  GetNormal().X() << std::endl;  
//  std::cout << "GetNormal().Y() " <<  GetNormal().Y() << std::endl;  
//  std::cout << "GetNormal().Perp() " << GetNormal().Perp() << std::endl;  
//  std::cout << "GetNormal().X()/GetNormal().Perp() " << GetNormal().X()/GetNormal().Perp() << std::endl;  
//  std::cout << "GetNormal().Y()/GetNormal().Perp() " << GetNormal().Y()/GetNormal().Perp() << std::endl;  
//  std::cout << "xx.X()-GetXc().X() " <<  xx.X()-GetXc().X() << std::endl;  
//  std::cout << "xx.Y()-GetXc().Y() " <<  xx.Y()-GetXc().Y() << std::endl;  
//
//  std::cout << "zeta " << zeta << std::endl;  
//  std::cout << "xi "   << xi   << std::endl;  
//  std::cout << "zeta half width " << GetZetawidth()/2 << std::endl;  
//  std::cout << "xi half width " << GetXiwidth()/2 << std::endl;  
//  std::cout << "offset  " << GetXioffset() << std::endl;  
//
//  std::cout << "distance from plane " << (xx.X()-GetXc().X())*GetNormal().X() + (xx.Y()-GetXc().Y())*GetNormal().Y() << std::endl; 

  bool onSurface = false ;

  if( (xx.X()-GetXc().X())*GetNormal().X() + (xx.Y()-GetXc().Y())*GetNormal().Y() < 1e-4){
    if( xi <= GetXioffset() + GetXiwidth()/2  && xi >= GetXioffset() - GetXiwidth()/2  && TMath::Abs(zeta) <= GetZetawidth()/2){
      onSurface = true;
    }
    else{
      onSurface = false;
    }
  }

  return onSurface;

}

