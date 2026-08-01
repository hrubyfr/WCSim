#include "WCSimSteppingAction.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "G4ios.hh"
#include "G4SteppingVerbose.hh"
#include "G4SteppingManager.hh"
#include "G4PVParameterised.hh"
#include "G4PVReplica.hh"
#include "G4SDManager.hh"
#include "G4RunManager.hh"
#include "G4EventManager.hh"
#include "G4OpBoundaryProcess.hh"
#include "G4ProcessManager.hh"
#include "G4ProcessVector.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4Event.hh"
#include "G4Material.hh"
#include "G4VPhysicalVolume.hh"
#include "WCSimAllSecondariesTree.hh"
#include "WCSimAllSecondaryPhotonsTree.hh"

#include <cmath>
#include <string>
#include <unordered_map>

G4bool WCSimSaveAllSecondaryTruthTrees();

int WCSimSteppingAction::primaryMuonTrackID = -1;
int WCSimSteppingAction::primaryMuonEventID = -1;

namespace {
struct AllSecondaryPhotonStartInfo {
  int evt = -1;
  int trk = -1;
  int parent = -1;
  int parent_pdg = 0;
  int parent_parent = -1;
  int parent_step = -1;
  int pdg = 0;

  float parent_ke_MeV = 0.f;
  float sx_cm = 0.f;
  float sy_cm = 0.f;
  float sz_cm = 0.f;
  float st_ns = 0.f;
  float dir_x = 0.f;
  float dir_y = 0.f;
  float dir_z = 0.f;
  float lambda_nm = 0.f;

  int hit_pmt = -1;
  float hit_x_cm = 0.f;
  float hit_y_cm = 0.f;
  float hit_z_cm = 0.f;
  float hit_t_ns = 0.f;
  std::string hit_volume;

  std::string creator;
  std::string parent_creator;
};

std::unordered_map<const G4Track*, AllSecondaryPhotonStartInfo>
    gAllSecondaryPhotonBirth;
int gAllSecondaryPhotonEvent = -1;

std::string VolumeName(const G4VPhysicalVolume* volume)
{
  return volume ? std::string(volume->GetName()) : std::string("NONE");
}

std::string MaterialName(const G4StepPoint* point)
{
  const auto* material = point ? point->GetMaterial() : nullptr;
  return material ? std::string(material->GetName()) : std::string("NONE");
}

int PMTIDFromStep(const G4Step* step)
{
  const auto* post = step ? step->GetPostStepPoint() : nullptr;
  const auto* touchable = post ? post->GetTouchable() : nullptr;
  if (!touchable) return -1;

  int localPMTCopy = -1;
  int moduleCopy = -1;
  const int depth = touchable->GetHistoryDepth();
  for (int level = 0; level <= depth; ++level) {
    const auto* volume = touchable->GetVolume(level);
    if (!volume) continue;

    const std::string name = volume->GetName();
    if (name == "pmt") {
      localPMTCopy = touchable->GetCopyNumber(level);
    }
    else if (name == "WCMultiPMT" ||
             name == "WCMultiPMT_AirGap" ||
             name == "WCMultiPMT_NoGap") {
      moduleCopy = touchable->GetCopyNumber(level);
    }
  }

  // WCTE truth-tree PMT IDs always use 100*moduleCopy + localPMTCopy,
  // with the local PMT copy number restricted to 0--18.
  if (localPMTCopy < 0 || localPMTCopy > 18 || moduleCopy < 0) return -1;
  return 100 * moduleCopy + localPMTCopy;
}

G4OpBoundaryProcess* FindBoundaryProcess(const G4Track* track)
{
  if (!track || !track->GetDefinition()) return nullptr;
  G4ProcessManager* manager = track->GetDefinition()->GetProcessManager();
  if (!manager) return nullptr;

  G4ProcessVector* processes = manager->GetProcessList();
  const G4int count = manager->GetProcessListLength();
  for (G4int i = 0; processes && i < count; ++i) {
    G4VProcess* process = (*processes)[i];
    if (process && process->GetProcessName() == "OpBoundary") {
      return static_cast<G4OpBoundaryProcess*>(process);
    }
  }
  return nullptr;
}

void FillAllSecondaries(const G4Step* step, const G4Event* event)
{
  const G4Track* track = step ? step->GetTrack() : nullptr;
  if (!track ||
      track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return;
  }

  const auto* definition = track->GetDefinition();
  const auto* pre = step->GetPreStepPoint();
  const auto* post = step->GetPostStepPoint();

  AllSecondariesTree_ResetVars();
  alls_evt = event ? event->GetEventID() : -1;
  alls_trk = track->GetTrackID();
  alls_parent = track->GetParentID();
  alls_pdg = definition ? definition->GetPDGEncoding() : 0;
  alls_step = track->GetCurrentStepNumber();

  if (pre) {
    const auto position = pre->GetPosition();
    alls_x_cm = position.x() / cm;
    alls_y_cm = position.y() / cm;
    alls_z_cm = position.z() / cm;
    alls_t_ns = pre->GetGlobalTime() / ns;

    const auto direction = pre->GetMomentumDirection();
    alls_dir_x = direction.x();
    alls_dir_y = direction.y();
    alls_dir_z = direction.z();

    alls_ke_MeV = pre->GetKineticEnergy() / MeV;
    alls_volume = VolumeName(pre->GetPhysicalVolume());
    alls_material = MaterialName(pre);
  }

  if (post) {
    const auto position = post->GetPosition();
    alls_post_x_cm = position.x() / cm;
    alls_post_y_cm = position.y() / cm;
    alls_post_z_cm = position.z() / cm;
    alls_post_t_ns = post->GetGlobalTime() / ns;

    const auto direction = post->GetMomentumDirection();
    alls_post_dir_x = direction.x();
    alls_post_dir_y = direction.y();
    alls_post_dir_z = direction.z();

    alls_post_ke_MeV = post->GetKineticEnergy() / MeV;
    alls_post_volume = VolumeName(post->GetPhysicalVolume());
    alls_post_material = MaterialName(post);

    const G4VProcess* process = post->GetProcessDefinedStep();
    alls_step_process = process
        ? std::string(process->GetProcessName())
        : std::string("NONE");
  }

  alls_edep_MeV = step->GetTotalEnergyDeposit() / MeV;
  alls_step_length_cm = step->GetStepLength() / cm;
  alls_track_length_cm = track->GetTrackLength() / cm;
  alls_charge = definition
      ? static_cast<float>(definition->GetPDGCharge())
      : 0.f;
  alls_particle = definition
      ? std::string(definition->GetParticleName())
      : std::string("NONE");

  const G4VProcess* creator = track->GetCreatorProcess();
  alls_creator = creator
      ? std::string(creator->GetProcessName())
      : std::string("NONE");

  AllSecondariesTree_Fill();
}

void CacheNewSecondaryPhotons(const G4Step* step, int eventID)
{
  const G4Track* parent = step ? step->GetTrack() : nullptr;
  if (!parent ||
      parent->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) {
    return;
  }

  const auto* secondaries = step->GetSecondaryInCurrentStep();
  if (!secondaries) return;

  const auto* parentDefinition = parent->GetDefinition();
  const auto* parentPre = step->GetPreStepPoint();
  const G4VProcess* parentCreator = parent->GetCreatorProcess();

  for (const G4Track* secondary : *secondaries) {
    if (!secondary ||
        secondary->GetDefinition() !=
            G4OpticalPhoton::OpticalPhotonDefinition()) {
      continue;
    }

    const G4VProcess* creatorProcess = secondary->GetCreatorProcess();
    const G4String creator = creatorProcess
        ? creatorProcess->GetProcessName()
        : "NONE";
    if (creator != "Cerenkov") continue;

    AllSecondaryPhotonStartInfo info;
    info.evt = eventID;
    info.trk = secondary->GetTrackID();
    info.parent = parent->GetTrackID();
    info.parent_pdg = parentDefinition
        ? parentDefinition->GetPDGEncoding()
        : 0;
    info.parent_parent = parent->GetParentID();
    info.parent_step = parent->GetCurrentStepNumber();
    info.pdg = secondary->GetDefinition()->GetPDGEncoding();
    info.parent_ke_MeV = parentPre
        ? parentPre->GetKineticEnergy() / MeV
        : parent->GetKineticEnergy() / MeV;

    const auto position = secondary->GetPosition();
    info.sx_cm = position.x() / cm;
    info.sy_cm = position.y() / cm;
    info.sz_cm = position.z() / cm;
    info.st_ns = secondary->GetGlobalTime() / ns;

    const auto direction = secondary->GetMomentumDirection();
    info.dir_x = direction.x();
    info.dir_y = direction.y();
    info.dir_z = direction.z();

    const G4double energy = secondary->GetTotalEnergy();
    info.lambda_nm = energy > 0.
        ? static_cast<float>((h_Planck * c_light / energy) / nm)
        : -1.f;
    info.creator = std::string(creator);
    info.parent_creator = parentCreator
        ? std::string(parentCreator->GetProcessName())
        : std::string("NONE");

    gAllSecondaryPhotonBirth[secondary] = info;
  }
}

void UpdateAndFinalizeSecondaryPhoton(const G4Step* step,
                                      const G4VPhysicalVolume* preVolume,
                                      const G4VPhysicalVolume* postVolume)
{
  const G4Track* track = step ? step->GetTrack() : nullptr;
  if (!track) return;

  auto found = gAllSecondaryPhotonBirth.find(track);
  if (found == gAllSecondaryPhotonBirth.end()) return;

  AllSecondaryPhotonStartInfo& info = found->second;
  const std::string preName = VolumeName(preVolume);
  const std::string postName = VolumeName(postVolume);

  if (info.hit_pmt < 0 && postName == "pmt" && preName != "pmt") {
    const int pmtID = PMTIDFromStep(step);
    if (pmtID >= 0) {
      info.hit_pmt = pmtID;
      info.hit_volume = postName;
      const auto* post = step->GetPostStepPoint();
      if (post) {
        const auto position = post->GetPosition();
        info.hit_x_cm = position.x() / cm;
        info.hit_y_cm = position.y() / cm;
        info.hit_z_cm = position.z() / cm;
        info.hit_t_ns = post->GetGlobalTime() / ns;
      }
    }
  }

  const G4TrackStatus status = track->GetTrackStatus();
  const bool killed = status == fStopAndKill ||
                      status == fKillTrackAndSecondaries;
  if (!killed) return;

  AllSecondaryPhotonsTree_ResetVars();
  asph_evt = info.evt;
  asph_trk = track->GetTrackID();
  asph_parent = info.parent;
  asph_parent_pdg = info.parent_pdg;
  asph_parent_parent = info.parent_parent;
  asph_parent_step = info.parent_step;
  asph_pdg = info.pdg;
  asph_parent_ke_MeV = info.parent_ke_MeV;

  asph_sx_cm = info.sx_cm;
  asph_sy_cm = info.sy_cm;
  asph_sz_cm = info.sz_cm;
  asph_st_ns = info.st_ns;
  asph_dir_x = info.dir_x;
  asph_dir_y = info.dir_y;
  asph_dir_z = info.dir_z;
  asph_lambda_nm = info.lambda_nm;

  asph_hit_pmt = info.hit_pmt;
  asph_hit_x_cm = info.hit_x_cm;
  asph_hit_y_cm = info.hit_y_cm;
  asph_hit_z_cm = info.hit_z_cm;
  asph_hit_t_ns = info.hit_t_ns;
  asph_hit_volume = info.hit_volume;

  int pePMT = -1;
  float peTime = 0.f;
  float peLambda = 0.f;
  if (AllSecondaryPhotonsTree_GetPE(
          info.evt, track->GetTrackID(), pePMT, peTime, peLambda)) {
    asph_made_pe = 1;
    // WCSD records the PMT at actual QE/collection-efficiency acceptance.
    // This can differ from the first PMT entered after reflections or re-entry.
    asph_pe_pmt = pePMT;
    asph_pe_time_ns = peTime;
    asph_pe_lambda_nm = peLambda;
  }

  const auto* post = step->GetPostStepPoint();
  if (post) {
    const auto position = post->GetPosition();
    asph_end_x_cm = position.x() / cm;
    asph_end_y_cm = position.y() / cm;
    asph_end_z_cm = position.z() / cm;
    asph_end_t_ns = post->GetGlobalTime() / ns;
    const G4VProcess* process = post->GetProcessDefinedStep();
    asph_end_process = process
        ? std::string(process->GetProcessName())
        : std::string("NONE");
  }

  asph_end_status = static_cast<int>(status);
  asph_end_boundary = postName;
  asph_creator = info.creator;
  asph_parent_creator = info.parent_creator;
  AllSecondaryPhotonsTree_Fill();
  gAllSecondaryPhotonBirth.erase(found);
}
}

G4int WCSimSteppingAction::n_photons_through_mPMTLV = 0;
G4int WCSimSteppingAction::n_photons_through_acrylic = 0;
G4int WCSimSteppingAction::n_photons_through_gel = 0;
G4int WCSimSteppingAction::n_photons_on_blacksheet = 0;
G4int WCSimSteppingAction::n_photons_on_smallPMT = 0;

WCSimSteppingAction::WCSimSteppingAction(WCSimRunAction* myRun,
                                         WCSimDetectorConstruction* myDet)
  : runAction(myRun), det(myDet)
{
}

void WCSimSteppingAction::UserSteppingAction(const G4Step* aStep)
{
  if (!aStep) return;

  const G4Event* event =
      G4EventManager::GetEventManager()->GetConstCurrentEvent();
  if (!event || event->IsAborted() || event->GetEventID() < 0) return;

  G4Track* track = const_cast<G4Track*>(aStep->GetTrack());
  if (!track || !track->GetDefinition()) return;

  const bool isOptical =
      track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition();

  // Retain the existing protection against pathological optical tracks.
  if (isOptical) {
    const G4double timeLimit = 20000. * ns;
    const G4int stepLimit = 5000;
    if (track->GetGlobalTime() > timeLimit ||
        track->GetCurrentStepNumber() > stepLimit) {
      track->SetTrackStatus(fStopAndKill);
    }

    if (track->GetKineticEnergy() < 0.) {
      const G4StepPoint* post = aStep->GetPostStepPoint();
      const G4VProcess* process = post ? post->GetProcessDefinedStep() : nullptr;
      static G4int negativeEnergyWarnings = 0;
      if (negativeEnergyWarnings++ < 20) {
        G4cout << "Negative optical-photon kinetic energy after step: "
               << track->GetKineticEnergy() / MeV << " MeV, process="
               << (process ? process->GetProcessName() : "NONE")
               << ", volume="
               << (track->GetVolume() ? track->GetVolume()->GetName() : "NULL")
               << G4endl;
      }
      track->SetTrackStatus(fStopAndKill);
      track->SetKineticEnergy(0.0);
    }
  }

  const G4bool saveTruth = WCSimSaveAllSecondaryTruthTrees();
  if (saveTruth) {
    const int eventID = event->GetEventID();
    if (eventID != gAllSecondaryPhotonEvent) {
      gAllSecondaryPhotonEvent = eventID;
      gAllSecondaryPhotonBirth.clear();
      AllSecondaryPhotonsTree_ResetEvent(eventID);
    }

    FillAllSecondaries(aStep, event);
    CacheNewSecondaryPhotons(aStep, eventID);
  }

  G4StepPoint* prePoint = aStep->GetPreStepPoint();
  G4StepPoint* postPoint = aStep->GetPostStepPoint();
  G4VPhysicalVolume* preVolume = prePoint ? prePoint->GetPhysicalVolume() : nullptr;
  G4VPhysicalVolume* postVolume = postPoint ? postPoint->GetPhysicalVolume() : nullptr;

  if (!isOptical) return;

  if (saveTruth) {
    UpdateAndFinalizeSecondaryPhoton(aStep, preVolume, postVolume);
  }

  if (!preVolume || !postVolume) return;

  const std::string preName = preVolume->GetName();
  const std::string postName = postVolume->GetName();

  if (preName.find("MultiPMT") != std::string::npos &&
      postName.find("vessel") != std::string::npos) {
    ++n_photons_through_mPMTLV;
  }
  if (postName.find("container") != std::string::npos &&
      preName.find("vessel") != std::string::npos) {
    ++n_photons_through_acrylic;
  }
  if (preName.find("container") != std::string::npos) {
    ++n_photons_through_gel;
  }
  if (preName.find("container") != std::string::npos &&
      postName.find("inner") != std::string::npos) {
    ++n_photons_on_blacksheet;
  }
  if (preName.find("container") != std::string::npos &&
      postName.find("pmt") != std::string::npos) {
    ++n_photons_on_smallPMT;
  }

  if ((track->GetTrackStatus() == fStopAndKill ||
       track->GetTrackStatus() == fKillTrackAndSecondaries)) {
    G4OpBoundaryProcess* boundary = FindBoundaryProcess(track);
    if (boundary && boundary->GetStatus() == NoRINDEX) {
      const G4Material* preMaterial = prePoint ? prePoint->GetMaterial() : nullptr;
      const G4Material* postMaterial = postPoint ? postPoint->GetMaterial() : nullptr;
      G4cout << "Optical photon killed because of a missing refractive index in "
             << (preMaterial ? preMaterial->GetName() : "NONE") << " or "
             << (postMaterial ? postMaterial->GetName() : "NONE")
             << " (transition from " << preName << " to " << postName << ")"
             << G4endl;
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

