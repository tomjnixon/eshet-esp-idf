A basic example exposing some GPIOs as ESHET states and events, using this
project together with
[esp_simple_wifi](https://github.com/tomjnixon/esp_simple_wifi) to setup wifi.

## flashing

- install and set up esp-idf

- edit `sdkconfig.defaults` to set the wifi credentials and ESHET server

- run `idf.py flash`

to monitor and re-flash, the most convenient way is to use `idf.py monitor`.

## OTA flashing

OTA flashing is set up in this project, but you'll need to install `eshet_ota`
-- see the main README for details.

## what does it do?

### /esp32/led_2

The example observes this state, which should contain a boolean value, and sets
the on-board led (pin 2 on my board) to correspond to it.

run this to publish `true` to light the LED:

    eshet publish /esp32/led_2 true

and type `false`/`true` to change it.

If you restart the board, the LED state should be correct once it connects.

### /esp32/input_21_state

The example publishes a boolean to this state, indicating whether pin 21 (which
has a pull-up enabled) is pulled low. Run:

    eshet observe /esp32/input_21_state

to see the current state, and short pin 21 to ground to see it update. Or, run:

    eshet get /esp32/input_21_state

to get the state once.

### /esp32/input_22_event

The example emits `null` from this event whenever pin 22 (which has pull-ups
enabled) is pulled low. Run:

    eshet listen /esp32/input_22_event

and short pin 22 to ground to see an event.

### /esp32/add_one and /esp32/sub_one

A couple of basic actions, to show the boilerplate required to expose something
with a pile of actions, which is often the easiest way to get started with some
new hardware:

    $ eshet call /esp32/add_one 5
    6
    $ eshet call /esp32/sub_one 6
    5

## use with the python examples

The [examples in
eshet.py](https://github.com/tomjnixon/eshet.py/tree/main/examples) can be used
to tie these together. For example, to make the LED reflect the input state:

    python examples/bind_state.py /esp32/input_21_state /esp32/led_2

Or to activate the LED on an input event:

    python examples/event_to_state_timer.py /esp32/input_22_event /esp32/led_2
