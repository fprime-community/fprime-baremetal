import sys
import os
import argparse
import json

from baremetal.size.print_cli import *

def main():
    _PATH_BUILD_ARTIFACTS = f'{os.getcwd()}/build-artifacts'
    
    parser = argparse.ArgumentParser(description="F Prime baremetal-size tool to measure binary, deployment, and component sizes.")
    
    parser.add_argument('toolchain', type=str, nargs='?', default='native', help='Name of the toolchain to check (i.e. native, teensy41)')
    
    args = parser.parse_args()

    if args.toolchain == 'native':
        build = sys.platform.capitalize()
    else:
        build = args.toolchain
        
    # Make sure build exists
    if not os.path.isdir(f'{_PATH_BUILD_ARTIFACTS}/{build}'):
        print('Build not found. Maybe try running:')
        print(f'\tfprime-util build {args.toolchain}')
        exit(0)
        
    # Get toolchain path
    if args.toolchain == 'native':
        toolchain_path_suffix = ''
    else:
        try:
            # Load JSON data generated from arduino-cli wrapper
            with open(f'build-fprime-automatic-{args.toolchain}/arduino-cli-compiler-settings.json', 'r') as file:
                data = json.load(file)
        except:
            print(f'{args.toolchain} not yet supported in baremetal-size')
            exit(0)
        c_compiler = data.get('tools').get('C')
        toolchain_path_suffix = c_compiler[:-3]
    
    # Get sizes of all deployments in project
    for deployment in os.listdir(f'{_PATH_BUILD_ARTIFACTS}/{build}'):
        print('\n\n\n')
        print('=========================================')
        print(f'       Deployment: {deployment}         ')
        print('=========================================')

        # Get deployment binary
        binary = f'{_PATH_BUILD_ARTIFACTS}/{build}/{deployment}/bin/{deployment}'
        if not os.path.isfile(binary):
            binary += '.elf'
        if not os.path.isfile(binary):
            print(f'File not found: {binary}')
            continue

        print('------------------------------')
        print(f'    Size for F\' Components   ')
        print('------------------------------')
        exists_tlm_chan, exists_cmd_disp = print_component_size(deployment, toolchain_path_suffix, binary)

        print('\n\n----------------------------------')
        print(f'    Minimum F\' Configurations   ')
        print('----------------------------------')
        print_channel_size(exists_tlm_chan, f'{_PATH_BUILD_ARTIFACTS}/{build}/{deployment}/dict/{deployment}TopologyDictionary.json', deployment)
        print_command_size(exists_cmd_disp, f'{_PATH_BUILD_ARTIFACTS}/{build}/{deployment}/dict/{deployment}TopologyDictionary.json', deployment)

        print('\n------------------------------')
        print(f'    Size for {build}')
        print('------------------------------')
        print_build_size(build, toolchain_path_suffix, binary)


if __name__ == '__main__':
    sys.exit(main())
