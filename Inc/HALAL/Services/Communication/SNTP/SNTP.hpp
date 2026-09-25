/*
 * SNTP.hpp
 *
 *  Created on: 21 feb. 2023
 *      Author: Ricardo
 *  Edited on: 22 sep. 2026
 *      by: Víctor
 */

#pragma once

// #include "lwip/apps/sntp.h"
#include "lwip/udp.h"
#include "HALAL/Models/IPV4/IPV4.hpp"
#include "C++Utilities/CppUtils.hpp"
#include "HALAL/Services/Time/Scheduler.hpp"

#ifndef STLIB_SNTP_PORT
/* default port is 8123 */
# define STLIB_SNTP_PORT 8123
#endif

namespace ST_LIB {

struct SNTP {
    /* config values for SNTP */

    static constexpr uint16_t PORT = STLIB_SNTP_PORT;
    static constexpr uint8_t MAX_SERVERS = 1;
    static constexpr uint32_t RETRY_TIMEOUT = 15000;
    static constexpr uint32_t RECV_TIMEOUT = 15000;
    static constexpr uint32_t UPDATE_DELAY = 3600000;

    /* to have a maximum amount of time SNTP is on for, we use retry_max and recv_max constants */

    static constexpr uint16_t REQUEST_MAX = 512;
    static constexpr uint16_t RECV_MAX = 8;

    static constexpr uint8_t OPMODE_POLL = 0;
    static constexpr uint8_t OPMODE_LISTENONLY = 1;

    /* chosen opmode */
    static constexpr uint8_t MODE = SNTP::OPMODE_POLL;

    /* SNTP specific values */

    /* kiss-of-death packet */
    static constexpr uint8_t ERR_KOD = 1;

    static constexpr uint16_t MSG_LEN = 48;

    /* NOTE: We don't care about leap seconds so we'll use no-warning */
    static constexpr uint8_t LEAP_INDICATOR_NO_WARNING = 0x00 << 6;
    static constexpr uint8_t LEAP_INDICATOR_LAST_MINUTE_61_SEC = 0x01 << 6;
    static constexpr uint8_t LEAP_INDICATOR_LAST_MINUTE_59_SEC = 0x02 << 6;
    static constexpr uint8_t LEAP_INDICATOR_ALARM_CONDITION = 0x03 << 6;

    /* SNTP Version 4 */
    static constexpr uint8_t VERSION = 4 << 3;

    static constexpr uint8_t MODE_MASK = 0x07;
    static constexpr uint8_t MODE_CLIENT = 0x03;
    static constexpr uint8_t MODE_SERVER = 0x04;
    static constexpr uint8_t MODE_BROADCAST = 0x05;

    static constexpr uint8_t OFFSET_STRATUM = 1;
    static constexpr uint8_t STRATUM_KOD = 0x00;

    static constexpr uint8_t OFFSET_TRANSMIT_TIME = 40;

    struct Server {
#if SNTP_SERVER_DNS
        const char* name;
#endif /* SNTP_SERVER_DNS */
        ip_addr_t addr;
#if SNTP_MONITOR_SERVER_REACHABILITY
        /** Reachability shift register as described in RFC 5905 */
        u8_t reachability;
#endif /* SNTP_MONITOR_SERVER_REACHABILITY */
    };

    /**
     * 64-bit NTP timestamp, in network byte order.
     */
    struct Time {
        uint32_t sec;
        uint32_t frac;
    };

    struct Timestamps {
#if SNTP_COMP_ROUNDTRIP || SNTP_CHECK_RESPONSE >= 2
        SNTP::Time orig;
        SNTP::Time recv;
#endif
        SNTP::Time xmit;
    };

#pragma pack(push, 1)
    struct Message {
        uint8_t li_vn_mode;
        uint8_t stratum;
        uint8_t poll;
        uint8_t precision;
        uint32_t root_delay;
        uint32_t root_dispersion;
        uint32_t reference_identifier;
        uint32_t reference_timestamp[2];
        uint32_t originate_timestamp[2];
        uint32_t receive_timestamp[2];
        uint32_t transmit_timestamp[2];
    };
#pragma pack(pop)

    static inline SNTP::Server servers[SNTP::MAX_SERVERS];

#if SNTP_SUPPORT_MULTIPLE_SERVERS
    /** The currently used server (initialized to 0) */
    static inline uint8_t sntp_current_server;
    static inline void try_next_server(const ip_addr_t* server_addr) {
#error SNTP multiple servers not supported.
    }
#else  /* SNTP_SUPPORT_MULTIPLE_SERVERS */
    static constexpr uint8_t current_server = 0;
#endif /* SNTP_SUPPORT_MULTIPLE_SERVERS */

    /** The UDP pcb used by the SNTP client */
    static inline struct udp_pcb* pcb;
    static inline uint32_t retry_timeout = SNTP::RETRY_TIMEOUT;

    static void start(ip_addr_t address);
    static void start(const char* ip);
    static void start(
        uint8_t address_head,
        uint8_t address_second,
        uint8_t address_third,
        uint8_t address_last
    );
    static void stop(void);

    /* internals */

    static inline uint16_t request_task_id = Scheduler::INVALID_ID;
    static inline uint16_t try_next_server_task_id = Scheduler::INVALID_ID;
    static inline uint8_t opmode = SNTP::MODE;

    static inline uint16_t request_count = 0;
    static inline uint16_t recv_count = 0;

    static void initialize_request(SNTP::Message* req);
    static void send_request(const ip_addr_t* server_addr);
    static void retry(void);
    static void request(void);
    static void process(const SNTP::Timestamps* timestamps);

    static void
    recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port);

#if !SNTP_SUPPORT_MULTIPLE_SERVERS
    static constexpr void (*try_next_server)(void) = SNTP::retry;
#endif
}; // struct SNTP
}; // namespace ST_LIB

#if 0
static constexpr const char* DEFAULT_SERVER_IP = "192.168.0.9";
#endif
