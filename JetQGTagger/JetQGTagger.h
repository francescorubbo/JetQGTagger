#ifndef JETQGTAGGER_H
#define JETQGTAGGER_H

#include "JetQGTagger/IJetQGTagger.h"
#include "PATInterfaces/SystematicsTool.h"
#include "AsgTools/AsgTool.h"

namespace InDet { class InDetTrackSelectionTool; }

namespace CP {

  class JetQGTagger: public CP::IJetQGTagger, public asg::AsgTool, public CP::SystematicsTool{
    ASG_TOOL_CLASS( JetQGTagger, CP::IJetQGTagger)

      public: 
    JetQGTagger( const std::string& name);
    
    virtual StatusCode initialize();
    virtual StatusCode finalize();
    
    StatusCode setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv);

    SystematicCode sysApplySystematicVariation(const SystematicSet& systSet);
    
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
      
  };      

} /* namespace CP */

#endif /* JETQGTAGGER_H */
