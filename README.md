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