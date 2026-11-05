#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "sbnobj/Common/EventGen/MeVPrtl/MeVPrtlTruth.h"

#include <vector>
#include <algorithm> // std::find

namespace evgen {
namespace ldm {

class MeVPrtlTruthDumper : public art::EDAnalyzer {
public:
  explicit MeVPrtlTruthDumper(fhicl::ParameterSet const& p);

  void analyze(art::Event const& e) override;

private:
  art::InputTag fTag;
};

MeVPrtlTruthDumper::MeVPrtlTruthDumper(fhicl::ParameterSet const& p)
  : art::EDAnalyzer(p)
  , fTag(p.get<art::InputTag>("InputTag", art::InputTag{"generator"}))
{}

void MeVPrtlTruthDumper::analyze(art::Event const& e)
{
  auto const& truths =
    *e.getValidHandle<std::vector<evgen::ldm::MeVPrtlTruth>>(fTag);

  mf::LogInfo("MeVPrtlTruthDumper")
    << "Run " << e.run() << " SubRun " << e.subRun() << " Event " << e.event()
    << " | MeVPrtlTruth count = " << truths.size();

  for (size_t i = 0; i < truths.size(); ++i) {
    auto const& t = truths[i];

    // check e- e+ (order-independent)
    bool has_em = (std::find(t.daughter_pdg.begin(), t.daughter_pdg.end(),  11) != t.daughter_pdg.end());
    bool has_ep = (std::find(t.daughter_pdg.begin(), t.daughter_pdg.end(), -11) != t.daughter_pdg.end());
    bool is_ee = has_em && has_ep;

    mf::LogInfo("MeVPrtlTruthDumper")
      << "  truth[" << i << "] "
      << "mass(GeV)=" << t.mass
      << " C1(mixing)=" << t.C1
      << " meson_pdg=" << t.meson_pdg
      << " gen=" << int(t.gen)
      << " decay_pos(cm,ns)=(" << t.decay_pos.X() << ", " << t.decay_pos.Y() << ", " << t.decay_pos.Z()
      << "; t=" << t.decay_pos.T() << ")"
      << "  ==> e-e+? " << (is_ee ? "YES" : "NO");

    // stampa figlie
    mf::LogInfo("MeVPrtlTruthDumper") << "    ndaughters=" << t.daughter_pdg.size();
    for (size_t j = 0; j < t.daughter_pdg.size(); ++j) {
      int pdg = t.daughter_pdg[j];
      double E = (j < t.daughter_e.size()) ? t.daughter_e[j] : -999.0;

      double px = (j < t.daughter_mom.size()) ? t.daughter_mom[j].X() : -999.0;
      double py = (j < t.daughter_mom.size()) ? t.daughter_mom[j].Y() : -999.0;
      double pz = (j < t.daughter_mom.size()) ? t.daughter_mom[j].Z() : -999.0;

      mf::LogInfo("MeVPrtlTruthDumper")
        << "      [" << j << "] pdg=" << pdg
        << " E(GeV?)=" << E
        << " p=(" << px << "," << py << "," << pz << ")";
    }
  }
}

DEFINE_ART_MODULE(MeVPrtlTruthDumper)

} // namespace ldm
} // namespace evgen