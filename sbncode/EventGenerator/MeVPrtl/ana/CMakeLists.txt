cet_build_plugin( MeVPrtlTruthDumper art::module
  LIBRARIES
    art::Framework_Core
    art::Framework_Principal
    art::Framework_Services_Registry
    art::Persistency_Common
    art::Utilities
    canvas::canvas
    fhiclcpp::fhiclcpp
    messagefacility::MF_MessageLogger
    cetlib::cetlib cetlib_except::cetlib_except
    sbnobj::Common_EventGen_MeVPrtl
)
cet_build_plugin( MeVPrtlTruthTreeMaker art::module
  LIBRARIES
    art::Framework_Core
    art::Framework_Principal
    art::Framework_Services_Registry
    art_root_io::TFileService_service
    canvas::canvas
    fhiclcpp::fhiclcpp
    messagefacility::MF_MessageLogger
    cetlib::cetlib cetlib_except::cetlib_except
    sbnobj::Common_EventGen_MeVPrtl
    ROOT::Tree
    ROOT::Core
    ROOT::Physics
)

install_headers()
install_source()
install_headers()
install_source()