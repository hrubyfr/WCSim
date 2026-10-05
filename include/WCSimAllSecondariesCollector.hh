#ifndef WCSIM_ALL_SECONDARIES_COLLECTOR_HH
#define WCSIM_ALL_SECONDARIES_COLLECTOR_HH

#include "TFile.h"
#include "TTree.h"

#include <string>

class G4Event;
class G4Step;

class WCSimAllSecondariesCollector
{
public:
  void Book(TFile* file);
  void RecordStep(const G4Step* step, const G4Event* event,
                  bool savePrimary, bool saveAllParticles);
  void Write();

private:
  TTree* tree = nullptr;
  int eventId = -1;
  int trackId = -1;
  int parentId = -1;
  int pdg = 0;
  int stepNumber = -1;
  float xCm = 0.f;
  float yCm = 0.f;
  float zCm = 0.f;
  float timeNs = 0.f;
  float postXCm = 0.f;
  float postYCm = 0.f;
  float postZCm = 0.f;
  float postTimeNs = 0.f;
  float dirX = 0.f;
  float dirY = 0.f;
  float dirZ = 0.f;
  float postDirX = 0.f;
  float postDirY = 0.f;
  float postDirZ = 0.f;
  float kineticEnergyMeV = 0.f;
  float postKineticEnergyMeV = 0.f;
  float energyDepositMeV = 0.f;
  float stepLengthCm = 0.f;
  float trackLengthCm = 0.f;
  float charge = 0.f;
  std::string particle;
  std::string creator;
  std::string stepProcess;
  std::string volume;
  std::string postVolume;
  std::string material;
  std::string postMaterial;

  void ResetRow();
};

#endif