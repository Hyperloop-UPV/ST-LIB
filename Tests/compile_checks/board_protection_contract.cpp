#include "ST-LIB.hpp"

namespace {

float protected_value = 1.0f;
constexpr uint16_t id = 1001;

inline constexpr auto protected_value_protection =
    Protections::protection<"protected_value", id, protected_value>(Protections::Rules::above(10.0f)
    );

using ContractBoard = ST_LIB::Board<ST_LIB::DefaultFaultPolicy, protected_value_protection>;

static_assert(ContractBoard::ProtectionEngine::protection_count == 1);
static_assert(std::same_as<
              decltype(ContractBoard::protection<protected_value_protection>()),
              Protections::Protection<float, 1>&>);

void compile_board_protection_contract() { ContractBoard::evaluate_protections(); }

} // namespace
