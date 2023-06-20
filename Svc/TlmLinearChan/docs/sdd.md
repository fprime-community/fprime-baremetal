\page BaremetalTlmLinearChanComponent Baremetal::TlmLinearChan Component
# Baremetal::TlmLinearChan Component

## 1. Introduction

The `Baremetal::TlmLinearChan` component is a variation of the `Svc::TlmChan` F´ core component and 
is used to store telemetry values written by other components. The values are stored in serialized form. 
The data is stored as a set of telemetry channels in a vector. 
The data can be individually read back or periodically pushed to another component for transporting out of the system. 
`Baremetal::TlmLinearChan` is an implementation of the `Svc::TlmStore` component in the `Svc/Tlm` directory.

## 2. Requirements

The requirements for `Baremetal::TlmLinearChan` are as follows:

Requirement | Description | Verification Method
----------- | ----------- | -------------------
TLC-001 | The `Baremetal::TlmLinearChan` component shall provide an interface to submit telemetry | Unit Test
TLC-002 | The `Baremetal::TlmLinearChan` component shall provide an interface to read telemetry | Unit Test
TLC-003 | The `Baremetal::TlmLinearChan` component shall provide an interface to run periodically to write telemetry | Unit Test
TLC-004 | The `Baremetal::TlmLinearChan` component shall write changed telemetry channels when invoked by the run port | Unit Test

## 3. Design

### 3.1 Ports

The `Baremetal::TlmLinearChan` component uses the following port types:

Port Data Type | Name | Direction | Kind | Usage
-------------- | ---- | --------- | ---- | -----
`Svc::Sched` | Run | Input | Asynchronous | Execute a cycle to write changed telemetry channels
`Fw::Tlm` | TlmRecv | Input | Synchronous Input | Update a telemetry channel
`Fw::Tlm` | TlmGet | Input | Synchronous Input | Read a telemetry channel
`Fw::Com` | PktSend | Output | n/a | Write a set of packets with updated telemetry

### 3.2 Functional Description

The `Baremetal::TlmLinearChan` component is identical to the `Svc::TlmChan` component, except it performs a linear search on
a one-dimensional vector rather than a double-buffered table.

### 3.3 State

`Baremetal::TlmLinearChan` has no state machines.

### 3.4 Algorithms

In order to reduce memory usage, a simple one-dimensional array is used to store telemetry channels. To keep the configuration of this array consistent with
`Svc::TlmChan`, the same `TLMCHAN_HASH_BUCKETS` configuration value in `TlmChanImplCfg.h` is used.

## 4. Change Log

Date | Description
---- | -----------
06/20/2023 | Initial Commit


