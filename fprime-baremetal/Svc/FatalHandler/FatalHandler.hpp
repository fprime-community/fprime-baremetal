// ======================================================================
// \title  FatalHandlerImpl.hpp
// \author lestarch
// \brief  hpp file for FatalHandler component implementation class
//
// \copyright
// Copyright 2009-2015, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================

#ifndef BAREMETAL_FatalHandler_HPP
#define BAREMETAL_FatalHandler_HPP

#include "fprime-baremetal/Svc/FatalHandler/FatalHandlerComponentAc.hpp"

namespace Baremetal {

class FatalHandler : public FatalHandlerComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Construction, initialization, and destruction
    // ----------------------------------------------------------------------

    //! Construct object FatalHandler
    //!
    FatalHandler(const char* const compName /*!< The component name*/
    );

    //! Destroy object FatalHandler
    //!
    ~FatalHandler();

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for user-defined typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for FatalReceive
    //!
    void FatalReceive_handler(const FwIndexType portNum, /*!< The port number*/
                              FwEventIdType Id           /*!< The ID of the FATAL event*/
    );
};

}  // end namespace Svc

#endif
