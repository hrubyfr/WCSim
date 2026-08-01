#include "WCSimSecondaryElectronTree.hh"

#include "TFile.h"
#include "TTree.h"

TTree* secondary_electron_tree = nullptr;

int   se_evt    = -1;
int   se_trk    = -1;
int   se_parent = -1;
int   se_pdg    = 11;

float se_x0_cm = 0.f;
float se_y0_cm = 0.f;
float se_z0_cm = 0.f;
float se_t0_ns = 0.f;

float se_ke0_MeV = 0.f;

float se_p0_MeVc  = 0.f;
float se_px0_MeVc = 0.f;
float se_py0_MeVc = 0.f;
float se_pz0_MeVc = 0.f;

float se_x1_cm = 0.f;
float se_y1_cm = 0.f;
float se_z1_cm = 0.f;
float se_t1_ns = 0.f;
float se_ke1_MeV = 0.f;

std::string se_creator     = "";
std::string se_end_process = "";
std::string se_end_volume  = "";

void SecondaryElectronTree_Book(TFile* fout)
{
  if (secondary_electron_tree) return;
  if (!fout) return;

  fout->cd();

  secondary_electron_tree =
      new TTree("secondary_electron", "All secondary electron truth / kinematics");

  secondary_electron_tree->Branch("evt",    &se_evt,    "evt/I");
  secondary_electron_tree->Branch("trk",    &se_trk,    "trk/I");
  secondary_electron_tree->Branch("parent", &se_parent, "parent/I");
  secondary_electron_tree->Branch("pdg",    &se_pdg,    "pdg/I");

  secondary_electron_tree->Branch("x0_cm", &se_x0_cm, "x0_cm/F");
  secondary_electron_tree->Branch("y0_cm", &se_y0_cm, "y0_cm/F");
  secondary_electron_tree->Branch("z0_cm", &se_z0_cm, "z0_cm/F");
  secondary_electron_tree->Branch("t0_ns", &se_t0_ns, "t0_ns/F");

  secondary_electron_tree->Branch("ke0_MeV", &se_ke0_MeV, "ke0_MeV/F");

  secondary_electron_tree->Branch("p0_MeVc",  &se_p0_MeVc,  "p0_MeVc/F");
  secondary_electron_tree->Branch("px0_MeVc", &se_px0_MeVc, "px0_MeVc/F");
  secondary_electron_tree->Branch("py0_MeVc", &se_py0_MeVc, "py0_MeVc/F");
  secondary_electron_tree->Branch("pz0_MeVc", &se_pz0_MeVc, "pz0_MeVc/F");

  secondary_electron_tree->Branch("x1_cm", &se_x1_cm, "x1_cm/F");
  secondary_electron_tree->Branch("y1_cm", &se_y1_cm, "y1_cm/F");
  secondary_electron_tree->Branch("z1_cm", &se_z1_cm, "z1_cm/F");
  secondary_electron_tree->Branch("t1_ns", &se_t1_ns, "t1_ns/F");
  secondary_electron_tree->Branch("ke1_MeV", &se_ke1_MeV, "ke1_MeV/F");

  secondary_electron_tree->Branch("creator",     &se_creator);
  secondary_electron_tree->Branch("end_process", &se_end_process);
  secondary_electron_tree->Branch("end_volume",  &se_end_volume);
}

void SecondaryElectronTree_Fill()
{
  if (secondary_electron_tree) secondary_electron_tree->Fill();
}

void SecondaryElectronTree_Write()
{
  if (!secondary_electron_tree) return;
  secondary_electron_tree->Write("", TObject::kOverwrite);
}
