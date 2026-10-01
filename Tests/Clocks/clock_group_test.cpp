#include "HALAL/Models/Clocks/ClockDomain.hpp"
#include "HALAL/Models/SPI/SPI2.hpp"

using namespace ST_LIB;

namespace {
constexpr ClockDomain::ClockTree tree_8m{
    .hse_frequency = 8'000'000,
    .hse_bypass = true,
    .pll1_m = 4,
    .pll1_n = 275,
    .pll1_p = 1,
    .pll1_q = 4,
    .pll1_r = 2,
    .d1cpre = 2,
};
} // namespace

static_assert([] {
    using SpiModel = SPIClockModel<ClockDomain::ClockGroup::SPI123_G, 10'000'000, 0>;
    constexpr auto tree = [] {
        auto t = tree_8m;
        t.spi123_src = ClockDomain::ClockTree::Source::HSI;
        return t;
    }();
    constexpr std::array<ClockDomain::Entry, 1> entries{{
        {.group = SpiModel::group, .try_solve = &SpiModel::try_solve},
    }};
    ClockDomain::validate(tree, std::span<const ClockDomain::Entry, 1>{entries});
    return true;
}());

static_assert([] {
    using SpiModel = SPIClockModel<ClockDomain::ClockGroup::SPI45_G, 1'000'000, 0>;
    constexpr auto tree = [] {
        auto t = tree_8m;
        t.spi45_src = ClockDomain::ClockTree::Source::HSI;
        return t;
    }();
    constexpr std::array<ClockDomain::Entry, 1> entries{{
        {.group = SpiModel::group, .try_solve = &SpiModel::try_solve},
    }};
    ClockDomain::validate(tree, std::span<const ClockDomain::Entry, 1>{entries});
    return true;
}());

static_assert([] {
    using SpiA = SPIClockModel<ClockDomain::ClockGroup::SPI45_G, 20'000'000, 0>;
    using SpiB = SPIClockModel<ClockDomain::ClockGroup::SPI45_G, 10'000'000, 0>;
    constexpr auto tree = [] {
        auto t = tree_8m;
        t.spi45_src = ClockDomain::ClockTree::Source::HSI;
        return t;
    }();
    constexpr std::array<ClockDomain::Entry, 2> entries{{
        {.group = SpiA::group, .try_solve = &SpiA::try_solve},
        {.group = SpiB::group, .try_solve = &SpiB::try_solve},
    }};
    ClockDomain::validate(tree, std::span<const ClockDomain::Entry, 2>{entries});
    return true;
}());

static_assert([] {
    using SpiA = SPIClockModel<ClockDomain::ClockGroup::SPI123_G, 50'000'000, 0>;
    using SpiB = SPIClockModel<ClockDomain::ClockGroup::SPI45_G, 1'000'000, 0>;
    constexpr auto tree = [] {
        auto t = tree_8m;
        t.spi123_src = ClockDomain::ClockTree::Source::HSI;
        t.spi45_src = ClockDomain::ClockTree::Source::PCLK2;
        return t;
    }();
    constexpr std::array<ClockDomain::Entry, 2> entries{{
        {.group = SpiA::group, .try_solve = &SpiA::try_solve},
        {.group = SpiB::group, .try_solve = &SpiB::try_solve},
    }};
    ClockDomain::validate(tree, std::span<const ClockDomain::Entry, 2>{entries});
    return true;
}());

static_assert([] {
    constexpr std::array<ClockDomain::Entry, 0> entries{};
    ClockDomain::validate(tree_8m, std::span<const ClockDomain::Entry, 0>{entries});
    return true;
}());

static_assert([] {
    using Spi123 = SPIClockModel<ClockDomain::ClockGroup::SPI123_G, 10'000'000, 0>;
    using Spi45 = SPIClockModel<ClockDomain::ClockGroup::SPI45_G, 10'000'000, 0>;
    using Spi6 = SPIClockModel<ClockDomain::ClockGroup::SPI6_G, 10'000'000, 0>;
    constexpr auto tree = [] {
        auto t = tree_8m;
        t.spi123_src = ClockDomain::ClockTree::Source::HSI;
        t.spi45_src = ClockDomain::ClockTree::Source::HSI;
        t.spi6_src = ClockDomain::ClockTree::Source::HSI;
        return t;
    }();
    constexpr std::array<ClockDomain::Entry, 3> entries{{
        {.group = Spi123::group, .try_solve = &Spi123::try_solve},
        {.group = Spi45::group, .try_solve = &Spi45::try_solve},
        {.group = Spi6::group, .try_solve = &Spi6::try_solve},
    }};
    ClockDomain::validate(tree, std::span<const ClockDomain::Entry, 3>{entries});
    return true;
}());
