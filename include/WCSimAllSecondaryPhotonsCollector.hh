#ifndef WCSIM_ALL_SECONDARY_PHOTONS_COLLECTOR_HH
#define WCSIM_ALL_SECONDARY_PHOTONS_COLLECTOR_HH

#include "TFile.h"
#include "TTree.h"

#include <cstdint>
#include <string>
#include <unordered_map>

#include "G4ThreeVector.hh"

class G4Event;
class G4Step;

class WCSimAllSecondaryPhotonsCollector
{
public:
  void Book(TFile* file);
  void RecordStep(const G4Step* step, const G4Event* event);
  void NotePE(int event, int track, int pmt, float timeNs, float wavelengthNm);
  void Write();

private:
  struct PhotonInfo
  {
    int event = -1;
    int track = -1;
    int parent = -1;
    int parentPdg = 0;
    int parentParent = -1;
    int parentStep = -1;
    int pdg = 0;
    float parentKineticEnergyMeV = 0.f;
    G4ThreeVector position;
    G4ThreeVector direction;
    float timeNs = 0.f;
    float wavelengthNm = 0.f;
    std::string creator;
    std::string parentCreator;
  };

  struct PEInfo
  {
    int pmt = -1;
    float timeNs = 0.f;
    float wavelengthNm = 0.f;
  };

  TTree* tree = nullptr;
  int eventId = -1;
  int trackId = -1;
  int parentId = -1;
  int parentPdg = 0;
  int parentParentId = -1;
  int parentStep = -1;
  int pdg = 0;
  float parentKineticEnergyMeV = 0.f;
  float startXCm = 0.f;
  float startYCm = 0.f;
  float startZCm = 0.f;
  float startTimeNs = 0.f;
  float dirX = 0.f;
  float dirY = 0.f;
  float dirZ = 0.f;
  float wavelengthNm = 0.f;
  int hitPmt = -1;
  float hitXCm = 0.f;
  float hitYCm = 0.f;
  float hitZCm = 0.f;
  float hitTimeNs = 0.f;
  int madePE = 0;
  int pePmt = -1;
  float peTimeNs = 0.f;
  float peWavelengthNm = 0.f;
  float endXCm = 0.f;
  float endYCm = 0.f;
  float endZCm = 0.f;
  float endTimeNs = 0.f;
  int endStatus = -1;
  std::string creator;
  std::string parentCreator;
  std::string hitVolume;
  std::string endBoundary;
  std::string endProcess;

  int currentEvent = -1;
  std::unordered_map<std::uint64_t, PhotonInfo> photons;
  std::unordered_map<std::uint64_t, PEInfo> peCache;

  static std::uint64_t Key(int event, int track);
  void ResetRow();
  void ResetEvent(int event);
  void CacheCreatedPhoton(const G4Step* step, const G4Event* event);
  void FinishPhoton(const G4Step* step, const G4Event* event);
};

#endif