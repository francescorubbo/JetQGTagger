#ifndef JETQGTAGGER_H
#define JETQGTAGGER_H

#include "JetQGTagger/IJetQGTagger.h"
#include "PATInterfaces/SystematicsTool.h"
#include "AsgTools/AsgTool.h"

namespace InDet { class InDetTrackSelectionTool; }
namespace InDet { class InDetTrackTruthFilterTool; }
namespace InDet { class InDetTrackTruthOriginTool; }
namespace InDet { class JetTrackFilterTool; }

namespace CP {

  enum SystApplied {
    NONE,
    QG_TRACKEFFICIENCY,
    QG_TRACKFAKES,
    QG_NCHARGEDEXP,
    QG_NCHARGEDME,
    QG_NCHARGEDPDF
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
    int getNTrack(const xAOD::Jet * jet, const xAOD::Vertex * pv);
    
    std::string m_taggername;
    std::string m_weight_decoration_name;
    std::string m_tagger_decoration_name;    
    float m_minpt;
    float m_maxeta;
    SG::AuxElement::Decorator< float >* m_taggerdec;
    SG::AuxElement::Decorator< float >* m_weightdec;

    InDet::InDetTrackSelectionTool * m_trkSelectionTool;

    SystApplied m_appliedSystEnum;
    InDet::InDetTrackTruthOriginTool * m_trkTruthOriginTool; //!
    InDet::InDetTrackTruthFilterTool * m_trkTruthFilterTool; //!
    InDet::InDetTrackTruthFilterTool * m_trkFakeTool; //!
    InDet::JetTrackFilterTool * m_jetTrackFilterTool; //!

  };      

} /* namespace CP */

#endif /* JETQGTAGGER_H */
