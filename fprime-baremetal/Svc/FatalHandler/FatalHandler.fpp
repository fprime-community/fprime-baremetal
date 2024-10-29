module Baremetal {
    @ Handles FATAL calls
    passive component FatalHandler {
        @ FATAL event receive port
        sync input port FatalReceive: Svc.FatalEvent
  }
}
