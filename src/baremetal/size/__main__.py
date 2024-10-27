import sys
from sys import platform
import os

from baremetal.size.print_cli import print_component_size, print_channel_size, print_command_size, print_build_size

ROOT_USER = os.path.expanduser('~')
PROJECT_ROOT = os.getcwd()
BUILD_ARTIFACTS = f'{PROJECT_ROOT}/build-artifacts'

# Supported Toolchain paths
# =======================================================================
toolchains = {
    'esp32': f'{ROOT_USER}/.arduino15/packages/esp32/tools/xtensa-esp32-elf-gcc/esp-2021r2-patch5-8.4.0/bin/xtensa-esp32-elf-',
    'featherM0': f'{ROOT_USER}/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-',
    'native': '',
    'rpipicow': f'{ROOT_USER}/.arduino15/packages/rp2040/tools/pqt-gcc/2.1.0-a-d3d2e6b/bin/arm-none-eabi-',
    'teensy32': f'{ROOT_USER}/.arduino15/packages/teensy/tools/teensy-compile/11.3.1/arm/bin/arm-none-eabi-',
    'teensy40': f'{ROOT_USER}/.arduino15/packages/teensy/tools/teensy-compile/11.3.1/arm/bin/arm-none-eabi-',
    'teensy41': f'{ROOT_USER}/.arduino15/packages/teensy/tools/teensy-compile/11.3.1/arm/bin/arm-none-eabi-',
}
# =======================================================================

_EXISTS_CMD_DISP = False
_EXISTS_TLM_CHAN = False

def main():
    global _EXISTS_CMD_DISP
    global _EXISTS_TLM_CHAN

    argc = len(sys.argv)
    argv = sys.argv

    # Check arguments for build name
    if argc != 2:
        print('Specify build. (i.e. $ baremetal-size teensy41)')
        return
    _build = argv[1]

    if _build == 'native':
        build = platform.capitalize()
    else:
        build = _build

    # Check if build is supported for size checking
    if not _build in toolchains.keys():
        print(f'ERROR: Size check for {_build} not yet supported! Exiting...')
        return

    SIZE_UTIL_EXECS = toolchains[_build]

    # Check if build exists
    if not os.path.exists(f'{BUILD_ARTIFACTS}/{build}'):
        print('Build not found. Maybe try running:')
        print(f'  fprime-util build {_build}')
        return

    for deployment in os.listdir(f'{BUILD_ARTIFACTS}/{build}'):
        print('\n\n\n')
        print('=========================================')
        print(f'       Deployment: {deployment}         ')
        print('=========================================')

        BIN = f'{BUILD_ARTIFACTS}/{build}/{deployment}/bin/{deployment}'

        print_component_size(deployment, SIZE_UTIL_EXECS, BIN)
        print_channel_size(f'{BUILD_ARTIFACTS}/{build}/{deployment}/dict/{deployment}TopologyAppDictionary.xml')
        print_command_size(f'{BUILD_ARTIFACTS}/{build}/{deployment}/dict/{deployment}TopologyAppDictionary.xml')
        print_build_size(build, SIZE_UTIL_EXECS, BIN)

        _EXISTS_CMD_DISP = False
        _EXISTS_TLM_CHAN = False

if __name__ == '__main__':
    sys.exit(main())
    
