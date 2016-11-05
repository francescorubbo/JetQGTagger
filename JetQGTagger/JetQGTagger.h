#ifndef JETQGTAGGER_H
#define JETQGTAGGER_H

#include "JetQGTagger/IJetQGTagger.h"
#include "PATInterfaces/SystematicsTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "InDetTrackSystematicsTools/InDetTrackTruthOriginTool.h"

#include <TH2D.h>

namespace InDet { class IInDetTrackSelectionTool; }
namespace InDet { class IInDetTrackTruthFilterTool; }
namespace InDet { class IJetTrackFilterTool; }

namespace CP {

  enum SystApplied {
    NONE,
    QG_TRACKEFFICIENCY,
    QG_TRACKFAKES,
    QG_NCHARGEDEXP_UP,
    QG_NCHARGEDME_UP,
    QG_NCHARGEDPDF_UP,
    QG_NCHARGEDEXP_DOWN,
    QG_NCHARGEDME_DOWN,
    QG_NCHARGEDPDF_DOWN
  };

  class JetQGTagger: public IJetQGTagger, public asg::AsgTool, public SystematicsTool{
    ASG_TOOL_CLASS( JetQGTagger, IJetQGTagger)
      
      public: 
    JetQGTagger( const std::string& name);
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    
    StatusCode setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv);

    bool isAffectedBySystematic(const SystematicVariation& var) const{return SystematicsTool::isAffectedBySystematic(var);}
    SystematicSet affectingSystematics() const {return SystematicsTool::affectingSystematics();}
    SystematicSet recommendedSystematics() const {return SystematicsTool::recommendedSystematics();}
    SystematicCode applySystematicVariation(const SystematicSet& set) {return SystematicsTool::applySystematicVariation(set);}
    SystematicCode sysApplySystematicVariation(const SystematicSet&);

  private:
    JetQGTagger();
    StatusCode getNTrack(const xAOD::Jet * jet, const xAOD::Vertex * pv, int &ntracks);
    StatusCode getNTrackWeight(const xAOD::Jet * jet, double &weight);

    SystApplied m_appliedSystEnum;
    
    TH2D* m_hquark;
    TH2D* m_hgluon;

    TH2D* m_exp_hquark_up;
    TH2D* m_exp_hquark_down;
    TH2D* m_exp_hgluon_up;
    TH2D* m_exp_hgluon_down;

    TH2D* m_me_hquark_up;
    TH2D* m_me_hquark_down;
    TH2D* m_me_hgluon_up;
    TH2D* m_me_hgluon_down;

    TH2D* m_pdf_hquark_up;
    TH2D* m_pdf_hquark_down;
    TH2D* m_pdf_hgluon_up;
    TH2D* m_pdf_hgluon_down;

    StatusCode loadHist(TH2D *&hist,std::string filename,std::string histname);

    std::string m_taggername;
    std::string m_expfile;
    std::string m_mefile;
    std::string m_pdffile;
    std::string m_weight_decoration_name;
    std::string m_tagger_decoration_name;    
    float m_minpt;
    float m_maxeta;
    SG::AuxElement::Decorator< float >* m_taggerdec;
    SG::AuxElement::Decorator< float >* m_weightdec;

    asg::AnaToolHandle<InDet::IInDetTrackSelectionTool> m_trkSelectionTool;
    asg::AnaToolHandle<InDet::IInDetTrackTruthFilterTool> m_trkTruthFilterTool;
    asg::AnaToolHandle<InDet::IInDetTrackTruthFilterTool> m_trkFakeTool;
    asg::AnaToolHandle<InDet::IJetTrackFilterTool> m_jetTrackFilterTool;
    std::unique_ptr<InDet::InDetTrackTruthOriginTool> m_originTool;

  };      

} /* namespace CP */

#endif /* JETQGTAGGER_H */
