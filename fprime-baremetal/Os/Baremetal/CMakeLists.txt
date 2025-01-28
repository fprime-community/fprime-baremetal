register_os_implementation("Cpu" Baremetal)
register_os_implementation("Memory" Baremetal)
register_os_implementation("Mutex;ConditionVariable" Baremetal)
register_os_implementation("Task" Baremetal fprime-baremetal_Os_TaskRunner)