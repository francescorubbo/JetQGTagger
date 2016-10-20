#ifndef IJETQGTAGGER_H
#define IJETQGTAGGER_H

#include "PATInterfaces/CorrectionCode.h"
#include "PATInterfaces/ISystematicsTool.h"

#include "xAODJet/Jet.h"

namespace CP {

  namespace QGntrackSyst {
    const static SystematicVariation trackefficiency("QG_trackEfficiency");
    const static SystematicVariation trackfakes("QG_trackFakes");
    const static SystematicVariation nchargedexp("QG_nchargedExp");
    const static SystematicVariation nchargedme("QG_nchargedME");
    const static SystematicVariation nchargedpdf("QG_nchargedPDF");
  } //namespace QGntrackSyst



  class IJetQGTagger : public virtual CP::ISystematicsTool {
    
    ASG_TOOL_INTERFACE( CP::IJetQGTagger )
      
      public:
    
    virtual ~IJetQGTagger() {}
    
    virtual StatusCode setTagger(const xAOD::Jet * jet, const xAOD::Vertex * pv) = 0;
    
  };

} // namespace CP

#endif /* IJETQGTAGGER_H */
