import subprocess

# Print Size of Components
def print_component_size(deployment, util_path, bin_path):
    print('------------------------------')
    print(f'    Size for F\' Components   ')
    print('------------------------------')
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
def print_channel_size(exists_tlm_chan, xml_path):
    if not exists_tlm_chan:
        return

    print('\n\n----------------------------------')
    print(f'    Minimum F\' Configurations   ')
    print('----------------------------------')
    p1 = subprocess.Popen(['cat', xml_path], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(['grep', '<channel '], stdin=p1.stdout, stdout=subprocess.PIPE)
    output = subprocess.check_output(['wc', '-l'], stdin=p2.stdout)

    # Find unique components
    p1 = subprocess.Popen(['cat', xml_path], stdout=subprocess.PIPE)
    p2 = subprocess.check_output(['grep', '<channel '], stdin=p1.stdout)
    output_arr = p2.decode('utf-8').strip().split()
    unique_components = []
    for item in output_arr:
        if item.find('component=') == 0 and not item[11:len(item) - 1] in unique_components:
            unique_components.append(item[11:len(item) - 1])

    # Ignore commQueue
    commQueue = 0
    if 'commQueue' in unique_components:
        commQueue = 1
    print(f'=== Number of Telemetry Channel Hash Slots (config/TlmChanImplCfg.hpp:45)')
    print(f'\t- TLMCHAN_NUM_TLM_HASH_SLOTS = {len(unique_components) - commQueue}')

    # Ignore 15 - CPU Cores
    # Ignore 2  - commQueue
    print(f'=== Number of Telemetry Channel Buckets (config/TlmChanImplCfg.hpp:50)')
    print(f'\t- TLMCHAN_HASH_BUCKETS = {int(output.decode("utf-8").strip()) - 15 - (commQueue * 2)}')

# Print Size of Commands
def print_command_size(exists_cmd_disp, xml_path):
    if not exists_cmd_disp:
        return
    
    p1 = subprocess.Popen(['cat', xml_path], stdout=subprocess.PIPE)
    p2 = subprocess.Popen(['grep', '<command '], stdin=p1.stdout, stdout=subprocess.PIPE)
    output = subprocess.check_output(['wc', '-l'], stdin=p2.stdout)

    print(f'=== Number of Commands (config/CommandDispatcherImplCfg.hpp:14)')
    print(f'\t- CMD_DISPATCHER_DISPATCH_TABLE_SIZE = {output.decode("utf-8").strip()}')

# Print Size of Entire Build
def print_build_size(build, util_path, bin_path):
    print('\n------------------------------')
    print(f'    Size for {build}')
    print('------------------------------')

    subprocess.run([f'{util_path}size', bin_path])
