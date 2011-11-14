
#include "ILDTPCKalDetector.h"
#include "ILDCylinderMeasLayer.h"
#include "ILDCylinderHit.h"

#include "TMath.h"
#include "TTUBE.h"

#include "MaterialDataBase.h"

#include <sstream>
#include <iomanip>

#include "gear/GEAR.h"
#include "gear/BField.h"
#include "gear/TPCParameters.h"
#include "gear/PadRowLayout2D.h"
#include "gearimpl/Util.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "streamlog/streamlog.h"


ILDTPCKalDetector::ILDTPCKalDetector( const gear::GearMgr& gearMgr ) : 
TVKalDetector(250) // SJA:FIXME initial size, 250 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  
  streamlog_out(DEBUG1) << "ILDTPCKalDetector building TPC detector using GEAR " << std::endl ;
  
  const gear::TPCParameters& tpcParams = gearMgr.getTPCParameters();
  
  const gear::PadRowLayout2D& pL = tpcParams.getPadLayout() ; 
  
  const Double_t bz = gearMgr.getBField().at( gear::Vector3D( 0.,0.,0.)  ).z() ;
  
  static const Int_t    nlayers   =  pL.getNRows() ;   // n rows
  static const Double_t lhalf     =  tpcParams.getMaxDriftLength() ;  // half length
  
  static const Double_t rstep     =  pL.getRowHeight(0) ;     // step length of radius
  
  // assuming that this is the radius of the first measurment layer ....
  static const Double_t rmin      =  tpcParams.getPlaneExtent()[0]   + rstep/2. ;   // minimum radius
  
  streamlog_out( DEBUG0 ) << tpcParams << std::endl ;
  
  static const Double_t rtub      = tpcParams.getDoubleVal("tpcInnerRadius")  ; // inner r of support tube
  static const Double_t outerr    = tpcParams.getDoubleVal("tpcOuterRadius")  ; // outer radius of TPC
  
  static const Double_t inthick   =  tpcParams.getDoubleVal("tpcInnerWallThickness")  ;   // thickness of inner shell
  static const Double_t outthick  =  tpcParams.getDoubleVal("tpcOuterWallThickness")  ;   // thickness of outer shell
  
  TMaterial & air       = *MaterialDataBase::Instance().getMaterial("air");
  TMaterial & tpcgas    = *MaterialDataBase::Instance().getMaterial("tpcgas");
  TMaterial & aluminium = *MaterialDataBase::Instance().getMaterial("aluminium");
  
  Bool_t active = true;
  Bool_t dummy  = false;
  
  std::string name = "TPC";
  
  // add inner field cage
  Add( new ILDCylinderMeasLayer(air, aluminium , rtub, lhalf, bz, dummy ) );
  streamlog_out( DEBUG0 )   << " *** adding " << name << " Measurement layer using CellID: [ inner field cage ] at R = " << rtub
  << " X0_in = " << air.GetRadLength() << "  X0_out = " <<  aluminium.GetRadLength()    
  << std::endl ;  
  
  Add( new ILDCylinderMeasLayer(aluminium , tpcgas, rtub+inthick, lhalf, bz, dummy ) );
  streamlog_out( DEBUG0 )   << " *** adding " << name << " Measurement layer using CellID: [ inner field cage ] at R = " << rtub+inthick
  << " X0_in = " << aluminium.GetRadLength() << "  X0_out = " <<  tpcgas.GetRadLength()    
  << std::endl ;  
  
  // create measurement layers
  Double_t r = rmin;
  
  UTIL::BitField64 encoder( ILDCellID0::encoder_string ) ; 
  
  for (Int_t layer = 0; layer < nlayers; layer++) {
    
    encoder.reset() ;  // reset to 0
    
    encoder[ILDCellID0::subdet] = ILDDetID::TPC ;
    encoder[ILDCellID0::layer] = layer ;
    
    int CellID = encoder.lowWord() ;
    
    ILDCylinderMeasLayer* tpcL =  new ILDCylinderMeasLayer(tpcgas, tpcgas, r, lhalf, bz, active, CellID, "TPC") ;
    
    Add( tpcL ) ;  
    
    int nth_layers(10) ;
    
    if( layer % nth_layers == 0 ){
      
      streamlog_out( DEBUG0 )   << " *** for TPC Gas printing only every " << nth_layers << "th layer"  << std::endl ; 
      streamlog_out( DEBUG0 )   << " *** adding " << name << " Measurement layer using CellID: [" << CellID <<  "] at R = " << r
      << " X0_in = " << tpcgas.GetRadLength() << "  X0_out = " <<  tpcgas.GetRadLength()    
      << std::endl ;  
    }
    r += rstep;
  }
  
  // add outer field cage
  Add( new ILDCylinderMeasLayer(tpcgas, aluminium, outerr-outthick, lhalf, bz, dummy) ) ;
  streamlog_out( DEBUG0 )   << " *** adding " << name << " Measurement layer using CellID: [ outer field cage ] at R = " << outerr-outthick
  << " X0_in = " << tpcgas.GetRadLength() << "  X0_out = " <<  aluminium.GetRadLength()    
  << std::endl ;  
  
  Add( new ILDCylinderMeasLayer(aluminium, air, outerr, lhalf, bz, dummy) ) ;
  streamlog_out( DEBUG0 )   << " *** adding " << name << " Measurement layer using CellID: [ outer field cage ] at R = " << outerr
  << " X0_in = " << aluminium.GetRadLength() << "  X0_out = " <<  air.GetRadLength()    
  << std::endl ;  
  
  
  SetOwner();
}


