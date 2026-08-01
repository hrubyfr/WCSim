#include "WCSimTrackingAction.hh"
#include "WCSimTrajectory.hh"
#include "G4ParticleTypes.hh"
#include "G4TrackingManager.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "Randomize.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"
#include "G4RunManager.hh"
#include "WCSimTrackInformation.hh"
#include "WCSimTrackingMessenger.hh"
#include "WCSimPrimaryGeneratorAction.hh"
#include "G4SystemOfUnits.hh"

WCSimTrackingAction::WCSimTrackingAction()
{
  ProcessList.insert("Decay");
  ProcessList.insert("conv");
  //ProcessList.insert("muMinusCaptureAtRest");
  ProcessList.insert("nCapture");
  ProcessList.insert("OpWLS");

  ParticleList.insert(111);   // pi0
  ParticleList.insert(211);   // pion+
  ParticleList.insert(-211);  // pion-
  ParticleList.insert(321);   // kaon+
  ParticleList.insert(-321);  // kaon-
  ParticleList.insert(311);   // kaon0
  ParticleList.insert(-311);  // kaon0 bar
  ParticleList.insert(11);    // e-
  ParticleList.insert(-11);   // e+
  ParticleList.insert(13);    // mu-
  ParticleList.insert(-13);   // mu+
  ParticleList.insert(2212);  // proton
  ParticleList.insert(2112);  // neutron

  percentageOfCherenkovPhotonsToDraw = 0.0;
#ifndef WCSIM_SAVE_PHOTON_HISTORY
  SAVE_PHOTON_HISTORY = false;
#else
  SAVE_PHOTON_HISTORY = true;
#endif

  messenger = new WCSimTrackingMessenger(this);

  // Maximum time for radioactive decay.
  fMaxTime = 1. * CLHEP::second;
  fTime_birth = 0.;
}

WCSimTrackingAction::~WCSimTrackingAction()
{
}

void WCSimTrackingAction::PreUserTrackingAction(const G4Track* aTrack)
{
  if (!aTrack) return;

  // Store all non-optical trajectories. Optical trajectories are stored only
  // according to the configured sampling/history options.
  if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition() ||
      G4UniformRand() < percentageOfCherenkovPhotonsToDraw / 100.) {
    WCSimTrajectory* thisTrajectory = new WCSimTrajectory(aTrack);
    thisTrajectory->SetSavePhotonTrack(true);
    fpTrackingManager->SetTrajectory(thisTrajectory);
    fpTrackingManager->SetStoreTrajectory(true);
  }
  else if (SAVE_PHOTON_HISTORY) {
    WCSimTrajectory* thisTrajectory = new WCSimTrajectory(aTrack);
    thisTrajectory->SetSavePhotonTrack(false);
    fpTrackingManager->SetTrajectory(thisTrajectory);
    fpTrackingManager->SetStoreTrajectory(true);
  }
  else {
    fpTrackingManager->SetStoreTrajectory(false);
  }

  WCSimPrimaryGeneratorAction* primaryGenerator =
      const_cast<WCSimPrimaryGeneratorAction*>(
          dynamic_cast<const WCSimPrimaryGeneratorAction*>(
              G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction()));

  if (primaryGenerator && !primaryGenerator->IsConversionFound()) {
    if (aTrack->GetParentID() == 0) {
      primaryID = aTrack->GetTrackID();
    }
    else if (aTrack->GetParentID() == primaryID) {
      const G4VProcess* creator = aTrack->GetCreatorProcess();
      if (creator && creator->GetProcessName() == "conv") {
        primaryGenerator->FoundConversion();
      }

      // Intentionally retain the existing WCTE behavior: do not abort the
      // event after the primary conversion has been identified.
      //G4EventManager::GetEventManager()->AbortCurrentEvent();
      //G4EventManager::GetEventManager()->GetNonconstCurrentEvent()->SetEventAborted();
    }
  }

  // Kill a nucleus generated as the first track only after all of its
  // secondaries have been processed.
  G4ParticleDefinition* particle = aTrack->GetDefinition();
  const G4double charge = particle->GetPDGCharge();
  G4Track* track = const_cast<G4Track*>(aTrack);

  if (aTrack->GetTrackID() == 1) {
    fTime_birth = 0.;
    if (charge > 2.) track->SetTrackStatus(fStopButAlive);
  }

  if (aTrack->GetTrackID() == 2) {
    fTime_birth = aTrack->GetGlobalTime();
  }
}

void WCSimTrackingAction::PostUserTrackingAction(const G4Track* aTrack)
{
  if (!aTrack) return;

  const G4VProcess* creatorProcess = aTrack->GetCreatorProcess();

  WCSimTrackInformation* anInfo = nullptr;
  if (aTrack->GetUserInformation()) {
    anInfo = static_cast<WCSimTrackInformation*>(aTrack->GetUserInformation());
  }
  else {
    anInfo = new WCSimTrackInformation();
  }

  // Save primaries and tracks selected by particle type or creator process.
  if (aTrack->GetParentID() == 0 ||
      (creatorProcess && ProcessList.count(creatorProcess->GetProcessName())) ||
      ParticleList.count(aTrack->GetDefinition()->GetPDGEncoding())) {
    anInfo->WillBeSaved(true);
  }
  else {
    anInfo->WillBeSaved(false);
  }

  G4Track* theTrack = const_cast<G4Track*>(aTrack);
  theTrack->SetUserInformation(anInfo);

  // Pass the parent trajectory to all children.
  G4TrackVector* secondaries = fpTrackingManager->GimmeSecondaries();
  WCSimTrajectory* currentTrajectory =
      static_cast<WCSimTrajectory*>(fpTrackingManager->GimmeTrajectory());

  if (currentTrajectory && !anInfo->GetMyTrajectory()) {
    anInfo->SetMyTrajectory(currentTrajectory);
  }

  if (secondaries) {
    for (G4Track* secondary : *secondaries) {
      if (!secondary) continue;
      WCSimTrackInformation* infoSec = new WCSimTrackInformation();
      infoSec->SetParentTrajectory(anInfo->GetMyTrajectory());
      secondary->SetUserInformation(infoSec);
    }
  }

  // If this track produced a hit, flag the saved parent trajectory chain.
  if (anInfo->GetProducesHit() && saveHitProducingTracks) {
    WCSimTrajectory* parentTrajectory = anInfo->GetParentTrajectory();
    while (parentTrajectory && !parentTrajectory->GetProducesHit()) {
      if (parentTrajectory->GetPDGEncoding() == 0 &&
          !parentTrajectory->GetSavePhotonTrack()) {
        break;
      }
      parentTrajectory->SetProducesHit(true);
      parentTrajectory = parentTrajectory->GetParentTrajectory();
    }
  }

  if (currentTrajectory) {
    G4ThreeVector currentPosition = aTrack->GetPosition();
    currentTrajectory->SetStoppingPoint(currentPosition);
    currentTrajectory->SetStoppingVolume(aTrack->GetVolume());

    if (aTrack->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition()) {
      currentTrajectory->SetSaveFlag(anInfo->isSaved());
      currentTrajectory->SetProducesHit(anInfo->GetProducesHit());
    }
    else if (currentTrajectory->GetSavePhotonTrack()) {
      currentTrajectory->SetSaveFlag(anInfo->isSaved());
    }
    else {
      currentTrajectory->SetSaveFlag(false);
    }
  }

  // Record gamma-conversion daughters for the primary generator when they are
  // actually available. The original code assumed two secondaries and could
  // dereference null step/process pointers.
  WCSimPrimaryGeneratorAction* primaryGenerator =
      const_cast<WCSimPrimaryGeneratorAction*>(
          dynamic_cast<const WCSimPrimaryGeneratorAction*>(
              G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction()));
  const G4Step* finalStep = aTrack->GetStep();
  const G4StepPoint* postPoint = finalStep ? finalStep->GetPostStepPoint() : nullptr;
  const G4VProcess* endProcess = postPoint ? postPoint->GetProcessDefinedStep() : nullptr;

  if (primaryGenerator && !primaryGenerator->IsConversionFound() &&
      aTrack->GetTrackID() == primaryID &&
      endProcess && endProcess->GetProcessName() == "conv" &&
      secondaries && secondaries->size() >= 2) {
    for (int i = 0; i < 2; ++i) {
      G4Track* secondary = secondaries->at(i);
      if (!secondary) continue;
      primaryGenerator->SetConversionProductParticle(
          i, secondary->GetParticleDefinition());
      primaryGenerator->SetConversionProductMomentum(i, secondary->GetMomentum());
    }
  }
}
