#ifndef IJETQGTAGGER_H
#define IJETQGTAGGER_H

#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"

#include "xAODJet/Jet.h"

namespace CP {

  namespace QGntrackSyst {
    const static SystematicVariation trackefficiency("QG_trackEfficiency");
    const static SystematicVariation trackfakes("QG_trackFakes");
    const static SystematicVariation nchargedexp_up("QG_nchargedExp_up");
    const static SystematicVariation nchargedme_up("QG_nchargedME_up");
    const static SystematicVariation nchargedpdf_up("QG_nchargedPDF_up");
    const static SystematicVariation nchargedexp_down("QG_nchargedExp_down");
    const static SystematicVariation nchargedme_down("QG_nchargedME_down");
    const static SystematicVariation nchargedpdf_down("QG_nchargedPDF_down");
  } //namespace QGntrackSyst

  class IJetQGTagger : public virtual CP::ISystematicsTool {
    
    ASG_TOOL_INTERFACE( CP::IJetQGTagger )
      
      public:
    
    virtual ~IJetQGTagger() {}
    
    virtual StatusCode setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv) = 0;
    
  };

} // namespace CP

#endif /* IJETQGTAGGER_H */
