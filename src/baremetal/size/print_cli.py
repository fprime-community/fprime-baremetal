import subprocess
import json

# Print Size of Components
def print_component_size(deployment, util_path, bin_path):
    p1 = subprocess.Popen([f'{util_path}nm', bin_path, '-CStd'], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(['grep', f'{deployment}::'], stdin=p1.stdout, stdout=subprocess.PIPE)
    p3 = subprocess.Popen(['sort', '-k2'], stdin=p2.stdout, stdout=subprocess.PIPE)
    output = subprocess.check_output(['grep', ' B '], stdin=p3.stdout)

    output_arr = output.decode('utf-8').strip().split()
    print('.bss (Bytes)\tComponent', end='')
    for i in range(len(output_arr)):
        if i % 4 != 0 and i % 2 != 0:
            if output_arr[i].isnumeric():
                print(int(output_arr[i]), end='\t\t')
            else:
                print(output_arr[i], end='\t')
        elif i % 4 == 0:
            print()
    
    exists_tlm_chan = False
    exists_cmd_disp = False
    
    if f'{deployment}::tlmSend' in output_arr:
        exists_tlm_chan = True
    if f'{deployment}::cmdDisp' in output_arr:
        exists_cmd_disp = True
        
    return exists_tlm_chan, exists_cmd_disp

# Print Size of Telemetry Channels
def print_channel_size(exists_tlm_chan, dict_path, deployment_name):
    if not exists_tlm_chan:
        return

    if '.xml' in dict_path:
        p1 = subprocess.Popen(['cat', dict_path], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', '<channel '], stdin=p1.stdout, stdout=subprocess.PIPE)
        output = subprocess.check_output(['wc', '-l'], stdin=p2.stdout)

        # Find unique components
        p1 = subprocess.Popen(['cat', dict_path], stdout=subprocess.PIPE)
        p2 = subprocess.check_output(['grep', '<channel '], stdin=p1.stdout)
        output_arr = p2.decode('utf-8').strip().split()
        unique_components = []
        for item in output_arr:
            if item.find('component=') == 0 and not item[11:len(item) - 1] in unique_components:
                unique_components.append(item[11:len(item) - 1])

        # Ignore commQueue
        commQueue = 0
        if 'commQueue' in unique_components or 'comQueue' in unique_components:
            commQueue = 1
        print(f'=== Number of Telemetry Channel Hash Slots (config/TlmChanImplCfg.hpp:45)')
        print(f'\t- TLMCHAN_NUM_TLM_HASH_SLOTS = {len(unique_components) - commQueue}')

        # Ignore 15 - CPU Cores
        # Ignore 2  - commQueue
        print(f'=== Number of Telemetry Channel Buckets (config/TlmChanImplCfg.hpp:50)')
        print(f'\t- TLMCHAN_HASH_BUCKETS = {int(output.decode("utf-8").strip()) - 15 - (commQueue * 2)}')
    elif '.json' in dict_path:
        with open(dict_path, 'r') as file:
            data = json.load(file)

            # Find unique components
            unique_components = []
            telemetry_channels = []
            for item in data['telemetryChannels']:
                component_name = item['name'].split('.')[1]
                if component_name == 'commQueue' or component_name == 'comQueue':
                    continue
                if component_name not in unique_components:
                    unique_components.append(component_name)

                telemetry_channels.append(item['name'])

            # Ignore 15 CPU Cores (CPU_01 - CPU_15) if present
            systemResourcesCorrection = 0
            if 'systemResources' in unique_components:
                systemResourcesCorrection = 15
            
            print(f'=== Number of Telemetry Channel Hash Slots ({deployment_name}/config/TlmChanImplCfg.hpp:45)')
            print(f'\t- TLMCHAN_NUM_TLM_HASH_SLOTS = {len(unique_components)}')

            print(f'=== Number of Telemetry Channel Buckets ({deployment_name}/config/TlmChanImplCfg.hpp:50)')
            print(f'\t- TLMCHAN_HASH_BUCKETS = {len(telemetry_channels) - systemResourcesCorrection}')

# Print Size of Commands
def print_command_size(exists_cmd_disp, dict_path, deployment_name):
    if not exists_cmd_disp:
        return
    
    if '.xml' in dict_path:
        p1 = subprocess.Popen(['cat', dict_path], stdout=subprocess.PIPE)
        p2 = subprocess.Popen(['grep', '<command '], stdin=p1.stdout, stdout=subprocess.PIPE)
        output = subprocess.check_output(['wc', '-l'], stdin=p2.stdout)

        print(f'=== Number of Commands ({deployment_name}/config/CommandDispatcherImplCfg.hpp:14)')
        print(f'\t- CMD_DISPATCHER_DISPATCH_TABLE_SIZE = {output.decode("utf-8").strip()}')
    elif '.json' in dict_path:
        with open(dict_path, 'r') as file:
            data = json.load(file)

            # Find unique commands
            unique_commands = []
            for item in data['commands']:
                if item['name'] not in unique_commands:
                    unique_commands.append(item['name'])

            print(f'=== Number of Commands ({deployment_name}config/CommandDispatcherImplCfg.hpp:14)')
            print(f'\t- CMD_DISPATCHER_DISPATCH_TABLE_SIZE = {len(unique_commands)}')

# Print Size of Entire Build
def print_build_size(build, util_path, bin_path):
    subprocess.run([f'{util_path}size', bin_path])
