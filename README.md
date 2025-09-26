# fprime-baremetal

Add this project as a git submodule to your project, and then list the folder as a library in your settings.ini file.

This project provides components designed for baremetal F´ deployments with the intention of reducing memory usage.

## baremetal-size

This package also includes a baremetal sizing utility that measures the .bss size of each F´ component and deployment, as well
as provides a recommendation of configuration values to reduce memory usage.

To install or upgrade this utility, clone this repository and run:
```shell
# In fprime-baremetal
pip install --upgrade .
```

Usage:
```shell
baremetal-size {build}
```

Change `{build}` to your specific build (i.e. `teensy41`, `featherM0`, etc.)

## Tracking memory allocation done by new & delete
fprime-baremental includes a feature which overrides the default implementations of new, new[], delete, and delete[] with calls to a Fw::MallocAllocator class. 
There are also helper functions for registering a Fw::MallocAllocator and for setting the default memoryId to be used when allocating memory. 
This feature is disabled by default, it can be enabled by setting FPRIME_BAREMENTAL_OVERRIDE_NEW_DELETE in your project's top level CMakeLists.txt.
One example of how to leverage this feature is the StrictMallocAllocator class in fprime-vorago