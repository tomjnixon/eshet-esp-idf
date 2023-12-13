#pragma once
#include "driver/gpio.h"
#include "eshet.hpp"

namespace eshet {
using namespace actorpp;

/// basic filter for GPIO inputs, requiring ticks_required ticks before
/// outputting a change
class GPIOFilter {
public:
  GPIOFilter(int ticks_required = 3) : ticks_required(ticks_required) {}

  void tick(bool input) {
    just_changed = false;

    if (ticks_with_input == -1) {
      state = last_input = input;
      ticks_with_input = ticks_required + 1;
      return;
    }

    if (input == last_input) {
      if (ticks_with_input <= ticks_required)
        ticks_with_input++;

      if (ticks_with_input >= ticks_required && state != input) {
        state = input;
        just_changed = true;
      }
    } else {
      ticks_with_input = 0;
    }

    last_input = input;
  }

  /// the current filtered state
  bool get_state() { return state; }

  /// did the last call to tick cause a state change?
  bool get_just_changed() { return just_changed; }

private:
  bool last_input = false;
  bool just_changed = false;
  bool state = false;
  int ticks_with_input = -1;
  int ticks_required;
};

/// make a state which contains a bool corresponding to the state of a GPIO,
/// with light filtering
class GPIOToStateFilterActor : public Actor {
public:
  GPIOToStateFilterActor(ESHETClient &client, const std::string &state,
                         int gpio, bool invert = false)
      : client(client), state(state), gpio(gpio), invert(invert) {
    Channel<Result> result_chan(*this);

    client.state_register(state, result_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));
  }

  void run() {
    Channel<Result> result_chan(*this);
    while (true) {
      filter.tick(input());

      if (first || filter.get_just_changed()) {
        client.state_changed(state, filter.get_state(), result_chan);
        first = false;
        while (result_chan.readable())
          assert(std::holds_alternative<Success>(result_chan.read()));
      }

      vTaskDelay(1);
    }
  }

  void exit() {}

private:
  bool input() { return invert ^ gpio_get_level((gpio_num_t)gpio); }

  ESHETClient &client;
  std::string state;
  int gpio;
  bool invert;
  bool first = true;

  GPIOFilter filter;
};

using GPIOToStateFilter = ActorThread<GPIOToStateFilterActor>;

/// make an event emits null on the rising (or falling if inverted) edge of a
/// GPIO, with light filtering
class GPIOToEventFilterActor : public Actor {
public:
  GPIOToEventFilterActor(ESHETClient &client, const std::string &event,
                         int gpio, bool invert = false)
      : client(client), event(event), gpio(gpio), invert(invert) {
    Channel<Result> result_chan(*this);

    client.event_register(event, result_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));
  }

  void run() {
    Channel<Result> result_chan(*this);
    while (true) {
      filter.tick(input());

      if (filter.get_just_changed() && filter.get_state()) {
        client.event_emit(event, msgpack::object(), result_chan);
        while (result_chan.readable())
          assert(std::holds_alternative<Success>(result_chan.read()));
      }

      vTaskDelay(1);
    }
  }

  void exit() {}

private:
  bool input() { return invert ^ gpio_get_level((gpio_num_t)gpio); }

  ESHETClient &client;
  std::string event;
  int gpio;
  bool invert;

  GPIOFilter filter;
};

using GPIOToEventFilter = ActorThread<GPIOToEventFilterActor>;

/// set a GPIO to the value of a state
///
/// if the valus is unknown, it will be set to default_state, unless
/// default_state is -1 in which case it will keep the last value
class StateToGPIOActor : public Actor {
public:
  StateToGPIOActor(ESHETClient &client, const std::string &state,
                   gpio_num_t gpio, int default_state = 0, bool invert = false)
      : client(client), gpio(gpio), default_state(default_state),
        invert(invert), on_state_change(*this) {

    ESP_ERROR_CHECK(gpio_set_direction(gpio, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(gpio, 0));

    {
      Channel<StateResult> result_chan(*this);
      client.state_observe(state, result_chan, on_state_change);
      handle_state_generic(result_chan.read());
    }
  }

  void run() {
    while (true)
      handle_state_generic(on_state_change.read());
  }

  void handle_state(const Known &k) { write(k.as<bool>()); }

  void handle_state(const Unknown &k) {
    if (default_state != -1)
      write(default_state);
  }

  // only required for initial value
  void handle_state(Error m) { assert(false); }

  template <typename T> void handle_state_generic(T m) {
    std::visit([this](auto mm) { handle_state(std::move(mm)); }, std::move(m));
  }

  void exit() {}

private:
  void write(bool value) {
    ESP_ERROR_CHECK(gpio_set_level(gpio, value ^ invert));
  }

  ESHETClient &client;
  gpio_num_t gpio;

  int default_state;
  bool invert;

  Channel<StateUpdate> on_state_change;
};

using StateToGPIO = ActorThread<StateToGPIOActor>;
} // namespace eshet
