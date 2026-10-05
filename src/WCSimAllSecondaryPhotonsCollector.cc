#include "WCSimAllSecondaryPhotonsCollector.hh"

#include "G4Event.hh"
#include "G4OpticalPhoton.hh"
#include "G4ParticleDefinition.hh"
#include "G4PhysicalConstants.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "G4SystemOfUnits.hh"
#include "G4Track.hh"
#include "G4VProcess.hh"
#include "TObject.h"

void WCSimAllSecondaryPhotonsCollector::Book(TFile* file)
{
  if (tree || !file) return;
  file->cd();
  tree = new TTree(
      "AllSecondaryPhotons",
      "Cherenkov photons produced by primary and secondary particles");
  tree->Branch("evt", &eventId, "evt/I");
  tree->Branch("trk", &trackId, "trk/I");
  tree->Branch("parent", &parentId, "parent/I");
  tree->Branch("parent_pdg", &parentPdg, "parent_pdg/I");
  tree->Branch("parent_parent", &parentParentId, "parent_parent/I");
  tree->Branch("parent_step", &parentStep, "parent_step/I");
  tree->Branch("pdg", &pdg, "pdg/I");
  tree->Branch("parent_ke_MeV", &parentKineticEnergyMeV, "parent_ke_MeV/F");
  tree->Branch("sx_cm", &startXCm, "sx_cm/F");
  tree->Branch("sy_cm", &startYCm, "sy_cm/F");
  tree->Branch("sz_cm", &startZCm, "sz_cm/F");
  tree->Branch("st_ns", &startTimeNs, "st_ns/F");
  tree->Branch("dir_x", &dirX, "dir_x/F");
  tree->Branch("dir_y", &dirY, "dir_y/F");
  tree->Branch("dir_z", &dirZ, "dir_z/F");
  tree->Branch("lambda_nm", &wavelengthNm, "lambda_nm/F");
  tree->Branch("hit_pmt", &hitPmt, "hit_pmt/I");
  tree->Branch("hit_x_cm", &hitXCm, "hit_x_cm/F");
  tree->Branch("hit_y_cm", &hitYCm, "hit_y_cm/F");
  tree->Branch("hit_z_cm", &hitZCm, "hit_z_cm/F");
  tree->Branch("hit_t_ns", &hitTimeNs, "hit_t_ns/F");
  tree->Branch("made_pe", &madePE, "made_pe/I");
  tree->Branch("pe_pmt", &pePmt, "pe_pmt/I");
  tree->Branch("pe_time_ns", &peTimeNs, "pe_time_ns/F");
  tree->Branch("pe_lambda_nm", &peWavelengthNm, "pe_lambda_nm/F");
  tree->Branch("end_x_cm", &endXCm, "end_x_cm/F");
  tree->Branch("end_y_cm", &endYCm, "end_y_cm/F");
  tree->Branch("end_z_cm", &endZCm, "end_z_cm/F");
  tree->Branch("end_t_ns", &endTimeNs, "end_t_ns/F");
  tree->Branch("end_status", &endStatus, "end_status/I");
  tree->Branch("creator", &creator);
  tree->Branch("parent_creator", &parentCreator);
  tree->Branch("hit_volume", &hitVolume);
  tree->Branch("end_boundary", &endBoundary);
  tree->Branch("end_process", &endProcess);
}

void WCSimAllSecondaryPhotonsCollector::RecordStep(
    const G4Step* step, const G4Event* event)
{
  if (!tree || !step || !event) return;
  ResetEvent(event->GetEventID());
  CacheCreatedPhoton(step, event);
  FinishPhoton(step, event);
}

void WCSimAllSecondaryPhotonsCollector::NotePE(
  int event, int track, int pmt, float timeNs, float noteWavelengthNm)
{
  if (!tree) return;
  ResetEvent(event);
  peCache[Key(event, track)] = {pmt, timeNs, noteWavelengthNm};
}

void WCSimAllSecondaryPhotonsCollector::Write()
{
  if (!tree) return;
  TFile* file = tree->GetCurrentFile();
  if (!file) return;
  file->cd();
  tree->Write("", TObject::kOverwrite);
}

std::uint64_t WCSimAllSecondaryPhotonsCollector::Key(int event, int track)
{
  return (static_cast<std::uint64_t>(static_cast<std::uint32_t>(event)) << 32) |
         static_cast<std::uint32_t>(track);
}

void WCSimAllSecondaryPhotonsCollector::ResetRow()
{
  eventId = trackId = parentId = parentParentId = -1;
  parentPdg = parentStep = pdg = 0;
  parentKineticEnergyMeV = 0.f;
  startXCm = startYCm = startZCm = startTimeNs = 0.f;
  dirX = dirY = dirZ = wavelengthNm = 0.f;
  hitPmt = -1;
  hitXCm = hitYCm = hitZCm = hitTimeNs = 0.f;
  madePE = 0;
  pePmt = -1;
  peTimeNs = peWavelengthNm = 0.f;
  endXCm = endYCm = endZCm = endTimeNs = 0.f;
  endStatus = -1;
  creator.clear();
  parentCreator.clear();
  hitVolume.clear();
  endBoundary.clear();
  endProcess.clear();
}

void WCSimAllSecondaryPhotonsCollector::ResetEvent(int event)
{
  if (event == currentEvent) return;
  currentEvent = event;
  photons.clear();
  peCache.clear();
}

void WCSimAllSecondaryPhotonsCollector::CacheCreatedPhoton(
    const G4Step* step, const G4Event* event)
{
  const G4Track* track = step->GetTrack();
  if (!track ||
      track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) return;
  const G4VProcess* process = track->GetCreatorProcess();
  if (!process || process->GetProcessName() != "Cerenkov" ||
      track->GetCurrentStepNumber() != 1) return;

  PhotonInfo info;
  info.event = event->GetEventID();
  info.track = track->GetTrackID();
  info.parent = track->GetParentID();
  info.parentKineticEnergyMeV = track->GetVertexKineticEnergy() / MeV;
  info.position = track->GetVertexPosition();
  info.direction = track->GetVertexMomentumDirection();
  info.timeNs = track->GetGlobalTime() / ns;
  info.wavelengthNm = track->GetVertexKineticEnergy() > 0.
      ? static_cast<float>((h_Planck * c_light / track->GetVertexKineticEnergy()) / nm)
      : 0.f;
  info.creator = process->GetProcessName();
  photons[Key(info.event, info.track)] = info;
}

void WCSimAllSecondaryPhotonsCollector::FinishPhoton(
    const G4Step* step, const G4Event* event)
{
  const G4Track* track = step->GetTrack();
  if (!track ||
      track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition() ||
      track->GetTrackStatus() == fAlive) return;

  const std::uint64_t key = Key(event->GetEventID(), track->GetTrackID());
  auto found = photons.find(key);
  if (found == photons.end()) return;

  const PhotonInfo& info = found->second;
  const G4StepPoint* post = step->GetPostStepPoint();
  ResetRow();
  eventId = info.event;
  trackId = info.track;
  parentId = info.parent;
  parentPdg = info.parentPdg;
  parentParentId = info.parentParent;
  parentStep = info.parentStep;
  pdg = track->GetDefinition()->GetPDGEncoding();
  parentKineticEnergyMeV = info.parentKineticEnergyMeV;
  startXCm = info.position.x() / cm;
  startYCm = info.position.y() / cm;
  startZCm = info.position.z() / cm;
  startTimeNs = info.timeNs;
  dirX = info.direction.x();
  dirY = info.direction.y();
  dirZ = info.direction.z();
  wavelengthNm = info.wavelengthNm;
  endXCm = track->GetPosition().x() / cm;
  endYCm = track->GetPosition().y() / cm;
  endZCm = track->GetPosition().z() / cm;
  endTimeNs = track->GetGlobalTime() / ns;
  endStatus = track->GetTrackStatus();
  creator = info.creator;
  endProcess = post && post->GetProcessDefinedStep()
      ? post->GetProcessDefinedStep()->GetProcessName() : "NONE";

  auto pe = peCache.find(key);
  if (pe != peCache.end()) {
    madePE = 1;
    pePmt = pe->second.pmt;
    peTimeNs = pe->second.timeNs;
    peWavelengthNm = pe->second.wavelengthNm;
    peCache.erase(pe);
  }
  tree->Fill();
  photons.erase(found);
}