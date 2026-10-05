#include "WCSimRunActionMessenger.hh"

#include "WCSimRunAction.hh"
#include "G4UIdirectory.hh"
#include "G4UIcommand.hh"
#include "G4UIparameter.hh"
#include "G4UIcmdWithAString.hh"
#include "G4UIcmdWithABool.hh"

WCSimRunActionMessenger::WCSimRunActionMessenger(WCSimRunAction* WCSimRA)
:WCSimRun(WCSimRA)
{ 
  WCSimIODir = new G4UIdirectory("/WCSimIO/");
  WCSimIODir->SetGuidance("Commands to select I/O options");

  RootFile = new G4UIcmdWithAString("/WCSimIO/RootFile",this);
  RootFile->SetGuidance("Set the root file name");
  RootFile->SetGuidance("Enter the name of the output ROOT file");
  RootFile->SetParameterName("RootFileName",true);
  RootFile->SetDefaultValue("wcsim.root");

  WriteDefaultRootFile = new G4UIcmdWithABool("/WCSimIO/WriteDefaultRootFile",this);
  WriteDefaultRootFile->SetGuidance("Do you want to write out the standard ROOT file format.");
  WriteDefaultRootFile->SetParameterName("WriteDefaultFile",true);
  WriteDefaultRootFile->SetDefaultValue(true);  //ToDo: memo: default = FALSE !! Move to novis.mac!

  WriteFlatRootFile = new G4UIcmdWithABool("/WCSimIO/WriteFlatRootFile",this);
  WriteFlatRootFile->SetGuidance("Do you want to write out the flat ROOT file format");
  WriteFlatRootFile->SetParameterName("WriteFlatFile",true);
  WriteFlatRootFile->SetDefaultValue(false);

  RooTracker = new G4UIcmdWithABool("/WCSimIO/SaveRooTracker",this);
  RooTracker->SetGuidance("Save the input NEUT Rootracker objects to the output file");
  RooTracker->SetGuidance("Enter a boolean to save or drop the NEUT RooTracker information");
  RooTracker->SetParameterName("SaveRooTracker",false);
  RooTracker->SetDefaultValue(false);

  UseTimer = new G4UIcmdWithABool("/WCSimIO/Timer",this);
  UseTimer->SetGuidance("Use a timer for runtime");
  UseTimer->SetParameterName("UseTimer",true);
  UseTimer->SetDefaultValue(false);

    SavePrimaryParticleTruth = new G4UIcmdWithABool(
      "/WCSimIO/SavePrimaryParticleTruth", this);
    SavePrimaryParticleTruth->SetGuidance(
      "Save step-by-step truth for primary non-optical particles.");
    SavePrimaryParticleTruth->SetParameterName("SavePrimaryParticleTruth", false);
    SavePrimaryParticleTruth->SetDefaultValue(true);
    SavePrimaryParticleTruth->AvailableForStates(G4State_PreInit, G4State_Idle);

    SaveAllParticleTruth = new G4UIcmdWithABool(
      "/WCSimIO/SaveAllParticleTruth", this);
    SaveAllParticleTruth->SetGuidance(
      "Save step-by-step truth for all non-optical particles and Cerenkov photons.");
    SaveAllParticleTruth->SetParameterName("SaveAllParticleTruth", false);
    SaveAllParticleTruth->SetDefaultValue(true);
    SaveAllParticleTruth->AvailableForStates(G4State_PreInit, G4State_Idle);

}

WCSimRunActionMessenger::~WCSimRunActionMessenger()
{
  delete WriteDefaultRootFile;
  delete WriteFlatRootFile;
  delete RootFile;
  delete RooTracker;
  delete UseTimer;
  delete SavePrimaryParticleTruth;
  delete SaveAllParticleTruth;
  delete WCSimIODir;
}

void WCSimRunActionMessenger::SetNewValue(G4UIcommand* command,G4String newValue)
{

  if (command == SavePrimaryParticleTruth) {
    const G4bool save = SavePrimaryParticleTruth->GetNewBoolValue(newValue);
    WCSimRun->SetSavePrimaryParticleTruth(save);
    G4cout << "Primary particle truth " << (save ? "ENABLED" : "DISABLED") << G4endl;
    return;
  }

  if (command == SaveAllParticleTruth) {
    const G4bool save = SaveAllParticleTruth->GetNewBoolValue(newValue);
    WCSimRun->SetSaveAllParticleTruth(save);
    G4cout << "All-particle truth " << (save ? "ENABLED" : "DISABLED") << G4endl;
    return;
  }

  if ( command == RootFile)
    {
      WCSimRun->SetRootFileName(newValue);
      G4cout << "Output ROOT file set to " << newValue << G4endl;
    }

  else if (command == WriteDefaultRootFile )
    {
      WCSimRun->SetOptionalRootFile(WriteDefaultRootFile->GetNewBoolValue(newValue));
      G4cout << "You chose to write out the standard ROOT file: " << WriteDefaultRootFile->GetNewBoolValue(newValue) << G4endl;
    }

  else if (command == WriteFlatRootFile )
    {
      WCSimRun->SetFlatRootFile(WriteFlatRootFile->GetNewBoolValue(newValue));
      G4cout << "You chose to write out the flat ROOT file: " << WriteFlatRootFile->GetNewBoolValue(newValue) << G4endl;
    }

  if ( command == RooTracker)
    {
      WCSimRun->SetSaveRooTracker(RooTracker->GetNewBoolValue(newValue));
      if(newValue) G4cout << "Saving NEUT RooTracker information to output file"  << G4endl;
    }
  else if(command == UseTimer)
    {
      bool use = UseTimer->GetNewBoolValue(newValue);
      WCSimRun->SetUseTimer(use);
      G4cout << "WCSimRunAction timer " << (use ? "ENABLED" : "DISABLED") << G4endl;
    }
}

