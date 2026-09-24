register_fprime_config(
        fprime-baremetal-config
    HEADERS
        "${CMAKE_CURRENT_LIST_DIR}/MicroFsCfg.hpp"
    DEPENDS
        Fw_Types
    INTERFACE
)
