#include "WCSimAllSecondariesCollector.hh"

#include "G4Event.hh"
#include "G4Material.hh"
#include "G4OpticalPhoton.hh"
#include "G4ParticleDefinition.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VProcess.hh"
#include "G4ThreeVector.hh"
#include "TObject.h"

namespace {
std::string VolumeName(const G4VPhysicalVolume* volume)
{
  return volume ? std::string(volume->GetName()) : std::string("NONE");
}

std::string MaterialName(const G4StepPoint* point)
{
  const G4Material* material = point ? point->GetMaterial() : nullptr;
  return material ? std::string(material->GetName()) : std::string("NONE");
}
}

void WCSimAllSecondariesCollector::Book(TFile* file)
{
  if (tree || !file) return;
  file->cd();
  tree = new TTree(
      "AllSecondaries",
      "Step-by-step truth for primary and non-optical secondary particles");
  tree->Branch("evt", &eventId, "evt/I");
  tree->Branch("trk", &trackId, "trk/I");
  tree->Branch("parent", &parentId, "parent/I");
  tree->Branch("pdg", &pdg, "pdg/I");
  tree->Branch("step", &stepNumber, "step/I");
  tree->Branch("x_cm", &xCm, "x_cm/F");
  tree->Branch("y_cm", &yCm, "y_cm/F");
  tree->Branch("z_cm", &zCm, "z_cm/F");
  tree->Branch("t_ns", &timeNs, "t_ns/F");
  tree->Branch("post_x_cm", &postXCm, "post_x_cm/F");
  tree->Branch("post_y_cm", &postYCm, "post_y_cm/F");
  tree->Branch("post_z_cm", &postZCm, "post_z_cm/F");
  tree->Branch("post_t_ns", &postTimeNs, "post_t_ns/F");
  tree->Branch("dir_x", &dirX, "dir_x/F");
  tree->Branch("dir_y", &dirY, "dir_y/F");
  tree->Branch("dir_z", &dirZ, "dir_z/F");
  tree->Branch("post_dir_x", &postDirX, "post_dir_x/F");
  tree->Branch("post_dir_y", &postDirY, "post_dir_y/F");
  tree->Branch("post_dir_z", &postDirZ, "post_dir_z/F");
  tree->Branch("ke_MeV", &kineticEnergyMeV, "ke_MeV/F");
  tree->Branch("post_ke_MeV", &postKineticEnergyMeV, "post_ke_MeV/F");
  tree->Branch("edep_MeV", &energyDepositMeV, "edep_MeV/F");
  tree->Branch("step_length_cm", &stepLengthCm, "step_length_cm/F");
  tree->Branch("track_length_cm", &trackLengthCm, "track_length_cm/F");
  tree->Branch("charge", &charge, "charge/F");
  tree->Branch("particle", &particle);
  tree->Branch("creator", &creator);
  tree->Branch("step_process", &stepProcess);
  tree->Branch("volume", &volume);
  tree->Branch("post_volume", &postVolume);
  tree->Branch("material", &material);
  tree->Branch("post_material", &postMaterial);
}

void WCSimAllSecondariesCollector::RecordStep(
    const G4Step* step, const G4Event* event,
    bool savePrimary, bool saveAllParticles)
{
  if (!tree || !step) return;
  const G4Track* track = step->GetTrack();
  if (!track || track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition()) return;

  const bool isPrimary = track->GetParentID() == 0;
  if (!saveAllParticles && !(savePrimary && isPrimary)) return;

  const G4StepPoint* pre = step->GetPreStepPoint();
  const G4StepPoint* post = step->GetPostStepPoint();
  const G4ParticleDefinition* definition = track->GetDefinition();
  ResetRow();
  eventId = event ? event->GetEventID() : -1;
  trackId = track->GetTrackID();
  parentId = track->GetParentID();
  pdg = definition ? definition->GetPDGEncoding() : 0;
  stepNumber = track->GetCurrentStepNumber();
  if (pre) {
    const G4ThreeVector position = pre->GetPosition();
    const G4ThreeVector direction = pre->GetMomentumDirection();
    xCm = position.x() / cm;
    yCm = position.y() / cm;
    zCm = position.z() / cm;
    timeNs = pre->GetGlobalTime() / ns;
    dirX = direction.x();
    dirY = direction.y();
    dirZ = direction.z();
    kineticEnergyMeV = pre->GetKineticEnergy() / MeV;
    volume = VolumeName(pre->GetPhysicalVolume());
    material = MaterialName(pre);
  }
  if (post) {
    const G4ThreeVector position = post->GetPosition();
    const G4ThreeVector direction = post->GetMomentumDirection();
    postXCm = position.x() / cm;
    postYCm = position.y() / cm;
    postZCm = position.z() / cm;
    postTimeNs = post->GetGlobalTime() / ns;
    postDirX = direction.x();
    postDirY = direction.y();
    postDirZ = direction.z();
    postKineticEnergyMeV = post->GetKineticEnergy() / MeV;
    postVolume = VolumeName(post->GetPhysicalVolume());
    postMaterial = MaterialName(post);
    const G4VProcess* process = post->GetProcessDefinedStep();
    stepProcess = process ? process->GetProcessName() : "NONE";
  }
  energyDepositMeV = step->GetTotalEnergyDeposit() / MeV;
  stepLengthCm = step->GetStepLength() / cm;
  trackLengthCm = track->GetTrackLength() / cm;
  charge = definition ? definition->GetPDGCharge() : 0.;
  particle = definition ? definition->GetParticleName() : "NONE";
  const G4VProcess* creatorProcess = track->GetCreatorProcess();
  creator = creatorProcess ? creatorProcess->GetProcessName() : "NONE";
  tree->Fill();
}

void WCSimAllSecondariesCollector::Write()
{
  if (!tree) return;
  TFile* file = tree->GetCurrentFile();
  if (!file) return;
  file->cd();
  tree->Write("", TObject::kOverwrite);
}

void WCSimAllSecondariesCollector::ResetRow()
{
  eventId = trackId = parentId = -1;
  pdg = 0;
  stepNumber = -1;
  xCm = yCm = zCm = timeNs = 0.f;
  postXCm = postYCm = postZCm = postTimeNs = 0.f;
  dirX = dirY = dirZ = 0.f;
  postDirX = postDirY = postDirZ = 0.f;
  kineticEnergyMeV = postKineticEnergyMeV = energyDepositMeV = 0.f;
  stepLengthCm = trackLengthCm = charge = 0.f;
  particle.clear();
  creator.clear();
  stepProcess.clear();
  volume.clear();
  postVolume.clear();
  material.clear();
  postMaterial.clear();
}