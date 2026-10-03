release: minor
summary: Rework SNTP module in ST-LIB to use ST-LIB scheduler instead of Lwip one

example usage:

```cpp
/* ... */
int main(void) {
    MainBoard::init();

    led_instance = &MainBoard::instance_of<led>();
    auto eth_instance = &MainBoard::instance_of<eth>();

    Scheduler::register_task(200'000, []{ led_instance->toggle(); });

    ST_LIB::SNTP::start("192.168.0.9");

    Scheduler::register_task(1'000'000, []{
        RTCData data = Global_RTC::get_rtc_timestamp();
        INFO("RTC: %u; %uyr %umon %uday %uh %umin %us",
            data.counter, data.year, data.month, data.day,
            data.hour, data.minute, data.second);
    });

    while (1) {
        eth_instance->update();
        Scheduler::update();
        Diagnostics::Hub::flush();
    }
}
```
Led is not needed but it is to show the board is doing work.
