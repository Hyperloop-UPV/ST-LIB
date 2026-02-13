#pragma once

#include "HALAL/Models/GPIO.hpp"
#include "HALAL/Models/Pin.hpp"
#define Oversampling_MAX 1024
#define Oversampling_MAX_Filter_4 215
#define Oversampling_MAX_Filter_5 73
#define Possible_Pin_Channel 20
using ST_LIB::GPIODomain;

namespace ST_LIB {
    using Callback = void(*)(void);
    extern void compile_error(const char *msg);

    struct DFSDM_DOMAIN{
   /* Constant Values of Register*/
   //DatPack = 0 Standar
   //DatMPx = 0 // External input
   //ChinSel = 0  // It make sense for our use case.
   //SPICKSel = ¿?
   enum class SPICKSel: uint8_t{
        CLK_IN = 0, 
        NORMAL_CLK_OUT = 1,
        CLK_DIVIDED_2_RISING = 2,
        CLK_DIVIDED_2_FALLING = 3
   };
   enum class SPI_Type: uint8_t{
        SPI_RISING = 0,
        SPI_FALLING = 1
   };

    enum  class Filter_Type : uint8_t {
        FastSinc = 0,
        Sinc1    = 1,
        Sinc2    = 2,
        Sinc3    = 3,
        Sinc4    = 4,
        Sinc5    = 5
    };

    enum  Fast_Conversion : uint8_t {
        Disable = 0,
        Enable  = 1
    };

    enum  class Data_Write : uint8_t {
        CPU = 0,
        DMA = 1
    };

    enum  class Sync_Conversion : uint8_t {
        Independent = 0,
        Sync_With_Flt0 = 1
    };

    enum  class Regular_Mode : uint8_t {
        Single     = 0,
        Continuous = 1
    };

    enum  class Analog_Watchdog_Fast_Mode : uint8_t {
        After_Filter = 0,   // AWFSEL = 0
        Channel_Data = 1    // AWFSEL = 1
    };
    //AWFSEL = 0 high precision, slow speed <- Ideally for our case, 
    //AWFSEL = 1 16 bits precision ultra high speed oversampling ratio 1..32 filter (1..3) 8 relojes de clock
    
    enum class Analog_Watchdog_Sinc: uint8_t{
        FastSinc = 0,
        sinc1 = 1,
        Sinc2 = 2,
        Sinc3 = 3,
        Sinc4 = 4,
        Sinc5 = 5
    };

    enum  class Analog_Watchdog : uint8_t {
        Disable = 0,
        Enable  = 1
    };

    enum  class Overrun : uint8_t {
        Enable = 0,
        Disable = 1
    };
    enum  class Clock_Absence : uint8_t{
        Disable = 0,
        Enable = 1
    };
    enum class Short_Circuit : uint8_t{
        Disable = 0,
        Enable = 1
    };
    enum class Extreme_Detector : uint8_t{
        Disable = 0,
        Enable = 1
    };

    struct Entry{
        uint8_t channel;
        uint32_t offset;
        uint32_t right_shift; // right shift
        SPICKSel spi_clock_sel;
        SPI_Type spi_type;
        Filter_Type filter_type;
        uint16_t oversampling; //1..1024
        uint16_t integrator; //1..256
        uint8_t Short_Circuit_Count; // Number of bits with the same value to guess that has been a Short Circuit

        Data_Write rdma;

        Fast_Conversion fast;
        Sync_Conversion rsync;
        Regular_Mode rcont;

        Clock_Absence ckabie;
        Overrun rovrie;
        Short_Circuit scdie;
        
        Extreme_Detector exch;
        uint8_t pulse_skip; // 0..63 Skip invalid values don't use please.
    };
    static constexpr std::array<std::pair<GPIODomain::Pin,uint8_t>,Possible_Pin_Channel> pin_to_channel =
    {{
        {PE4,3},{PC0,4},{PC1,0},{PC3,1},{PC5,2},
        {PB1,1},{PF13,6},{PE7,2},{PE10,4},{PE12,5},{PB10,7},{PB12,1},
        {PB14,2},{PD9,3},{PC7,3},{PC11,5},{PD1,6},{PD6,1},{PD7,4},
        {PB6,5}
    }};

    static consteval uint8_t get_channel(const GPIODomain::Pin& pin){
        for(int i = 0; i < Possible_Pin_Channel;i++){
            if(pin_to_channel[i].first == pin){
                return pin_to_channel[i].second;
            }
        }
        compile_error("This pin cannot be used as a DFSDM_Channel")
    };

       

    static consteval bool is_correct_oversampling(Filter_Type f, uint16_t osr) {
        switch (f) {
            case Filter_Type::FastSinc:
            case Filter_Type::Sinc1:
            case Filter_Type::Sinc2:
            case Filter_Type::Sinc3:
                return osr >= 1 && osr <= Oversampling_MAX;

            case Filter_Type::Sinc4:
                return osr >= 1 && osr <= Oversampling_MAX_Filter_4;

            case Filter_Type::Sinc5:
                return osr >= 1 && osr <= Oversampling_MAX_Filter_5;
        }
        return false;
    }




    struct DFSDM{
        using domain = DFSDM_DOMAIN;
        Entry e;
        static constexpr size_t max_instances{8};
        consteval DFSDM(GPIODomain::Pin& pin,uint32_t offset,uint32_t right_shift,
        SPICKSel spi_clock_sel,SPI_Type spi_type,Filter_Type filter_type = Filter_Type::FastSinc,
        uint16_t oversampling = 1,uint8_t integrator = 1,uint8_t Short_Circuit_Count = 0xFF,
        Data_Write rdma = Data_Write::DMA, Fast_Conversion fast = Fast_Conversion::Enable,
        Sync_Conversion rsync = Sync_Conversion::Sync_With_Flt0, Regular_Mode rcont = Regular_Mode::Continuous,
        Clock_Absence ckabie = Clock_Absence::Enable, Overrun rovrie = Overrun::Enable, 
        Short_Circuit scdie = Short_Circuit::Enable,Extreme_Detector exch = Extreme_Detector::Enable,
        uint8_t pulse_skip = 0)
    : e{
        .channel       = get_channel(pin),
        .right_shift   = right_shift,
        .spi_clock_sel = spi_clock_sel,
        .spi_type      = spi_type,
        .filter_type   = filter_type,
        .oversampling  = oversampling,
        .integrator    = integrator,
        .Short_Circuit_Count = Short_Circuit_Count,
        .rdma          = rdma,
        .fast          = fast,
        .rsync         = rsync,
        .rcont         = rcont,
        .ckabie        = ckabie,
        .rovrie        = rovrie,
        .scdie         = scdie,
        .exch          = exch,
        .pulse_skip    = pulse_skip
      }
      {
        if(integrator <= 0){
            compile_error("DFSDM_FILTER: Integrator out of range");
        }
        if (!is_correct_oversampling(filter_type, oversampling))
            compile_error("DFSDM_FILTER: invalid oversampling for selected filter type");
        
    }
    template<class Ctx>
    consteval std::size_t inscribe(Ctx &ctx) const {
        return ctx.template add<DFSDM_DOMAIN>(e, this);
    }
    struct Config {
       uint8_t filter;
       uint8_t channel;
       DFSDM_Filter_TypeDef init_data_filter;
       DFSDM_Channel_TypeDef init_data_channel; 

       uint32_t latency_cycles;
    };
    static consteval uint32_t compute_latency(const Entry& e){
        const uint32_t fosr = e.oversampling;
        const uint32_t iosr = e.integrator;

        if (e.fast == Fast_Conversion::Enable &&
            e.rcont == Regular_Mode::Continuous)
        {
            return fosr * iosr;
        }

        if (e.filter_type == Filter_Type::FastSinc) {
            return fosr * (iosr - 1 + 4) + 2;
        }

        const uint32_t ford = static_cast<uint32_t>(e.filter_type);
        return fosr * (iosr - 1 + ford) + ford;
    }
    static consteval uint32_t make_fltfcr(const Entry& e)
    {
        return
            (uint32_t(e.filter_type) << DFSDM_FLTFCR_FORD_Pos) |
            (uint32_t(e.oversampling -1) << DFSDM_FLTFCR_FOSR_Pos) |
            (uint32_t(e.integrator - 1) << DFSDM_FLTFCR_IOSR_Pos);
    }
    static consteval uint32_t make_fltcr1(const Entry& e)
    {
        return
        (uint32_t(e.fast)   << DFSDM_FLTCR1_FAST_Pos)   |
        (uint32_t(e.channel)<< DFSDM_FLTCR1_RCH_Pos)    |
        (uint32_t(e.rdma)   << DFSDM_FLTCR1_RDMAEN_Pos) |
        (uint32_t(e.rsync)  << DFSDM_FLTCR1_RSYNC_Pos)  |
        (uint32_t(e.rcont)  << DFSDM_FLTCR1_RCONT_Pos)  |
        DFSDM_FLTCR1_DFEN;
    }
    static consteval uint32_t make_fltcr2(const Entry& e)
    {
        uint32_t v = 0;

        // Extreme detector enable (bitmap)
        if (e.exch == Extreme_Detector::Enable)
            v |= (1u << (DFSDM_FLTCR2_EXCH_Pos + e.channel));
        // Global interrupts
        v |= (uint32_t(e.rovrie) << DFSDM_FLTCR2_ROVRIE_Pos);
        return v;
    }
    template <size_t N>
    static consteval std::array<Config, N> build(std::span<const Entry> entries) {
        std::array<Config, N> cfgs{};
        std::array<bool,8> channels_used;
        std::array<uint8_t,4> filters_used{0};
        bool filter_per_channel = (N <= 4) ? true : false;
        for (size_t i = 0; i < N; ++i) {
            DFSDM_Filter_TypeDef filter_config{};
            DFSDM_Channel_TypeDef channel_config{};
            const auto &e = entries[i];

            if(channels_used[e.channel] == true){
                compile_error("You have two pins using the same channel");
            }
            channels_used[e.channel] = true;
            auto& cfg = cfgs[i];

            cfg.channel = e.channel;
            if(filter_per_channel){
                cfg.filter = i;
            }else{
                cfg.filter = e.channel / 4;
            }
            filter_config.FLTCR1 |= make_fltcr1(e);
            filter_config.FLTCR2 |= make_fltcr2(e);
            filter_config.FLTC   |= make_fltfcr(e);
            if(filters_used[cfg.filter] != 0 && cfgs[filters_used[cfg.filter]].filter_config != cfg.filter_config){
                compile_error("You have two channels that goes to the same filter with different filter configuration");
            }
            filters_used[cfg.filter] = i;    
        
        }
        return cfgs;
    };

    struct Instance {
        DFSDM_Filter_TypeDef *regs{};
        Callback watchdog_cb{};
        private:
            bool is_enabled() const {
                return (regs->FLTCR1 & DFSDM_FLTCR1_DFEN);
            }

        public:
            bool enable() {
                regs->FLTCR1 |= DFSDM_FLTCR1_DFEN;
                return is_enabled();
            }

            bool disable() {
                regs->FLTCR1 &= ~DFSDM_FLTCR1_DFEN;
                return !is_enabled();
            }

            bool start() {
               if(!enable()) return false;
                regs->FLTCR1 |= DFSDM_FLTCR1_RSWSTART;
                return true;
            }

            void modify_sync_conversion(Sync_Conversion type) {
                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTCR1 &= ~DFSDM_FLTCR1_RSYNC_Msk;
                regs->FLTCR1 |= (uint32_t(type) << DFSDM_FLTCR1_RSYNC_Pos);

                if (was_enabled) enable();
            }
                       void modify_mode(Regular_Mode mode) {
                regs->FLTCR1 &= ~DFSDM_FLTCR1_RCONT_Msk;
                regs->FLTCR1 |= (uint32_t(mode) << DFSDM_FLTCR1_RCONT_Pos);
            }

            bool modify_oversampling(uint16_t oversampling) {

                if (oversampling == 0) return false;

                uint32_t ford = (regs->FLTFCR & DFSDM_FLTFCR_FORD_Msk)  >> DFSDM_FLTFCR_FORD_Pos;

                if (ford <= 3 && oversampling > Oversampling_MAX) return false;
                if (ford == 4 && oversampling > Oversampling_MAX_Filter_4)  return false;
                if (ford == 5 && oversampling > Oversampling_MAX_Filter_5)   return false;

                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTFCR &= ~DFSDM_FLTFCR_FOSR_Msk;
                regs->FLTFCR |= ((uint32_t)(oversampling -1) << DFSDM_FLTFCR_FOSR_Pos);

                if (was_enabled) enable();

                return true;
            }

            bool modify_integrator(uint8_t integrator) {

                if (integrator == 0 || integrator > 256) return false;
                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTFCR &= ~DFSDM_FLTFCR_IOSR_Msk;
                regs->FLTFCR |= ((integrator -1) << DFSDM_FLTFCR_IOSR_Pos);

                if (was_enabled) enable();
                return true;
            }

            bool modify_filter_order(Filter_Type type) {

                uint32_t fosr =((regs->FLTFCR & DFSDM_FLTFCR_FOSR_Msk) >> DFSDM_FLTFCR_FOSR_Pos);

                if (type == Filter_Type::Sinc4 && fosr > Oversampling_MAX_Filter_4) return false;
                if (type == Filter_Type::Sinc5 && fosr > Oversampling_MAX_Filter_5)  return false;

                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTFCR &= ~DFSDM_FLTFCR_FORD_Msk;
                regs->FLTFCR |= (uint32_t(type)
                                << DFSDM_FLTFCR_FORD_Pos);

                if (was_enabled) enable();
                return true;
            }

            uint32_t check_latency_cycles() {
                return regs->FLTCNVTIMR >> DFSDM_FLTCNVTIMR_CNVCNT_Pos;
            }

            uint32_t check_min_extreme_detector() {
                return regs->FLTEXMIN >> DFSDM_FLTEXMIN_EXMIN_Pos;
            }

            uint32_t check_max_extreme_detector() {
                return regs->FLTEXMAX >> DFSDM_FLTEXMAX_EXMAX_Pos;
            }

            bool clear_watchdog_high() {
                regs->FLTAWCFR |= DFSDM_FLTAWCFR_CLRAWHTF;
                return true;
            }

            bool clear_watchdog_low() {
                regs->FLTAWCFR |= DFSDM_FLTAWCFR_CLRAWLTF;
                return true;
            }

            bool modify_analog_watchdog_lth(uint32_t value) {

                bool was_enabled = is_enabled();
                if (was_enabled) disable();
                regs->FLTAWLTR &= ~DFSDM_FLTAWLTR_AWLT_Msk;
                bool fast = (regs->FLTCR1 & DFSDM_FLTCR1_AWFSEL);

                if (fast)
                    regs->FLTAWLTR = (value & 0xFFFF) << (DFSDM_FLTAWLTR_AWLT_Pos + DFSDM_FLTAWLTR_AWLT_Pos); // Only 16 bits
                else
                    regs->FLTAWLTR = (value & 0xFFFFFF) << DFSDM_FLTAWLTR_AWLT_Pos; // 24 bits

                if (was_enabled) enable();
                return true;
            }

            bool modify_analog_watchdog_hth(uint32_t value) {

                bool was_enabled = is_enabled();
                if (was_enabled) disable();
                regs->FLTAWLTR &=  ~DFSDM_FLTAWLTR_AWLT_Msk;
                bool fast = (regs->FLTCR1 & DFSDM_FLTCR1_AWFSEL);
                
                if (fast)
                    regs->FLTAWHTR = (value & 0xFFFF) << (DFSDM_FLTAWHTR_AWHT_Pos + DFSDM_FLTAWHTR_AWHT_Pos);
                else
                    regs->FLTAWHTR = (value & 0xFFFFFF) << DFSDM_FLTAWHTR_AWHT_Pos;

                if (was_enabled) enable();
                return true;
            }

            uint32_t read_data_from_filter_register() {

                if (!(regs->FLTISR & DFSDM_FLTISR_REOCF))
                    return 0xFFFFFFFFu;

                return regs->FLTRDATAR >> DFSDM_FLTRDATAR_RDATA_Pos;
            }

            bool activate_dma_reading() {

                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTCR1 |= DFSDM_FLTCR1_RDMAEN;

                if (was_enabled) enable();
                return true;
            }

            bool activate_cpu() {

                bool was_enabled = is_enabled();
                if (was_enabled) disable();

                regs->FLTCR1 &= ~DFSDM_FLTCR1_RDMAEN;

                if (was_enabled) enable();
                return true;
            }
};

    
    template <std::size_t N> struct Init {
        static constexpr DFSDM_Filter_TypeDef* filter_hw[4] = {
            DFSDM1_Filter0,
            DFSDM1_Filter1,
            DFSDM1_Filter2,
            DFSDM1_Filter3
        };
        static inline std::array<Instance, N> instances{};

        static void init(std::span<const Config, N> cfgs) {

            for (std::size_t i = 0; i < N; ++i) {
                    const auto &cfg = cfgs[i];
                    auto &inst = instances[i];
                    inst.regs = filter_hw[i];
                    if (!cfg.used_filter)
                        continue;

                    inst.regs = filter_hw[i];
                    inst.latency_cycles = cfg.latency_cycles;
                    inst.watchdog_cb = nullptr; 

                    inst.regs->FLTCR1 &= ~DFSDM_FLTCR1_DFEN;

                    inst.regs->FLTCR1 = cfg.init_data.FLTCR1;
                    inst.regs->FLTCR2 = cfg.init_data.FLTCR2;
                    inst.regs->FLTFCR = cfg.init_data.FLTFCR;
                    
                    //clean flags
                    inst.regs->FLTISR = 0xFFFFFFFFu;
                    //enable the filter
                    inst.regs->FLTCR1 |= DFSDM_FLTCR1_DFEN;
                    switch(i){
                        case 0:  NVIC_EnableIRQ(DFSDM1_FLT0_IRQn);     break;
                        case 1:  NVIC_EnableIRQ(DFSDM1_FLT1_IRQn);     break;
                        case 2: NVIC_EnableIRQ(DFSDM1_FLT2_IRQn);      break;
                        case 3: NVIC_EnableIRQ(DFSDM1_FLT3_IRQn);      break;
                    }
                    
                }
            //activate NVIC for every filter
            
        }
    };
};
};



























struct DFSDM_CLK_DOMAIN{
   
    static constexpr GPIODomain::Pin valid_clk_pins[] = {
            {GPIODomain::Port::C, 2}, 
            {GPIODomain::Port::B, 0}, 
            {GPIODomain::Port::E, 9}, 
            {GPIODomain::Port::D, 3},
            {GPIODomain::Port::D, 10}
    };
    static consteval void is_valid_dfsdm_clk_pin(const GPIODomain::Port port, uint32_t pin) {
        bool found = false;
        for (auto &p : valid_clk_pins) {
            if (p.port == port && p.pin == pin) {
                found = true;
                break;
            }
        }
        if (!found) {
            compile_error("This pin cannot be used as DFSDM CLK OUT");
        }
    }

    static consteval GPIODomain::AlternateFunction dfsdm_clk_af(const GPIODomain::Pin& pin) {
        if (pin.port == GPIODomain::Port::C && pin.pin == 2)
            return GPIODomain::AlternateFunction::AF4;
        return GPIODomain::AlternateFunction::AF6; //In every other case
    }
    struct Entry{
        size_t gpio_idx;
        uint8_t clk_divider;
    };
    
    struct DFSDM_CLK{
        using domain = DFSDM_CLK_DOMAIN;
        GPIODomain::GPIO gpio;
        Entry e;
        uint8_t clk_divider;
        consteval DFSDM_CLK(const GPIODomain::Pin &pin,uint8_t clk_divider = 4):
        gpio{pin,GPIODomain::OperationMode::ALT_PP,GPIODomain::Pull::None, GPIODomain::Speed::VeryHigh,dfsdm_clk_af(pin)},
        clk_divider(clk_divider)
        {
            is_valid_dfsdm_clk_pin(pin.port,pin.pin);
            if(clk_divider < 4){
                compile_error("The divider must be at least frequency/4");
            }
        }
        template<class Ctx> consteval std::size_t inscribe(Ctx &ctx) const{
            const auto gpio_idx = gpio.inscribe(ctx);
            Entry e{.gpio_idx = gpio_idx,.clk_divider = clk_divider};
            return ctx.template add<DFSDM_CLK_DOMAIN>(e,this);
        }
        static constexpr std::size_t max_instances{1};
        struct Config{
         size_t gpio_idx;
         uint8_t clk_divider;
        };
        template <size_t N>
        static consteval std::array<Config, N> build(std::span<const Entry> entries) {
            std::array<Config, N> cfgs{};
            static_assert(N == 1,"You can't have more than one clock_out");
            for (std::size_t i = 0; i < N; ++i) {
                cfgs[i] = {
                    .gpio_idx = entries[i].gpio_idx,
                    .clk_divider = entries[i].clk_divider
                };
            }
            return cfgs;
        }
        struct Instance{
            GPIODomain::Instance *gpio_instance;
            uint8_t clk_divider;
            /*Already called in init()*/
            void init(){
                RCC->APB2ENR |= RCC_APB2ENR_DFSDM1EN; //Activate the DFSDM Clocki OUT in RCC by default it uses rcc_pclk2(80MH)
                //CKOUT = kernell clock (rcc_pclk2)
                //The channel 0 also allows to configure the CKOUT
                DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_CKOUTSRC;
                
                //CKOUT DivideR. Divider = CKOUTDIV + 1
                DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_CKOUTDIV;
               DFSDM1_Channel0->CHCFGR1 |= (clk_divider -1) << DFSDM_CHCFGR1_CKOUTDIV_Pos;
                
                //enable the CKOUT
                DFSDM1_Channel0->CHCFGR1 |= DFSDM_CHCFGR1_DFSDMEN;
            }
            bool disable(){
                DFSDM1_Channel0->CHCFGR1 &= ~DFSDM_CHCFGR1_DFSDMEN;
                return (DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN) == 0;
            }
            bool enable(){
                DFSDM1_Channel0->CHCFGR1 |=  DFSDM_CHCFGR1_DFSDMEN;
                return(DFSDM1_Channel0->CHCFGR1 & DFSDM_CHCFGR1_DFSDMEN);
            }
            bool change_divider(uint8_t div){
                if(div < 4) return false;
                clk_divider = div;
                if(disable()){
                    init();
                    return true;
                }
                return false;
            } 

        };
        template <std::size_t N>
        struct Init {
            static inline std::array<Instance, N> instances{};
            static void init(std::span<const Config, N> cfgs,std::span<GPIODomain::Instance> gpio_instances) {
                //add ckaie scdie
                const auto &c = cfgs[0];
                auto &inst = instances[0];
                inst.gpio = &gpio_instances[c.gpio_idx];
                inst.clk_divider = c.clk_divider;
                inst.init();
            }
        };
    };

};
};
// /*Only is going to use filter fastsync and from 1..3*/
            //bool choose_latency(uint32_t latency)
            //{
                //if (latency == 0) return false;

                //bool was_enabled = is_enabled();
                //if (was_enabled) disable();

                //uint32_t best_error = 0xFFFFFFFF;
                //uint32_t best_ford  = 0;
                //uint32_t best_fosr  = 0;
                //uint32_t best_iosr  = 0;

                //// Solo filtros 0..3
                //for (uint32_t ford = 0; ford <= 3; ++ford)
                //{
                    //for (uint32_t iosr = 1; iosr <= 256; ++iosr)
                    //{
                        //uint32_t denom = (iosr - 1 + ford);
                        //if (denom == 0) continue;

                        //uint32_t fosr = (latency > ford)
                                        //? (latency - ford) / denom
                                        //: 0;

                        //if (fosr < 1 || fosr > 1024) continue;

                        //uint32_t real_latency =
                            //fosr * denom + ford;

                        //uint32_t error =
                            //(real_latency > latency)
                            //? (real_latency - latency)
                            //: (latency - real_latency);

                        //if (error < best_error)
                        //{
                            //best_error = error;
                            //best_ford  = ford;
                            //best_fosr  = fosr;
                            //best_iosr  = iosr;

                            //if (error == 0) break;
                        //}
                    //}

                    //if (best_error == 0) break;
                //}

                //if (best_fosr == 0)
                //{
                    //if (was_enabled) enable();
                    //return false;
                //}

                //regs->FLTFCR &=
                    //~(DFSDM_FLTFCR_FORD_Msk |
                    //DFSDM_FLTFCR_FOSR_Msk |
                    //DFSDM_FLTFCR_IOSR_Msk);

                //regs->FLTFCR |= (best_ford << DFSDM_FLTFCR_FORD_Pos);
                //regs->FLTFCR |= ((best_fosr - 1) << DFSDM_FLTFCR_FOSR_Pos);
                //regs->FLTFCR |= ((best_iosr - 1) << DFSDM_FLTFCR_IOSR_Pos);

                //if (was_enabled) enable();
                //return true;
            //}

            //bool choose_latency(uint32_t latency,Filter_Type filter_type) {
                   //if (latency == 0) return false;
                   //uint32_t ford = static_cast<uint32_t>(filter_type);
                   //uint32_t oversampling_max = (ford <= 3) ? Oversampling_MAX : (ford == 4) ? Oversampling_MAX_Filter_4 : Oversampling_MAX_Filter_5;
                   //bool was_enabled = is_enabled();
                    
                    //uint32_t best_error = 0xFFFFFFFF;
                    //uint32_t best_fosr  = 0;
                    //uint32_t best_iosr  = 0;

                    //for (uint32_t iosr = 1; iosr <= 256; ++iosr)
                    //{
                        //uint32_t denom = (iosr - 1 + ford);
                        //if (denom == 0) continue;
                            //uint32_t fosr =
                              //(latency > ford)
                              //? (latency - ford) / denom
                              //: 0;

                          //if (fosr < 1 || fosr > oversampling_max) continue;

                          //uint32_t real_latency =
                              //fosr * denom + ford;

                          //uint32_t error =
                              //(real_latency > latency)
                              //? (real_latency - latency)
                              //: (latency - real_latency);

                          //if (error < best_error)
                          //{
                              //best_error = error;
                              //best_fosr  = fosr;
                              //best_iosr  = iosr;

                        //if (error == 0) break;
                    //}
                    //}

                    //if (best_fosr == 0) return false;
                    //if (was_enabled) disable();
                    //regs->FLTFCR &=
                            //~(DFSDM_FLTFCR_FORD_Msk |
                            //DFSDM_FLTFCR_FOSR_Msk |
                            //DFSDM_FLTFCR_IOSR_Msk);

                    //regs->FLTFCR |= (ford << DFSDM_FLTFCR_FORD_Pos);
                    //regs->FLTFCR |= ((best_fosr - 1) << DFSDM_FLTFCR_FOSR_Pos);
                    //regs->FLTFCR |= ((best_iosr - 1) << DFSDM_FLTFCR_IOSR_Pos);

                    //if (was_enabled) enable();
                    //return true;
                //}
            //bool choose_latency(uint32_t latency,Filter_Type filter_type, uint16_t oversampling){
                //if (latency == 0) return false;

                //uint32_t ford = static_cast<uint32_t>(filter_type);
                //if (ford > 3) return false;
                //uint32_t oversampling_max = (ford <= 3) ? Oversampling_MAX : (ford == 4) ? Oversampling_MAX_Filter_4 : Oversampling_MAX_Filter_5;
                //if (oversampling < 1 || oversampling > oversampling_max) return false;
                //bool was_enabled = is_enabled();

                //uint32_t best_error = 0xFFFFFFFF;
                //uint32_t best_iosr  = 0;

                //for (uint32_t iosr = 1; iosr <= 256; ++iosr)
                //{
                    //uint32_t real_latency =
                        //oversampling *
                        //(iosr - 1 + ford)
                        //+ ford;

                    //uint32_t error =
                        //(real_latency > latency)
                        //? (real_latency - latency)
                        //: (latency - real_latency);

                    //if (error < best_error)
                    //{
                        //best_error = error;
                        //best_iosr  = iosr;

                        //if (error == 0) break;
                    //}
                //}

                //if (best_iosr == 0) return false;
                //if(was_enabled) disable();
                //regs->FLTFCR &=
                    //~(DFSDM_FLTFCR_FORD_Msk |
                    //DFSDM_FLTFCR_FOSR_Msk |
                    //DFSDM_FLTFCR_IOSR_Msk);

                //regs->FLTFCR |= (ford << DFSDM_FLTFCR_FORD_Pos);
                //regs->FLTFCR |= ((oversampling - 1)
                                //<< DFSDM_FLTFCR_FOSR_Pos);
                //regs->FLTFCR |= ((best_iosr - 1)
                                //<< DFSDM_FLTFCR_IOSR_Pos);

                //if (was_enabled) enable();
                //return true;
            //}

