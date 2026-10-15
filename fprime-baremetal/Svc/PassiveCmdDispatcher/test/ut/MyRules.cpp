// ======================================================================
// \title  MyRules.cpp
// \brief  Rules for PassiveCmdDispatcher
// ======================================================================

#include <iterator>
#include "PassiveCmdDispatcherTester.hpp"

namespace Baremetal {

namespace {
//! Pick a random entry from a non-empty map
template <typename Map>
typename Map::const_iterator pickEntry(const Map& map) {
    auto entry = map.begin();
    std::advance(entry, STest::Pick::startLength(0, static_cast<U32>(map.size())));
    return entry;
}
}  // namespace

// ------------------------------------------------------------------------------------------------------
// Rule:  RegisterCommand
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::RegisterCommand::RegisterCommand(FwIndexType port)
    : STest::Rule<PassiveCmdDispatcherTester>("RegisterCommand"), port(port) {}

bool PassiveCmdDispatcherTester::RegisterCommand::precondition(const PassiveCmdDispatcherTester& state) {
    return state.m_registered.size() < static_cast<size_t>(CMD_DISPATCHER_DISPATCH_TABLE_SIZE);
}

void PassiveCmdDispatcherTester::RegisterCommand::action(PassiveCmdDispatcherTester& state) {
    state.invokeRegister(state.pickUnregisteredOpcode(), this->port);
}

// ------------------------------------------------------------------------------------------------------
// Rule:  ReRegisterCommand
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::ReRegisterCommand::ReRegisterCommand()
    : STest::Rule<PassiveCmdDispatcherTester>("ReRegisterCommand") {}

bool PassiveCmdDispatcherTester::ReRegisterCommand::precondition(const PassiveCmdDispatcherTester& state) {
    return !state.m_registered.empty();
}

void PassiveCmdDispatcherTester::ReRegisterCommand::action(PassiveCmdDispatcherTester& state) {
    auto entry = pickEntry(state.m_registered);
    state.invokeRegister(entry->first, entry->second);
}

// ------------------------------------------------------------------------------------------------------
// Rule:  DispatchCommand
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::DispatchCommand::DispatchCommand(bool useBuffer, FwIndexType port)
    : STest::Rule<PassiveCmdDispatcherTester>(useBuffer ? "DispatchBuffer" : "DispatchCommand"),
      useBuffer(useBuffer),
      port(port) {}

bool PassiveCmdDispatcherTester::DispatchCommand::precondition(const PassiveCmdDispatcherTester& state) {
    return !state.m_registered.empty();
}

void PassiveCmdDispatcherTester::DispatchCommand::action(PassiveCmdDispatcherTester& state) {
    state.invokeDispatch(this->useBuffer, this->port, pickEntry(state.m_registered)->first, STest::Pick::any());
}

// ------------------------------------------------------------------------------------------------------
// Rule:  InvalidOpcode
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::InvalidOpcode::InvalidOpcode(FwOpcodeType opcode)
    : STest::Rule<PassiveCmdDispatcherTester>("InvalidOpcode"), opcode(opcode) {}

bool PassiveCmdDispatcherTester::InvalidOpcode::precondition(const PassiveCmdDispatcherTester& state) {
    return state.m_registered.count(this->opcode) == 0;
}

void PassiveCmdDispatcherTester::InvalidOpcode::action(PassiveCmdDispatcherTester& state) {
    const FwIndexType port = static_cast<FwIndexType>(STest::Pick::startLength(0, CmdDispatcherSequencePorts));
    state.invokeDispatch(false, port, this->opcode, STest::Pick::any());
}

// ------------------------------------------------------------------------------------------------------
// Rule:  MalformedPacket
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::MalformedPacket::MalformedPacket()
    : STest::Rule<PassiveCmdDispatcherTester>("MalformedPacket") {}

bool PassiveCmdDispatcherTester::MalformedPacket::precondition(const PassiveCmdDispatcherTester& state) {
    return true;
}

void PassiveCmdDispatcherTester::MalformedPacket::action(PassiveCmdDispatcherTester& state) {
    const FwIndexType port = static_cast<FwIndexType>(STest::Pick::startLength(0, CmdDispatcherSequencePorts));
    state.invokeMalformed(port, STest::Pick::any());
}

// ------------------------------------------------------------------------------------------------------
// Rule:  CommandStatus
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::CommandStatus::CommandStatus(Fw::CmdResponse response)
    : STest::Rule<PassiveCmdDispatcherTester>("CommandStatus"), response(response) {}

bool PassiveCmdDispatcherTester::CommandStatus::precondition(const PassiveCmdDispatcherTester& state) {
    return !state.m_pending.empty();
}

void PassiveCmdDispatcherTester::CommandStatus::action(PassiveCmdDispatcherTester& state) {
    auto entry = pickEntry(state.m_pending);
    state.invokeStatus(entry->second.opcode, entry->first, this->response);
}

// ------------------------------------------------------------------------------------------------------
// Rule:  UntrackedStatus
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::UntrackedStatus::UntrackedStatus()
    : STest::Rule<PassiveCmdDispatcherTester>("UntrackedStatus") {}

bool PassiveCmdDispatcherTester::UntrackedStatus::precondition(const PassiveCmdDispatcherTester& state) {
    return true;
}

void PassiveCmdDispatcherTester::UntrackedStatus::action(PassiveCmdDispatcherTester& state) {
    // The next sequence number has not been assigned to a command yet, so it cannot be tracked
    state.invokeStatus(STest::Pick::startLength(0, MAX_REGISTERED_OPCODE), state.m_nextSeq, Fw::CmdResponse::OK);
}

// ------------------------------------------------------------------------------------------------------
// Rule:  NoOp
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::NoOp::NoOp() : STest::Rule<PassiveCmdDispatcherTester>("NoOp") {}

bool PassiveCmdDispatcherTester::NoOp::precondition(const PassiveCmdDispatcherTester& state) {
    return true;
}

void PassiveCmdDispatcherTester::NoOp::action(PassiveCmdDispatcherTester& state) {
    state.invokeNoOp(STest::Pick::any());
}

// ------------------------------------------------------------------------------------------------------
// Rule:  ClearTracking
// ------------------------------------------------------------------------------------------------------

PassiveCmdDispatcherTester::ClearTracking::ClearTracking() : STest::Rule<PassiveCmdDispatcherTester>("ClearTracking") {}

bool PassiveCmdDispatcherTester::ClearTracking::precondition(const PassiveCmdDispatcherTester& state) {
    return true;
}

void PassiveCmdDispatcherTester::ClearTracking::action(PassiveCmdDispatcherTester& state) {
    state.invokeClearTracking(STest::Pick::any());
}

}  // namespace Baremetal
