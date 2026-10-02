#include "WCSimSteppingAction.hh"

#include <stdlib.h>
#include <stdio.h>
#include <WCSimRootEvent.hh>
//#include <G4SIunits.hh>
#include <G4OpticalPhoton.hh>
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4VParticleChange.hh"
#include "G4SteppingVerbose.hh"
#include "G4SteppingManager.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "WCSimAllSecondariesTree.hh"
#include "WCSimAllSecondaryPhotonsTree.hh"
#include <unordered_map>

G4bool WCSimSaveAllSecondaryTruthTrees();

G4int WCSimSteppingAction::n_photons_through_mPMTLV = 0;
G4int WCSimSteppingAction::n_photons_through_acrylic = 0;
G4int WCSimSteppingAction::n_photons_through_gel = 0;
G4int WCSimSteppingAction::n_photons_on_blacksheet = 0;
G4int WCSimSteppingAction::n_photons_on_smallPMT = 0;

namespace {
struct SecondaryPhotonInfo {
  int event = -1;
  int track = -1;
  int parent = -1;
  int parentPdg = 0;
  int parentParent = -1;
  int parentStep = -1;
  float parentKE = 0.f;
  G4ThreeVector position;
  G4ThreeVector direction;
  float time = 0.f;
  float wavelength = 0.f;
  std::string creator;
  std::string parentCreator;
};

std::unordered_map<int, SecondaryPhotonInfo> secondaryPhotons;

std::string VolumeName(const G4VPhysicalVolume* volume)
{
  return volume ? std::string(volume->GetName()) : std::string("NONE");
}

std::string MaterialName(const G4StepPoint* point)
{
  const G4Material* material = point ? point->GetMaterial() : nullptr;
  return material ? std::string(material->GetName()) : std::string("NONE");
}

void FillAllSecondaries(const G4Step* step, const G4Event* event)
{
  const G4Track* track = step ? step->GetTrack() : nullptr;
  if (!track || track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return;

  const G4StepPoint* pre = step->GetPreStepPoint();
  const G4StepPoint* post = step->GetPostStepPoint();
  const G4ParticleDefinition* definition = track->GetDefinition();
  AllSecondariesTree_ResetVars();
  alls_evt = event ? event->GetEventID() : -1;
  alls_trk = track->GetTrackID();
  alls_parent = track->GetParentID();
  alls_pdg = definition ? definition->GetPDGEncoding() : 0;
  alls_step = track->GetCurrentStepNumber();
  if (pre) {
    const G4ThreeVector position = pre->GetPosition();
    const G4ThreeVector direction = pre->GetMomentumDirection();
    alls_x_cm = position.x() / cm;
    alls_y_cm = position.y() / cm;
    alls_z_cm = position.z() / cm;
    alls_t_ns = pre->GetGlobalTime() / ns;
    alls_dir_x = direction.x();
    alls_dir_y = direction.y();
    alls_dir_z = direction.z();
    alls_ke_MeV = pre->GetKineticEnergy() / MeV;
    alls_volume = VolumeName(pre->GetPhysicalVolume());
    alls_material = MaterialName(pre);
  }
  if (post) {
    const G4ThreeVector position = post->GetPosition();
    const G4ThreeVector direction = post->GetMomentumDirection();
    alls_post_x_cm = position.x() / cm;
    alls_post_y_cm = position.y() / cm;
    alls_post_z_cm = position.z() / cm;
    alls_post_t_ns = post->GetGlobalTime() / ns;
    alls_post_dir_x = direction.x();
    alls_post_dir_y = direction.y();
    alls_post_dir_z = direction.z();
    alls_post_ke_MeV = post->GetKineticEnergy() / MeV;
    alls_post_volume = VolumeName(post->GetPhysicalVolume());
    alls_post_material = MaterialName(post);
    const G4VProcess* process = post->GetProcessDefinedStep();
    alls_step_process = process ? process->GetProcessName() : "NONE";
  }
  alls_edep_MeV = step->GetTotalEnergyDeposit() / MeV;
  alls_step_length_cm = step->GetStepLength() / cm;
  alls_track_length_cm = track->GetTrackLength() / cm;
  alls_charge = definition ? definition->GetPDGCharge() : 0.;
  alls_particle = definition ? definition->GetParticleName() : "NONE";
  const G4VProcess* creator = track->GetCreatorProcess();
  alls_creator = creator ? creator->GetProcessName() : "NONE";
  AllSecondariesTree_Fill();
}

void CacheSecondaryPhoton(const G4Step* step, const G4Event* event)
{
  const G4Track* track = step ? step->GetTrack() : nullptr;
  if (!track || !event ||
      track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) return;
  const G4VProcess* creator = track->GetCreatorProcess();
  if (!creator || creator->GetProcessName() != "Cerenkov") return;
  if (track->GetCurrentStepNumber() != 1) return;

  SecondaryPhotonInfo info;
  info.event = event->GetEventID();
  info.track = track->GetTrackID();
  info.parent = track->GetParentID();
  info.parentKE = track->GetVertexKineticEnergy() / MeV;
  info.position = track->GetVertexPosition();
  info.direction = track->GetVertexMomentumDirection();
  info.time = track->GetGlobalTime() / ns;
  info.wavelength = track->GetVertexKineticEnergy() > 0.
      ? static_cast<float>((h_Planck * c_light / track->GetVertexKineticEnergy()) / nm)
      : 0.f;
  info.creator = creator->GetProcessName();
  secondaryPhotons[info.track] = info;
}

void FinishSecondaryPhoton(const G4Step* step, const G4Event* event)
{
  const G4Track* track = step ? step->GetTrack() : nullptr;
  if (!track || !event ||
      track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition() ||
      track->GetTrackStatus() == fAlive) return;

  auto found = secondaryPhotons.find(track->GetTrackID());
  if (found == secondaryPhotons.end()) return;

  const SecondaryPhotonInfo& info = found->second;
  const G4StepPoint* post = step->GetPostStepPoint();
  AllSecondaryPhotonsTree_ResetVars();
  asph_evt = info.event;
  asph_trk = info.track;
  asph_parent = info.parent;
  asph_parent_pdg = info.parentPdg;
  asph_parent_parent = info.parentParent;
  asph_parent_step = info.parentStep;
  asph_pdg = track->GetDefinition()->GetPDGEncoding();
  asph_parent_ke_MeV = info.parentKE;
  asph_sx_cm = info.position.x() / cm;
  asph_sy_cm = info.position.y() / cm;
  asph_sz_cm = info.position.z() / cm;
  asph_st_ns = info.time;
  asph_dir_x = info.direction.x();
  asph_dir_y = info.direction.y();
  asph_dir_z = info.direction.z();
  asph_lambda_nm = info.wavelength;
  asph_end_x_cm = track->GetPosition().x() / cm;
  asph_end_y_cm = track->GetPosition().y() / cm;
  asph_end_z_cm = track->GetPosition().z() / cm;
  asph_end_t_ns = track->GetGlobalTime() / ns;
  asph_end_status = track->GetTrackStatus();
  asph_creator = info.creator;
  asph_end_process = post && post->GetProcessDefinedStep()
      ? post->GetProcessDefinedStep()->GetProcessName() : "NONE";

  const int eventId = event->GetEventID();
  int pmt = -1;
  float peTime = 0.f;
  float peLambda = 0.f;
  if (AllSecondaryPhotonsTree_GetPE(eventId, info.track, pmt, peTime, peLambda)) {
    asph_made_pe = 1;
    asph_pe_pmt = pmt;
    asph_pe_time_ns = peTime;
    asph_pe_lambda_nm = peLambda;
  }
  AllSecondaryPhotonsTree_Fill();
  secondaryPhotons.erase(found);
}
}

///////////////////////////////////////////////
///// BEGINNING OF WCSIM STEPPING ACTION //////
///////////////////////////////////////////////


WCSimSteppingAction::WCSimSteppingAction(WCSimRunAction *myRun, WCSimDetectorConstruction *myDet) : runAction(myRun), det(myDet) {

}

void WCSimSteppingAction::UserSteppingAction(const G4Step* aStep)
{
    if (!aStep) return;
    const G4Event *event = G4EventManager::GetEventManager()->GetConstCurrentEvent();
    if (!event) return;
    if(event->IsAborted() || event->GetEventID() < 0)
      return;

  if (WCSimSaveAllSecondaryTruthTrees())
  {
    FillAllSecondaries(aStep, event);
    CacheSecondaryPhoton(aStep, event);
    FinishSecondaryPhoton(aStep, event);
  }
  //DISTORTION must be used ONLY if INNERTUBE or INNERTUBEBIG has been defined in BidoneDetectorConstruction.cc
  
  const G4Track* track       = aStep->GetTrack();
  
  // Not used:
  //const G4Event* evt = G4RunManager::GetRunManager()->GetCurrentEvent();
  //G4VPhysicalVolume* volume  = track->GetVolume();
  //G4SDManager* SDman   = G4SDManager::GetSDMpointer();
  //G4HCofThisEvent* HCE = evt->GetHCofThisEvent();

  
  // Debug for photon tracking
  G4StepPoint* thePrePoint = aStep->GetPreStepPoint();
  G4VPhysicalVolume* thePrePV = thePrePoint->GetPhysicalVolume();

  G4StepPoint* thePostPoint = aStep->GetPostStepPoint();
  G4VPhysicalVolume* thePostPV = thePostPoint->GetPhysicalVolume();

  //G4OpBoundaryProcessStatus boundaryStatus=Undefined;
  //static G4ThreadLocal G4OpBoundaryProcess* boundary=NULL;  //doesn't work and needs #include tls.hh from Geant4.9.6 and beyond
  G4OpBoundaryProcess* boundary=NULL;
  
  //find the boundary process only once
  if(!boundary){
    G4ProcessManager* pm
      = aStep->GetTrack()->GetDefinition()->GetProcessManager();
    G4int nprocesses = pm->GetProcessListLength();
    G4ProcessVector* pv = pm->GetProcessList();
    G4int i;
    for( i=0;i<nprocesses;i++){
      if((*pv)[i]->GetProcessName()=="OpBoundary"){
	boundary = (G4OpBoundaryProcess*)(*pv)[i];
	break;
      }
    }
  }

  G4ParticleDefinition *particleType = track->GetDefinition();
  if(particleType == G4OpticalPhoton::OpticalPhotonDefinition() && thePrePV && thePostPV){
    if( (thePrePV->GetName().find("MultiPMT") != std::string::npos) &&
	(thePostPV->GetName().find("vessel") != std::string::npos) )
      n_photons_through_mPMTLV++;
    
    if( (thePostPV->GetName().find("container") != std::string::npos) &&
	(thePrePV->GetName().find("vessel") != std::string::npos))
      n_photons_through_acrylic++;
    
    if( (thePrePV->GetName().find("container") != std::string::npos) )
      n_photons_through_gel++;

    if( (thePrePV->GetName().find("container") != std::string::npos) &&
	(thePostPV->GetName().find("inner") != std::string::npos) )
      n_photons_on_blacksheet++;

    if( (thePrePV->GetName().find("container") != std::string::npos) &&
	(thePostPV->GetName().find("pmt") != std::string::npos) )
      n_photons_on_smallPMT++;
	

    /*
    if( (thePrePV->GetName().find("pmt") != std::string::npos)){
      G4cout << "Photon between " << thePrePV->GetName() <<
	" and " << thePostPV->GetName() << " because " << 
	thePostPoint->GetProcessDefinedStep()->GetProcessName() << 
	" and boundary status: " <<  boundary->GetStatus() << " with track status " << track->GetTrackStatus() << G4endl;
      
	}*/



    if(track->GetTrackStatus() == fStopAndKill){
      if(boundary->GetStatus() == NoRINDEX){
	G4cout << "Optical photon is killed because of missing refractive index in either " << thePrePoint->GetMaterial()->GetName() << " or " << thePostPoint->GetMaterial()->GetName() <<
	  " (transition from " << thePrePV->GetName() << " to " << thePostPV->GetName() << ")" <<
	  " : could also be caused by Overlaps with volumes with logicalBoundaries." << G4endl;
	
      }
      /* Debug :  
      if( (thePrePV->GetName().find("PMT") != std::string::npos) ||
	  (thePrePV->GetName().find("pmt") != std::string::npos)){
	
	if(boundary->GetStatus() != StepTooSmall){
	  //	if(thePostPoint->GetProcessDefinedStep()->GetProcessName() != "Transportation")
	  G4cout << "Killed photon between " << thePrePV->GetName() <<
	    " and " << thePostPV->GetName() << " because " << 
	    thePostPoint->GetProcessDefinedStep()->GetProcessName() << 
	    " and boundary status: " <<  boundary->GetStatus() <<
	    G4endl;
	}
	}	*/
      
    }
  }



}


G4int WCSimSteppingAction::G4ThreeVectorToWireTime(G4ThreeVector *pos3d,
						    G4ThreeVector lArPos,
						    G4ThreeVector start,
						    G4int i)
{
  G4double x0 = start[0]-lArPos[0];
  G4double y0 = start[1]-lArPos[1];
//   G4double y0 = 2121.3;//mm
  G4double z0 = start[2]-lArPos[2];

  G4double dt=0.8;//mm
//   G4double midt = 2651.625;
  G4double pitch = 3;//mm
//   G4double midwir = 1207.10;
  G4double c45 = 0.707106781;
  G4double s45 = 0.707106781;

  G4double w1;
  G4double w2;
  G4double t;

//   G4double xField(0.);
//   G4double yField(0.);
//   G4double zField(0.);

//   if(detector->getElectricFieldDistortion())
//     {
//       Distortion(pos3d->getX(),
// 		 pos3d->getY());
      
//       xField = ret[0];
//       yField = ret[1];
//       zField = pos3d->getZ();
      
//       w1 = (int)(((zField+z0)*c45 + (x0-xField)*s45)/pitch); 
//       w2 = (int)(((zField+z0)*c45 + (x0+xField)*s45)/pitch); 
//       t = (int)(yField+1);

//       //G4cout<<" x orig "<<pos3d->getX()<<" y orig "<<(pos3d->getY()+y0)/dt<<G4endl;
//       //G4cout<<" x new "<<xField<<" y new "<<yField<<G4endl;
//     }
//   else 
//     {
      
  w1 = (int) (((pos3d->getZ()+z0)*c45 + (x0-pos3d->getX())*s45)/pitch); 
  w2 = (int)(((pos3d->getZ()+z0)*c45 + (x0+pos3d->getX())*s45)/pitch); 
  t  = (int)((pos3d->getY()+y0)/dt +1);
//     }

  if (i==0)
    return (int)w1;
  else if (i==1)
    return (int)w2;
  else if (i==2)
    return (int)t;
  else return 0;
} 


void WCSimSteppingAction::Distortion(G4double /*x*/,G4double /*y*/)
{
 
//   G4double theta,steps,yy,y0,EvGx,EvGy,EField,velocity,tSample,dt;
//   y0=2121.3;//mm
//   steps=0;//1 mm steps
//   tSample=0.4; //micros
//   dt=0.8;//mm
//   LiquidArgonMedium medium;  
//   yy=y;
//   while(y<y0 && y>-y0 )
//     {
//       EvGx=FieldLines(x,y,1);
//       EvGy=FieldLines(x,y,2);
//       theta=atan(EvGx/EvGy);
//       if(EvGy>0)
// 	{
// 	  x+=sin(theta);
// 	  y+=cos(theta);
// 	}
//       else
// 	{
// 	  y-=cos(theta);
// 	  x-=sin(theta);
// 	}
//       EField=sqrt(EvGx*EvGx+EvGy*EvGy);//kV/mm
//       velocity=medium.DriftVelocity(EField*10);// mm/microsec
//       steps+=1/(tSample*velocity);

//       //G4cout<<" step "<<steps<<" x "<<x<<" y "<<y<<" theta "<<theta<<" Gx "<<eventaction->Gx->Eval(x,y)<<" Gy "<<eventaction->Gy->Eval(x,y)<<" EField "<<EField<<" velocity "<<velocity<<G4endl;
//     }

//   //numbers
//   //EvGx=FieldLines(0,1000,1);
//   //EvGy=FieldLines(0,1000,2);
//   //EField=sqrt(EvGx*EvGx+EvGy*EvGy);//kV/mm
//   //velocity=medium.DriftVelocity(EField*10);// mm/microsec
//   //G4double quenching;
//   //quenching=medium.QuenchingFactor(2.1,EField*10);
//   //G4cout<<" Gx "<<EvGx<<" Gy "<<EvGy<<" EField "<<EField<<" velocity "<<velocity<<" quenching "<<quenching<<G4endl;


  //ret[0]=5;
//   if(yy>0)
//     ret[1]=2*y0/dt -steps;
//   else
//     ret[1]=steps; 
}


double WCSimSteppingAction::FieldLines(G4double /*x*/,G4double /*y*/,G4int /*coord*/)
{ //0.1 kV/mm = field
  //G4double Radius=302;//mm
//   G4double Radius=602;//mm
//   if(coord==1) //x coordinate
//     return  (0.1*(2*Radius*Radius*x*abs(y)/((x*x+y*y)*(x*x+y*y))));
//   else //y coordinate
//     return 0.1*((abs(y)/y)*(1-Radius*Radius/((x*x+y*y)*(x*x+y*y))) + abs(y)*(2*Radius*Radius*y/((x*x+y*y)*(x*x+y*y))));
  return 0;
}
