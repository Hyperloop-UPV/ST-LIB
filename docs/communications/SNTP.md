# SNTP module in ST-LIB

The SNTP module has been reworked to use the ST-LIB scheduler instead of the Lwip scheduler. It will use a single task in the scheduler at most.

After starting SNTP with `SNTP::start(...)` it will retry at maximum 512 times and when it gets a valid response it will process a maximum of 8 responses and after it will call `SNTP::stop()` internally.

If you need to use SNTP again for whatever reason after it stops automatically, call `SNTP::start(...)` again and it will reset the counters for retry and response.

To change the SNTP port, `#define` STLIB_SNTP_PORT **before** `#include "ST-LIB.hpp"`

```cpp
static void start(ip_addr_t address);
static void start(const char* ip);
static void start(
    uint8_t address_head,
    uint8_t address_second,
    uint8_t address_third,
    uint8_t address_last
);
static void stop(void);
```
