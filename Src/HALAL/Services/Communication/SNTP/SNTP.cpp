/*
 * SNTP.cpp
 *
 *  Created on: 21 feb. 2023
 *      Author: Ricardo
 *  Edited on: 22 sep. 2026
 *      by: Víctor
 */

#include "HALAL/Services/Communication/SNTP/SNTP.hpp"
#include "HALAL/Services/Time/RTC.hpp"

/* This implementation uses Lwip code */

#define SUBSECONDS_PER_SECOND 32767
#define TRANSFORMATION_FACTOR (SUBSECONDS_PER_SECOND / 999999.0)

/* Start offset of the timestamps to extract from the SNTP packet */
#define SNTP_OFFSET_TIMESTAMPS (SNTP::OFFSET_TRANSMIT_TIME + 8 - sizeof(SNTP::Timestamps))

/* Number of seconds between 1970 and Feb 7, 2036 06:28:16 UTC (epoch 1) */
#define DIFF_SEC_1970_2036 ((u32_t)2085978496L)

#define SNTP_FRAC_TO_US(f) ((u32_t)(((u64_t)(f) * 1000000UL) >> 32))

#define SNTP_GET_SYSTEM_TIME_NTP(s, f)                                                             \
    do {                                                                                           \
        u32_t sec_, usec_;                                                                         \
        SNTP_GET_SYSTEM_TIME(sec_, usec_);                                                         \
        (s) = (s32_t)(sec_ - DIFF_SEC_1970_2036);                                                  \
        (f) = usec_ * 4295 - ((usec_ * 2143) >> 16) + 2147;                                        \
    } while (0)

#define SNTP_SET_SYSTEM_TIME_US(sec, us) stlib_sntp_set_time((sec), (us))
#define SNTP_GET_SYSTEM_TIME(sec, us)                                                              \
    do {                                                                                           \
        (sec) = stlib_sntp_get_rtc_seconds();                                                      \
        (us) = stlib_sntp_get_rtc_microseconds();                                                  \
    } while (0)

#define SNTP_SET_SYSTEM_TIME_NTP(s, f)                                                             \
    SNTP_SET_SYSTEM_TIME_US((u32_t)((s) + DIFF_SEC_1970_2036), SNTP_FRAC_TO_US(f))

namespace ST_LIB {

void SNTP::start(
    uint8_t address_head,
    uint8_t address_second,
    uint8_t address_third,
    uint8_t address_last
) {
    ip_addr_t address;
    IP_ADDR4(&address, address_head, address_second, address_third, address_last);
    SNTP::start(address);
}

void SNTP::start(const char* ip) {
    IPV4 target(ip);
    SNTP::start(target.address);
}

void SNTP::start(ip_addr_t address) {
    SNTP::servers[0].addr = address;

    if (SNTP::pcb == NULL) {
        SNTP::pcb = udp_new_ip_type(IPADDR_TYPE_ANY);
        LWIP_ASSERT("Failed to allocate udp pcb for sntp client", SNTP::pcb != NULL);
        if (SNTP::pcb != NULL) {
            udp_recv(SNTP::pcb, SNTP::recv, NULL);

            if (SNTP::opmode == SNTP::MODE) {
#if SNTP_STARTUP_DELAY
                request_task_id =
                    Scheduler::set_timeout((uint32_t)SNTP_STARTUP_DELAY_FUNC * 1000, SNTP::request);
#else
                SNTP::request();
#endif
            } else if (SNTP::opmode == SNTP::OPMODE_LISTENONLY) {
                ip_set_option(SNTP::pcb, SOF_BROADCAST);
                udp_bind(SNTP::pcb, IP_ANY_TYPE, SNTP::PORT);
            }
        }
    }
}

void SNTP::stop(void) {
    if (SNTP::pcb != NULL) {
#if SNTP_MONITOR_SERVER_REACHABILITY
        for (uint8_t i = 0; i < SNTP_MAX_SERVERS; i++) {
            SNTP::servers[i].reachability = 0;
        }
#endif
        Scheduler::cancel_timeout(SNTP::request_task_id);
        Scheduler::cancel_timeout(SNTP::try_next_server_task_id);
        udp_remove(SNTP::pcb);
        SNTP::pcb = NULL;
    }
}

void SNTP::initialize_request(SNTP::Message* req) {
    memset(req, 0, SNTP::MSG_LEN);
    req->li_vn_mode = SNTP::LEAP_INDICATOR_NO_WARNING | SNTP::VERSION | SNTP::MODE_CLIENT;

    {
        s32_t secs;
        uint32_t sec, frac;
        /* Get the transmit timestamp */
        SNTP_GET_SYSTEM_TIME_NTP(secs, frac);
        sec = lwip_htonl((uint32_t)secs);
        frac = lwip_htonl(frac);

#if SNTP_CHECK_RESPONSE >= 2
        sntp_last_timestamp_sent.sec = sec;
        sntp_last_timestamp_sent.frac = frac;
#endif
        req->transmit_timestamp[0] = sec;
        req->transmit_timestamp[1] = frac;
    }
}

void SNTP::send_request(const ip_addr_t* server_addr) {
    struct pbuf* p;

    LWIP_ASSERT("server_addr != NULL", server_addr != NULL);

    p = pbuf_alloc(PBUF_TRANSPORT, SNTP::MSG_LEN, PBUF_RAM);
    if (p != NULL) {
        SNTP::Message* sntpmsg = (SNTP::Message*)p->payload;
        SNTP::initialize_request(sntpmsg);
        /* send request */
        udp_sendto(SNTP::pcb, p, server_addr, SNTP::PORT);
        /* free the pbuf after sending it */
        pbuf_free(p);
#if SNTP_MONITOR_SERVER_REACHABILITY
        /* indicate new packet has been sent */
        SNTP::servers[SNTP::current_server].reachability <<= 1;
#endif /* SNTP_MONITOR_SERVER_REACHABILITY */
        /* set up receive timeout: try next server or retry on timeout */
        SNTP::try_next_server_task_id =
            Scheduler::set_timeout((uint32_t)SNTP::RECV_TIMEOUT * 1000, SNTP::try_next_server);
#if SNTP_CHECK_RESPONSE >= 1
        /* save server address to verify it in sntp_recv */
        ip_addr_copy(sntp_last_server_address, *server_addr);
#endif /* SNTP_CHECK_RESPONSE >= 1 */
    } else {
        /* out of memory: set up a timer to send a retry */
        SNTP::request_task_id =
            Scheduler::set_timeout((uint32_t)SNTP::RETRY_TIMEOUT * 1000, SNTP::request);
    }
}

void SNTP::retry(void) {
    /* set up a timer to send a retry and increase the retry delay */
    SNTP::request_task_id = Scheduler::set_timeout(SNTP::retry_timeout * 1000, SNTP::request);

#if SNTP_RETRY_TIMEOUT_EXP
    {
        uint32_t new_retry_timeout;
        /* increase the timeout for next retry */
        new_retry_timeout = SNTP::retry_timeout << 1;
        /* limit to maximum timeout and prevent overflow */
        if ((new_retry_timeout <= SNTP_RETRY_TIMEOUT_MAX) &&
            (new_retry_timeout > SNTP::retry_timeout)) {
            SNTP::retry_timeout = new_retry_timeout;
        }
    }
#endif /* SNTP_RETRY_TIMEOUT_EXP */
}

void SNTP::request(void) {
    ip_addr_t sntp_server_address;
    err_t err;

#if SNTP_SERVER_DNS
    if (SNTP::servers[SNTP::current_server].name) {
        /* always resolve the name and rely on dns-internal caching & timeout */
        ip_addr_set_zero(&SNTP::servers[SNTP::current_server].addr);
        err = dns_gethostbyname(
            SNTP::servers[SNTP::current_server].name,
            &sntp_server_address,
            SNTP::dns_found,
            NULL
        );
        if (err == ERR_INPROGRESS) {
            /* DNS request sent, wait for sntp_dns_found being called */
            return;
        } else if (err == ERR_OK) {
            SNTP::servers[SNTP::current_server].addr = sntp_server_address;
        }
    } else
#endif /* SNTP_SERVER_DNS */
    {
        sntp_server_address = SNTP::servers[SNTP::current_server].addr;
        err = (ip_addr_isany_val(sntp_server_address)) ? ERR_ARG : ERR_OK;
    }

    if (err == ERR_OK) {
        SNTP::send_request(&sntp_server_address);
    } else {
        /* address conversion failed, try another server */
        SNTP::try_next_server_task_id =
            Scheduler::set_timeout((uint32_t)SNTP::RETRY_TIMEOUT * 1000, SNTP::try_next_server);
    }
}

void SNTP::process(const SNTP::Timestamps* timestamps)
{
    s32_t sec;
    u32_t frac;

    sec = (s32_t)lwip_ntohl(timestamps->xmit.sec);
    frac = lwip_ntohl(timestamps->xmit.frac);

#if SNTP_COMP_ROUNDTRIP
#if SNTP_CHECK_RESPONSE >= 2
    if (timestamps->recv.sec != 0 || timestamps->recv.frac != 0)
#endif
    {
        s32_t dest_sec;
        u32_t dest_frac;
        u32_t step_sec;

        /* Get the destination time stamp, i.e. the current system time */
        SNTP_GET_SYSTEM_TIME_NTP(dest_sec, dest_frac);

        step_sec =
            (dest_sec < sec) ? ((u32_t)sec - (u32_t)dest_sec) : ((u32_t)dest_sec - (u32_t)sec);
        /* In order to avoid overflows, skip the compensation if the clock step
         * is larger than about 34 years. */
        if ((step_sec >> 30) == 0) {
            s64_t t1, t2, t3, t4;

            t4 = SNTP_SEC_FRAC_TO_S64(dest_sec, dest_frac);
            t3 = SNTP_SEC_FRAC_TO_S64(sec, frac);
            t1 = SNTP_TIMESTAMP_TO_S64(timestamps->orig);
            t2 = SNTP_TIMESTAMP_TO_S64(timestamps->recv);
            /* Clock offset calculation according to RFC 4330 */
            t4 += ((t2 - t1) + (t3 - t4)) / 2;

            sec = (s32_t)((u64_t)t4 >> 32);
            frac = (u32_t)((u64_t)t4);
        }
    }
#endif /* SNTP_COMP_ROUNDTRIP */

    SNTP_SET_SYSTEM_TIME_NTP(sec, frac);
    LWIP_UNUSED_ARG(frac); /* might be unused if only seconds are set */
}

void SNTP::recv(void* arg, struct udp_pcb* pcb, struct pbuf* p, const ip_addr_t* addr, u16_t port) {
    SNTP::Timestamps timestamps;
    uint8_t mode;
    uint8_t stratum;
    err_t err;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(pcb);

    err = ERR_ARG;
#if SNTP_CHECK_RESPONSE >= 1
    /* check server address and port */
    if (((SNTP::opmode != SNTP::OPMODE_POLL) || ip_addr_cmp(addr, &sntp_last_server_address)) &&
        (port == SNTP::PORT))
#else  /* SNTP_CHECK_RESPONSE >= 1 */
    LWIP_UNUSED_ARG(addr);
    LWIP_UNUSED_ARG(port);
#endif /* SNTP_CHECK_RESPONSE >= 1 */
    {
        /* process the response */
        if (p->tot_len == SNTP::MSG_LEN) {
            mode = pbuf_get_at(p, 0) & SNTP::MODE_MASK;
            /* if this is a SNTP response... */
            if (((SNTP::opmode == SNTP::OPMODE_POLL) && (mode == SNTP::MODE_SERVER)) ||
                ((SNTP::opmode == SNTP::OPMODE_LISTENONLY) && (mode == SNTP::MODE_BROADCAST))) {
                stratum = pbuf_get_at(p, SNTP::OFFSET_STRATUM);

                if (stratum == SNTP::STRATUM_KOD) {
                    /* Kiss-of-death packet. Use another server or increase UPDATE_DELAY. */
                    err = SNTP::ERR_KOD;
                } else {
                    pbuf_copy_partial(p, &timestamps, sizeof(timestamps), SNTP_OFFSET_TIMESTAMPS);
#if SNTP_CHECK_RESPONSE >= 2
                    /* check originate_timetamp against sntp_last_timestamp_sent */
                    if (timestamps.orig.sec != sntp_last_timestamp_sent.sec ||
                        timestamps.orig.frac != sntp_last_timestamp_sent.frac) {
                        // Invalid originate timestamp in response (ignore)
                    } else
#endif /* SNTP_CHECK_RESPONSE >= 2 */
                    /* @todo: add code for SNTP_CHECK_RESPONSE >= 3 and >= 4 here */
                    {
                        /* correct answer */
                        err = ERR_OK;
                    }
                }
            } else {
                // Invalid mode in response `mode`
                /* wait for correct response */
                err = ERR_TIMEOUT;
            }
        } else {
            // Invalid packet length `p->tot_len` (ignore)
        }
    }
#if SNTP_CHECK_RESPONSE >= 1
    else {
        /* packet from wrong remote address or port, wait for correct response */
        err = ERR_TIMEOUT;
    }
#endif /* SNTP_CHECK_RESPONSE >= 1 */

    pbuf_free(p);

    if (err == ERR_OK) {
        /* correct packet received: process it it */
        SNTP::process(&timestamps);

#if SNTP_MONITOR_SERVER_REACHABILITY
        /* indicate that server responded */
        SNTP::servers[SNTP::current_server].reachability |= 1;
#endif /* SNTP_MONITOR_SERVER_REACHABILITY */
        /* Set up timeout for next request (only if poll response was received)*/
        if (SNTP::opmode == SNTP::OPMODE_POLL) {
            u32_t sntp_update_delay;
            Scheduler::cancel_timeout(SNTP::try_next_server_task_id);
            Scheduler::cancel_timeout(SNTP::request_task_id);

            /* Correct response, reset retry timeout */
            SNTP::retry_timeout = SNTP::RETRY_TIMEOUT;

            sntp_update_delay = (u32_t)SNTP::UPDATE_DELAY;
            SNTP::request_task_id = Scheduler::set_timeout(sntp_update_delay, SNTP::request);
        }
    } else if (err == SNTP::ERR_KOD) {
        /* KOD errors are only processed in case of an explicit poll response */
        if (SNTP::opmode == SNTP::OPMODE_POLL) {
            /* Kiss-of-death packet. Use another server or increase UPDATE_DELAY. */
            SNTP::try_next_server();
        }
    } else {
        /* ignore any broken packet, poll mode: retry after timeout to avoid flooding */
    }
}

}; // namespace ST_LIB

extern "C" void stlib_sntp_set_rtc(
    uint16_t counter,
    uint8_t second,
    uint8_t minute,
    uint8_t hour,
    uint8_t day,
    uint8_t month,
    uint16_t year
) {
    Global_RTC::set_rtc_data(counter, second, minute, hour, day, month, year);
}

extern "C" uint32_t stlib_sntp_get_rtc_seconds() {
    if (!Global_RTC::has_valid_time()) {
        return 0;
    }

    const RTCData rtc_time = Global_RTC::get_rtc_timestamp();
    time_t nowtime = 0;
    struct tm* nowtm;
    nowtm = gmtime(&nowtime);
    nowtm->tm_year = rtc_time.year - 1900;
    nowtm->tm_mon = rtc_time.month - 1;
    nowtm->tm_mday = rtc_time.day;
    nowtm->tm_hour = rtc_time.hour;
    nowtm->tm_min = rtc_time.minute;
    nowtm->tm_sec = rtc_time.second;
    uint32_t sec = mktime(nowtm);
    return sec;
}

extern "C" uint32_t stlib_sntp_get_rtc_microseconds() {
    if (!Global_RTC::has_valid_time()) {
        return 0;
    }

    const RTCData rtc_time = Global_RTC::get_rtc_timestamp();
    return rtc_time.counter / TRANSFORMATION_FACTOR;
}

extern "C" void stlib_sntp_set_time(uint32_t sec, uint32_t us) {
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = us;
    time_t nowtime = sec;
    struct tm* nowtm = localtime(&nowtime);
    uint32_t subsecond = (uint32_t)(TRANSFORMATION_FACTOR * tv.tv_usec);
    stlib_sntp_set_rtc(
        subsecond,
        nowtm->tm_sec,
        nowtm->tm_min,
        nowtm->tm_hour,
        nowtm->tm_mday,
        1 + nowtm->tm_mon,
        1900 + (nowtm->tm_year)
    );
}
