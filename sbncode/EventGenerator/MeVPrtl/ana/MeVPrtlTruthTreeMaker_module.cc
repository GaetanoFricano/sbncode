#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art_root_io/TFileService.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbnobj/Common/EventGen/MeVPrtl/MeVPrtlTruth.h"

#include "TTree.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include <vector>
#include <string>
#include <cmath>
#include <limits>
#include <algorithm>

namespace evgen {
namespace ldm {

class MeVPrtlTruthTreeMaker : public art::EDAnalyzer {
public:
  explicit MeVPrtlTruthTreeMaker(fhicl::ParameterSet const& p);
  void analyze(art::Event const& e) override;

private:
  art::InputTag fTag;
  std::string   fTreeName;
  bool          fOneEntryPerTruth;

  // Tree and branches
  TTree* fTree{nullptr};

  // Event identifiers
  int run{}, subrun{}, event{};
  int truth_index{};
  int ntruth{};

  // Weights
  double flux_weight{}, ray_weight{}, decay_weight{}, total_weight{}, pot{};

  // LLP info
  double llp_mass{}, C1{}, C2{}, C3{}, C4{}, C5{};
  int gen{}, meson_pdg{};
  double equiv_enu{};

  // LLP kinematics (detector frame)
  double llp_E{}, llp_px{}, llp_py{}, llp_pz{};
  double llp_p{}, llp_pt{}, llp_eta{}, llp_phi{};
  double cos_theta_beam{}; // assume beam ~ +z in detector coords

  // Parent meson kinematics (optional but useful)
  double meson_E{}, meson_px{}, meson_py{}, meson_pz{};

  // Positions
  double prod_x{}, prod_y{}, prod_z{}, prod_t{};
  double decay_x{}, decay_y{}, decay_z{}, decay_t{};
  double entry_x{}, entry_y{}, entry_z{};
  double exit_x{}, exit_y{}, exit_z{};

  // Geometry / lifetime derived
  double path_length_in_det{};
  double decay_length{};        // distance from production to decay [cm]
  double beta_gamma{};          // p/m
  double proper_decay_length{}; // decay_length/(beta*gamma) [cm]
  double ctau_cm{};             // same as proper_decay_length [cm]

  // Daughters (vectors)
  std::vector<int>    d_pdg;
  std::vector<double> d_E;
  std::vector<double> d_px, d_py, d_pz;

  // Derived from daughters
  int ndaughters{};
  bool is_ee{};
  bool is_nuee{};
  int  nu_pdg{}; // 0 if not present
  double mee{};
  double open_angle_ee{};
  double visible_mass{};   // for nuee, same as mee
  double missing_energy{}; // llp_E - (Ee+ + Ee-) if meaningful

  // Helpers
  static double safe_eta(double px, double py, double pz);
  static double safe_phi(double px, double py);
  static bool is_neutrino(int pdg);
  void reset_per_entry();
  void fill_from_truth(evgen::ldm::MeVPrtlTruth const& t);
};

MeVPrtlTruthTreeMaker::MeVPrtlTruthTreeMaker(fhicl::ParameterSet const& p)
  : art::EDAnalyzer(p)
  , fTag(p.get<art::InputTag>("InputTag", art::InputTag{"generator"}))
  , fTreeName(p.get<std::string>("TreeName", "mevprtl"))
  , fOneEntryPerTruth(p.get<bool>("OneEntryPerTruth", true))
{
  art::ServiceHandle<art::TFileService> tfs;
  fTree = tfs->make<TTree>(fTreeName.c_str(), "MeVPrtlTruth flat tree");

  // --- Scalars
  fTree->Branch("run",   &run);
  fTree->Branch("subrun",&subrun);
  fTree->Branch("event", &event);
  fTree->Branch("ntruth",&ntruth);
  fTree->Branch("truth_index",&truth_index);

  fTree->Branch("pot", &pot);
  fTree->Branch("flux_weight",&flux_weight);
  fTree->Branch("ray_weight",&ray_weight);
  fTree->Branch("decay_weight",&decay_weight);
  fTree->Branch("total_weight",&total_weight);

  fTree->Branch("llp_mass",&llp_mass);
  fTree->Branch("C1",&C1);
  fTree->Branch("C2",&C2);
  fTree->Branch("C3",&C3);
  fTree->Branch("C4",&C4);
  fTree->Branch("C5",&C5);

  fTree->Branch("gen",&gen);
  fTree->Branch("meson_pdg",&meson_pdg);
  fTree->Branch("equiv_enu",&equiv_enu);

  fTree->Branch("llp_E",&llp_E);
  fTree->Branch("llp_px",&llp_px);
  fTree->Branch("llp_py",&llp_py);
  fTree->Branch("llp_pz",&llp_pz);
  fTree->Branch("llp_p",&llp_p);
  fTree->Branch("llp_pt",&llp_pt);
  fTree->Branch("llp_eta",&llp_eta);
  fTree->Branch("llp_phi",&llp_phi);
  fTree->Branch("cos_theta_beam",&cos_theta_beam);

  fTree->Branch("meson_E",&meson_E);
  fTree->Branch("meson_px",&meson_px);
  fTree->Branch("meson_py",&meson_py);
  fTree->Branch("meson_pz",&meson_pz);

  fTree->Branch("prod_x",&prod_x);
  fTree->Branch("prod_y",&prod_y);
  fTree->Branch("prod_z",&prod_z);
  fTree->Branch("prod_t",&prod_t);

  fTree->Branch("decay_x",&decay_x);
  fTree->Branch("decay_y",&decay_y);
  fTree->Branch("decay_z",&decay_z);
  fTree->Branch("decay_t",&decay_t);

  fTree->Branch("entry_x",&entry_x);
  fTree->Branch("entry_y",&entry_y);
  fTree->Branch("entry_z",&entry_z);

  fTree->Branch("exit_x",&exit_x);
  fTree->Branch("exit_y",&exit_y);
  fTree->Branch("exit_z",&exit_z);

  fTree->Branch("path_length_in_det",&path_length_in_det);
  fTree->Branch("decay_length",&decay_length);
  fTree->Branch("beta_gamma",&beta_gamma);
  fTree->Branch("proper_decay_length",&proper_decay_length);
  fTree->Branch("ctau_cm",&ctau_cm);

  fTree->Branch("ndaughters",&ndaughters);
  fTree->Branch("is_ee",&is_ee);
  fTree->Branch("is_nuee",&is_nuee);
  fTree->Branch("nu_pdg",&nu_pdg);

  fTree->Branch("mee",&mee);
  fTree->Branch("open_angle_ee",&open_angle_ee);
  fTree->Branch("visible_mass",&visible_mass);
  fTree->Branch("missing_energy",&missing_energy);

  // --- Vectors
  fTree->Branch("d_pdg",&d_pdg);
  fTree->Branch("d_E",&d_E);
  fTree->Branch("d_px",&d_px);
  fTree->Branch("d_py",&d_py);
  fTree->Branch("d_pz",&d_pz);
}

double MeVPrtlTruthTreeMaker::safe_phi(double px, double py) {
  return std::atan2(py, px);
}

double MeVPrtlTruthTreeMaker::safe_eta(double px, double py, double pz) {
  double p = std::sqrt(px*px + py*py + pz*pz);
  if (p <= 0) return 0.0;
  double cosTheta = std::clamp(pz / p, -1.0, 1.0);
  double theta = std::acos(cosTheta);
  double tanHalf = std::tan(theta/2.0);
  if (tanHalf <= 0) return 0.0;
  return -std::log(tanHalf);
}

bool MeVPrtlTruthTreeMaker::is_neutrino(int pdg) {
  int a = std::abs(pdg);
  return (a == 12 || a == 14 || a == 16);
}

void MeVPrtlTruthTreeMaker::reset_per_entry() {
  pot = 0.0;
  flux_weight = ray_weight = decay_weight = total_weight = 0.0;

  llp_mass = C1 = C2 = C3 = C4 = C5 = 0.0;
  gen = 0; meson_pdg = 0; equiv_enu = 0.0;

  llp_E = llp_px = llp_py = llp_pz = 0.0;
  llp_p = llp_pt = llp_eta = llp_phi = 0.0;
  cos_theta_beam = 0.0;

  meson_E = meson_px = meson_py = meson_pz = 0.0;

  prod_x = prod_y = prod_z = prod_t = 0.0;
  decay_x = decay_y = decay_z = decay_t = 0.0;
  entry_x = entry_y = entry_z = 0.0;
  exit_x = exit_y = exit_z = 0.0;

  path_length_in_det = 0.0;
  decay_length = 0.0;
  beta_gamma = 0.0;
  proper_decay_length = 0.0;
  ctau_cm = 0.0;

  ndaughters = 0;
  is_ee = false;
  is_nuee = false;
  nu_pdg = 0;

  mee = std::numeric_limits<double>::quiet_NaN();
  open_angle_ee = std::numeric_limits<double>::quiet_NaN();
  visible_mass = std::numeric_limits<double>::quiet_NaN();
  missing_energy = std::numeric_limits<double>::quiet_NaN();

  d_pdg.clear();
  d_E.clear();
  d_px.clear(); d_py.clear(); d_pz.clear();
}

void MeVPrtlTruthTreeMaker::fill_from_truth(evgen::ldm::MeVPrtlTruth const& t) {
  // weights
  pot = t.pot;
  flux_weight = t.flux_weight;
  ray_weight = t.ray_weight;
  decay_weight = t.decay_weight;
  total_weight = flux_weight * ray_weight * decay_weight;

  // llp model params
  llp_mass = t.mass;
  C1 = t.C1; C2 = t.C2; C3 = t.C3; C4 = t.C4; C5 = t.C5;
  gen = static_cast<int>(t.gen);
  meson_pdg = t.meson_pdg;
  equiv_enu = t.equiv_enu;

  // LLP 4-momentum (detector frame)
  llp_E  = t.mevprtl_mom.E();
  llp_px = t.mevprtl_mom.Px();
  llp_py = t.mevprtl_mom.Py();
  llp_pz = t.mevprtl_mom.Pz();
  llp_p  = std::sqrt(llp_px*llp_px + llp_py*llp_py + llp_pz*llp_pz);
  llp_pt = std::sqrt(llp_px*llp_px + llp_py*llp_py);
  llp_phi = safe_phi(llp_px, llp_py);
  llp_eta = safe_eta(llp_px, llp_py, llp_pz);
  cos_theta_beam = (llp_p > 0) ? (llp_pz / llp_p) : 0.0;

  // Parent meson 4-momentum (detector frame)
  meson_E  = t.meson_dmom.E();
  meson_px = t.meson_dmom.Px();
  meson_py = t.meson_dmom.Py();
  meson_pz = t.meson_dmom.Pz();

  // positions
  prod_x = t.mevprtl_start.X();
  prod_y = t.mevprtl_start.Y();
  prod_z = t.mevprtl_start.Z();
  prod_t = t.mevprtl_start.T();

  decay_x = t.decay_pos.X();
  decay_y = t.decay_pos.Y();
  decay_z = t.decay_pos.Z();
  decay_t = t.decay_pos.T();

  entry_x = t.mevprtl_enter.X();
  entry_y = t.mevprtl_enter.Y();
  entry_z = t.mevprtl_enter.Z();

  exit_x = t.mevprtl_exit.X();
  exit_y = t.mevprtl_exit.Y();
  exit_z = t.mevprtl_exit.Z();

  // geometry derived
  {
    TVector3 in(entry_x, entry_y, entry_z);
    TVector3 out(exit_x, exit_y, exit_z);
    path_length_in_det = (out - in).Mag();

    TVector3 prod(prod_x, prod_y, prod_z);
    TVector3 dec(decay_x, decay_y, decay_z);
    decay_length = (dec - prod).Mag();

    beta_gamma = (llp_mass > 0) ? (llp_p / llp_mass) : 0.0; // p/m
    proper_decay_length = (beta_gamma > 0) ? (decay_length / beta_gamma) : 0.0;
    ctau_cm = proper_decay_length;
  }

  // daughters
  ndaughters = static_cast<int>(t.daughter_pdg.size());
  d_pdg.reserve(ndaughters);
  d_E.reserve(ndaughters);
  d_px.reserve(ndaughters); d_py.reserve(ndaughters); d_pz.reserve(ndaughters);

  // build TLorentzVectors for each daughter if possible
  std::vector<TLorentzVector> p4;
  p4.reserve(ndaughters);

  for (int i = 0; i < ndaughters; ++i) {
    int pdg = t.daughter_pdg[i];
    double E = (i < (int)t.daughter_e.size()) ? t.daughter_e[i] : 0.0;
    TVector3 p = (i < (int)t.daughter_mom.size()) ? t.daughter_mom[i] : TVector3{0,0,0};

    d_pdg.push_back(pdg);
    d_E.push_back(E);
    d_px.push_back(p.X());
    d_py.push_back(p.Y());
    d_pz.push_back(p.Z());

    p4.emplace_back(p, E);
  }

  // classify topology + compute ee observables if present
  int i_em = -1, i_ep = -1;
  nu_pdg = 0;

  for (int i = 0; i < ndaughters; ++i) {
    if (d_pdg[i] == 11) i_em = i;
    if (d_pdg[i] == -11) i_ep = i;
    if (is_neutrino(d_pdg[i])) nu_pdg = d_pdg[i];
  }

  is_ee = (ndaughters == 2 && i_em >= 0 && i_ep >= 0);
  is_nuee = (ndaughters == 3 && i_em >= 0 && i_ep >= 0 && nu_pdg != 0);

  if (i_em >= 0 && i_ep >= 0) {
    TLorentzVector ee = p4[i_em] + p4[i_ep];
    mee = ee.M();
    visible_mass = mee;

    TVector3 pem = p4[i_em].Vect();
    TVector3 pep = p4[i_ep].Vect();
    double den = pem.Mag() * pep.Mag();
    if (den > 0) {
      double c = std::clamp(pem.Dot(pep)/den, -1.0, 1.0);
      open_angle_ee = std::acos(c);
    } else {
      open_angle_ee = std::numeric_limits<double>::quiet_NaN();
    }

    missing_energy = llp_E - (p4[i_em].E() + p4[i_ep].E());
  }
}

void MeVPrtlTruthTreeMaker::analyze(art::Event const& e) {
  run = e.run();
  subrun = e.subRun();
  event = e.event();

  auto const& truths = *e.getValidHandle<std::vector<evgen::ldm::MeVPrtlTruth>>(fTag);
  ntruth = static_cast<int>(truths.size());

  if (fOneEntryPerTruth) {
    for (int i = 0; i < ntruth; ++i) {
  truth_index = i;
  reset_per_entry();
  fill_from_truth(truths[i]);
  fTree->Fill();
    }
  } else {
    // If you ever want "one entry per event", you’d need vectors-of-truth.
    // For now, keep it simple & analysis-friendly: one entry per truth.
    truth_index = 0;
    reset_per_entry();
    if (!truths.empty()) fill_from_truth(truths.front());
    fTree->Fill();
  }
}

DEFINE_ART_MODULE(MeVPrtlTruthTreeMaker)

} // namespace ldm
} // namespace evgen
