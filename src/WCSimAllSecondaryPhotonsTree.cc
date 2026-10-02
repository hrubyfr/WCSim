#include "WCSimAllSecondaryPhotonsTree.hh"

#include "TObject.h"
#include <unordered_map>

TTree* all_secondary_photons_tree = nullptr;
int asph_evt = -1;
int asph_trk = -1;
int asph_parent = -1;
int asph_parent_pdg = 0;
int asph_parent_parent = -1;
int asph_parent_step = -1;
int asph_pdg = 0;
float asph_parent_ke_MeV = 0.f;
float asph_sx_cm = 0.f;
float asph_sy_cm = 0.f;
float asph_sz_cm = 0.f;
float asph_st_ns = 0.f;
float asph_dir_x = 0.f;
float asph_dir_y = 0.f;
float asph_dir_z = 0.f;
float asph_lambda_nm = 0.f;
int asph_hit_pmt = -1;
float asph_hit_x_cm = 0.f;
float asph_hit_y_cm = 0.f;
float asph_hit_z_cm = 0.f;
float asph_hit_t_ns = 0.f;
int asph_made_pe = 0;
int asph_pe_pmt = -1;
float asph_pe_time_ns = 0.f;
float asph_pe_lambda_nm = 0.f;
float asph_end_x_cm = 0.f;
float asph_end_y_cm = 0.f;
float asph_end_z_cm = 0.f;
float asph_end_t_ns = 0.f;
int asph_end_status = -1;
std::string asph_creator;
std::string asph_parent_creator;
std::string asph_hit_volume;
std::string asph_end_boundary;
std::string asph_end_process;

namespace {
struct PEInfo { int pmt; float time_ns; float lambda_nm; };
std::unordered_map<unsigned long long, PEInfo> pe_cache;
int pe_event = -1;
unsigned long long Key(int event, int track)
{
  return (static_cast<unsigned long long>(static_cast<unsigned int>(event)) << 32) |
         static_cast<unsigned int>(track);
}
}

void AllSecondaryPhotonsTree_ResetVars()
{
  asph_evt = asph_trk = asph_parent = asph_parent_parent = -1;
  asph_parent_pdg = asph_parent_step = asph_pdg = 0;
  asph_parent_ke_MeV = 0.f;
  asph_sx_cm = asph_sy_cm = asph_sz_cm = asph_st_ns = 0.f;
  asph_dir_x = asph_dir_y = asph_dir_z = asph_lambda_nm = 0.f;
  asph_hit_pmt = -1;
  asph_hit_x_cm = asph_hit_y_cm = asph_hit_z_cm = asph_hit_t_ns = 0.f;
  asph_made_pe = 0;
  asph_pe_pmt = -1;
  asph_pe_time_ns = asph_pe_lambda_nm = 0.f;
  asph_end_x_cm = asph_end_y_cm = asph_end_z_cm = asph_end_t_ns = 0.f;
  asph_end_status = -1;
  asph_creator.clear();
  asph_parent_creator.clear();
  asph_hit_volume.clear();
  asph_end_boundary.clear();
  asph_end_process.clear();
}

void AllSecondaryPhotonsTree_Book(TFile* fout)
{
  if (all_secondary_photons_tree || !fout) return;
  fout->cd();
  all_secondary_photons_tree = new TTree(
      "AllSecondaryPhotons",
      "Cherenkov photons produced by primary and secondary particles");
  all_secondary_photons_tree->Branch("evt", &asph_evt, "evt/I");
  all_secondary_photons_tree->Branch("trk", &asph_trk, "trk/I");
  all_secondary_photons_tree->Branch("parent", &asph_parent, "parent/I");
  all_secondary_photons_tree->Branch("parent_pdg", &asph_parent_pdg, "parent_pdg/I");
  all_secondary_photons_tree->Branch("parent_parent", &asph_parent_parent, "parent_parent/I");
  all_secondary_photons_tree->Branch("parent_step", &asph_parent_step, "parent_step/I");
  all_secondary_photons_tree->Branch("pdg", &asph_pdg, "pdg/I");
  all_secondary_photons_tree->Branch("parent_ke_MeV", &asph_parent_ke_MeV, "parent_ke_MeV/F");
  all_secondary_photons_tree->Branch("sx_cm", &asph_sx_cm, "sx_cm/F");
  all_secondary_photons_tree->Branch("sy_cm", &asph_sy_cm, "sy_cm/F");
  all_secondary_photons_tree->Branch("sz_cm", &asph_sz_cm, "sz_cm/F");
  all_secondary_photons_tree->Branch("st_ns", &asph_st_ns, "st_ns/F");
  all_secondary_photons_tree->Branch("dir_x", &asph_dir_x, "dir_x/F");
  all_secondary_photons_tree->Branch("dir_y", &asph_dir_y, "dir_y/F");
  all_secondary_photons_tree->Branch("dir_z", &asph_dir_z, "dir_z/F");
  all_secondary_photons_tree->Branch("lambda_nm", &asph_lambda_nm, "lambda_nm/F");
  all_secondary_photons_tree->Branch("hit_pmt", &asph_hit_pmt, "hit_pmt/I");
  all_secondary_photons_tree->Branch("hit_x_cm", &asph_hit_x_cm, "hit_x_cm/F");
  all_secondary_photons_tree->Branch("hit_y_cm", &asph_hit_y_cm, "hit_y_cm/F");
  all_secondary_photons_tree->Branch("hit_z_cm", &asph_hit_z_cm, "hit_z_cm/F");
  all_secondary_photons_tree->Branch("hit_t_ns", &asph_hit_t_ns, "hit_t_ns/F");
  all_secondary_photons_tree->Branch("made_pe", &asph_made_pe, "made_pe/I");
  all_secondary_photons_tree->Branch("pe_pmt", &asph_pe_pmt, "pe_pmt/I");
  all_secondary_photons_tree->Branch("pe_time_ns", &asph_pe_time_ns, "pe_time_ns/F");
  all_secondary_photons_tree->Branch("pe_lambda_nm", &asph_pe_lambda_nm, "pe_lambda_nm/F");
  all_secondary_photons_tree->Branch("end_x_cm", &asph_end_x_cm, "end_x_cm/F");
  all_secondary_photons_tree->Branch("end_y_cm", &asph_end_y_cm, "end_y_cm/F");
  all_secondary_photons_tree->Branch("end_z_cm", &asph_end_z_cm, "end_z_cm/F");
  all_secondary_photons_tree->Branch("end_t_ns", &asph_end_t_ns, "end_t_ns/F");
  all_secondary_photons_tree->Branch("end_status", &asph_end_status, "end_status/I");
  all_secondary_photons_tree->Branch("creator", &asph_creator);
  all_secondary_photons_tree->Branch("parent_creator", &asph_parent_creator);
  all_secondary_photons_tree->Branch("hit_volume", &asph_hit_volume);
  all_secondary_photons_tree->Branch("end_boundary", &asph_end_boundary);
  all_secondary_photons_tree->Branch("end_process", &asph_end_process);
}

void AllSecondaryPhotonsTree_Fill()
{
  if (all_secondary_photons_tree) all_secondary_photons_tree->Fill();
}

void AllSecondaryPhotonsTree_Write()
{
  if (!all_secondary_photons_tree) return;
  TFile* file = all_secondary_photons_tree->GetCurrentFile();
  if (!file) return;
  file->cd();
  all_secondary_photons_tree->Write("", TObject::kOverwrite);
}

void AllSecondaryPhotonsTree_ResetEvent(int event)
{
  if (event == pe_event) return;
  pe_event = event;
  pe_cache.clear();
}

void AllSecondaryPhotonsTree_NotePE(int event, int track, int pmt,
                                    float time_ns, float lambda_nm)
{
  AllSecondaryPhotonsTree_ResetEvent(event);
  pe_cache[Key(event, track)] = {pmt, time_ns, lambda_nm};
}

bool AllSecondaryPhotonsTree_GetPE(int event, int track, int& pmt,
                                   float& time_ns, float& lambda_nm)
{
  AllSecondaryPhotonsTree_ResetEvent(event);
  auto it = pe_cache.find(Key(event, track));
  if (it == pe_cache.end()) return false;
  pmt = it->second.pmt;
  time_ns = it->second.time_ns;
  lambda_nm = it->second.lambda_nm;
  pe_cache.erase(it);
  return true;
}
