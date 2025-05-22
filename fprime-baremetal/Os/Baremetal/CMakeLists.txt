####
# F prime CMakeLists.txt:
#
# SOURCE_FILES: combined list of source and autocoding files
# MOD_DEPS: (optional) module dependencies
#
####
add_custom_target("${FPRIME_CURRENT_MODULE}")

# Set up Baremetal implementation
register_os_implementation("Cpu" Baremetal)
register_os_implementation("Memory" Baremetal)
register_os_implementation("Task" Baremetal fprime-baremetal_Os_TaskRunner)
