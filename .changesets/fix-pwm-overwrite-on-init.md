release: patch
summary: Fix PWM duty cycle being overwritten with 0 on init by computing .Pulse in timer ticks from the stored duty cycle

PWM::init() used to pass Pulse = 0 to config_output_compare_channel, which overwrote CCRx after the duty cycle had been set. Added get_ticks_duty_cycle() to convert the duty cycle percentage into timer ticks and use it as Pulse.
