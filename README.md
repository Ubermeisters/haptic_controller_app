# haptic_controller_app
This is an application to control the TI DRV2506L haptic motor driver. It includes the beginnings of an extremely simple RTOS.

The application begins in "preset" state, where it plays a haptic effect from the driver's library once a second.

A brief press of a button will cycle to the next haptic effect, looping back to the start when the end is reached.

A long press of a button will cycle to "record" state, where for the next 10 seconds, button presses will be recorded as pulses.

After 10 seconds, the state will transition to "playback" state, where the pulses recorded in "record" state will be played back over the LED and motor driver.

After the playback is complete, the application's state will transition back to "preset" mode.
