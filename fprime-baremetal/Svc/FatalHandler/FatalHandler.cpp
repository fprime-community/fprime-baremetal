// ======================================================================
// \title  FatalHandlerImpl.cpp
// \author lestarch
// \brief  cpp file for FatalHandler component implementation class
//
// \copyright
// Copyright 2024, by the California Institute of Technology.
// ALL RIGHTS RESERVED.  United States Government Sponsorship
// acknowledged.
//
// ======================================================================
#include <config/FpConfig.hpp>
#include <Fw/Logger/Logger.hpp>
#include <fprime-baremetal/Svc/FatalHandler/FatalHandler.hpp>

namespace Baremetal {

// ----------------------------------------------------------------------
// Construction, initialization, and destruction
// ----------------------------------------------------------------------

FatalHandler ::FatalHandler(const char* const compName) : FatalHandlerComponentBase(compName) {}

FatalHandler ::~FatalHandler() {}

// ----------------------------------------------------------------------
// Handler implementations for user-defined typed input ports
// ----------------------------------------------------------------------

void FatalHandler::FatalReceive_handler(FwIndexType portNum, FwEventIdType Id) {
    // for **nix, delay then exit with error code
    Fw::Logger::log("FATAL %" PRI_FwEventIdType "handled.\n", Id);
    while (true) {}  // Returning might be bad
}

}  // end namespace Svc
