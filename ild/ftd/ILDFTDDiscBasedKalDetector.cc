
#include "ILDFTDDiscBasedKalDetector.h"

#include "MaterialDataBase.h"

#include <sstream>
#include <iomanip>

#include "gear/GEAR.h"
#include "gear/BField.h"
#include "gearimpl/Util.h"
#include "gear/FTDLayerLayout.h"

#include "ILDPlanarHit.h"
#include "ILDDiscMeasLayer.h"

#include <UTIL/BitField64.h>
#include <UTIL/ILDConf.h>

#include "streamlog/streamlog.h"

#include "TVector3.h"

ILDFTDDiscBasedKalDetector::ILDFTDDiscBasedKalDetector( const gear::GearMgr& gearMgr ) : 
TVKalDetector(30), _nDisks(0) // SJA:FIXME initial size, 300 looks reasonable for ILD, though this would be better stored as a const somewhere
{
  
  streamlog_out(DEBUG1) << "ILDFTDDiscBasedKalDetector building Simple Disc Based FTD detector using GEAR " << std::endl ;
  
  setupGearGeom( gearMgr ) ; 
  
  TMaterial & air       = *MaterialDataBase::Instance().getMaterial("air") ;
  TMaterial & silicon   = *MaterialDataBase::Instance().getMaterial("silicon") ;
  TMaterial & carbon    = *MaterialDataBase::Instance().getMaterial("carbon") ;
  
  Bool_t active = true ;
  Bool_t dummy  = false ;
  
  std::string name = "FTD" ;
  
  UTIL::BitField64 encoder( lcio::ILDCellID0::encoder_string ) ; 
  
  for (int idisk = 0; idisk < _nDisks; ++idisk) {
    
    static const  Double_t eps = 1e-6;
    
    double rOuter = _FTDgeo[idisk].rOuter ;
    double rInner = _FTDgeo[idisk].rInner ;
    double senThickness = _FTDgeo[idisk].senThickness ;
    double supThickness = _FTDgeo[idisk].supThickness ;
    double zPos = _FTDgeo[idisk].zPos;
    
    encoder[lcio::ILDCellID0::subdet] = lcio::ILDDetID::FTD ;
    encoder[lcio::ILDCellID0::side] = 1 ;
    encoder[lcio::ILDCellID0::layer]  = idisk ;
    encoder[lcio::ILDCellID0::module] = 0 ;
    encoder[lcio::ILDCellID0::sensor] = 0 ;
    
    int CellID_FWD = encoder.lowWord() ;
    
    encoder[lcio::ILDCellID0::side] = -1 ;
    
    int CellID_BWD = encoder.lowWord() ;
    
    // note the z position given in gear is actually the mid point (z) of the sensitive i.e. the z of the measurement plane
    TVector3 sen_front_face_centre_fwd( 0.0, 0.0, zPos - senThickness*0.5); // for +z  
    
    TVector3 measurement_plane_centre_fwd( sen_front_face_centre_fwd.X(), 
                                          sen_front_face_centre_fwd.Y(), 
                                          sen_front_face_centre_fwd.Z() + senThickness*0.5 ); 
    
    TVector3 sen_rear_face_centre_fwd( sen_front_face_centre_fwd.X(), 
                                      sen_front_face_centre_fwd.Y(), 
                                      sen_front_face_centre_fwd.Z() + senThickness ); 
    
    TVector3 sup_rear_face_centre_fwd( sen_rear_face_centre_fwd.X(), 
                                      sen_rear_face_centre_fwd.Y(), 
                                      sen_rear_face_centre_fwd.Z() + supThickness ); 
    
    TVector3 normal_fwd(sen_front_face_centre_fwd) ;    
    normal_fwd.SetMag(1.0) ;    
    
    Add(new ILDDiscMeasLayer( air, silicon, sen_front_face_centre_fwd, normal_fwd, _bZ, sen_front_face_centre_fwd.Z(), rInner, rOuter, dummy ) );
    
    Add(new ILDDiscMeasLayer( silicon, silicon, measurement_plane_centre_fwd, normal_fwd, _bZ, measurement_plane_centre_fwd.Z(), rInner, rOuter, active, CellID_FWD ) );
    
    Add(new ILDDiscMeasLayer( silicon, carbon, sen_rear_face_centre_fwd, normal_fwd, _bZ, sen_rear_face_centre_fwd.Z(), rInner, rOuter, dummy ) );
    
    Add(new ILDDiscMeasLayer( carbon, air, sup_rear_face_centre_fwd, normal_fwd, _bZ, sup_rear_face_centre_fwd.Z(), rInner, rOuter, dummy ) );
    
    
    // note the z position given in gear is actually the mid point (z) of the sensitive i.e. the z of the measurement plane
    TVector3 sen_front_face_centre_bwd( 0.0, 0.0, -zPos + senThickness*0.5 );         // for -z  
    
    TVector3 measurement_plane_centre_bwd( sen_front_face_centre_bwd.X(), 
                                          sen_front_face_centre_bwd.Y(), 
                                          sen_front_face_centre_bwd.Z() - senThickness*0.5 ); 
    
    TVector3 sen_rear_face_centre_bwd( sen_front_face_centre_bwd.X(), 
                                      sen_front_face_centre_bwd.Y(), 
                                      sen_front_face_centre_bwd.Z() - senThickness ); 
    
    TVector3 sup_rear_face_centre_bwd( sen_rear_face_centre_bwd.X(), 
                                      sen_rear_face_centre_bwd.Y(), 
                                      sen_rear_face_centre_bwd.Z() - supThickness ); 
    
    TVector3 normal_bwd(sen_front_face_centre_bwd) ;
    normal_bwd.SetMag(1.0) ;
    
    
    
    Add(new ILDDiscMeasLayer( air, silicon, sen_front_face_centre_bwd, normal_bwd, _bZ, fabs(sen_front_face_centre_bwd.Z())+eps, rInner, rOuter, dummy ) );
    
    Add(new ILDDiscMeasLayer( silicon, silicon, measurement_plane_centre_bwd, normal_bwd, _bZ, fabs(measurement_plane_centre_bwd.Z())+eps, rInner, rOuter, active, CellID_BWD ) );
    
    Add(new ILDDiscMeasLayer( silicon, carbon, sen_rear_face_centre_bwd, normal_bwd, _bZ, fabs(sen_rear_face_centre_bwd.Z())+eps, rInner, rOuter, dummy ) );
    
    Add(new ILDDiscMeasLayer( carbon, air, sup_rear_face_centre_bwd, normal_bwd, _bZ, fabs(sup_rear_face_centre_bwd.Z())+eps, rInner, rOuter, dummy ) );
    
    
  }
  
  SetOwner();
  
}



void ILDFTDDiscBasedKalDetector::setupGearGeom( const gear::GearMgr& gearMgr ){
  
  
  const gear::GearParameters& pFTD = gearMgr.getGearParameters("FTD");
  
  _bZ = gearMgr.getBField().at( gear::Vector3D( 0.,0.,0.)  ).z() ;
  
  const EVENT::DoubleVec& FTD_si  =  pFTD.getDoubleVals("FTDDiskSiThickness" )  ;
  const EVENT::DoubleVec& FTD_sp  =  pFTD.getDoubleVals("FTDDiskSupportThickness" )  ;
  const EVENT::DoubleVec& FTD_ri  =  pFTD.getDoubleVals("FTDInnerRadius" )  ;
  const EVENT::DoubleVec& FTD_ro  =  pFTD.getDoubleVals("FTDOuterRadius" )  ;
  const EVENT::DoubleVec& FTD_z   =  pFTD.getDoubleVals("FTDZCoordinate" )  ;
  
  _nDisks = FTD_si.size() ; 
  _FTDgeo.resize(_nDisks);
  
  for(int disk=0; disk< _nDisks; ++disk){
    
    _FTDgeo[disk].rInner = FTD_ri[disk];
    _FTDgeo[disk].rOuter = FTD_ro[disk];
    _FTDgeo[disk].senThickness =  FTD_si[disk];
    _FTDgeo[disk].supThickness =  FTD_sp[disk];
    _FTDgeo[disk].zPos = FTD_z[disk];
    
    
  }
  
  
}
