#include "eshet.hpp"
#include "eshet_esp.hpp"
#include "eshet_esp/eshet_gpio.hpp"
#include "eshet_ota.hpp"
#include "simple_wifi.hpp"

using namespace actorpp;
using namespace eshet;

/// example actor responding to a couple of actions
class ExampleActionActor : public Actor {
public:
  ExampleActionActor(ESHETClient &client, const std::string &base)
      : add_chan(*this), sub_chan(*this) {
    Channel<Result> result_chan(*this);

    client.action_register(base + "/add_one", result_chan, add_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));

    client.action_register(base + "/sub_one", result_chan, sub_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));
  }

  void run() {
    while (true) {
      switch (wait(add_chan, sub_chan)) {
      case 0: {
        auto call = add_chan.read();
        int x;
        call.convert(std::tie(x));
        call.reply(Success(x + 1));
      } break;
      case 1: {
        auto call = sub_chan.read();
        int x;
        call.convert(std::tie(x));
        call.reply(Success(x - 1));
      } break;
      }
    }
  }

  void exit() {}

private:
  Channel<Call> add_chan;
  Channel<Call> sub_chan;
};

using ExampleAction = ActorThread<ExampleActionActor>;

extern "C" void app_main() {
  simple_wifi::start();
  ESHETClient client(get_default_hostport(), get_id_from_mac());
  OTAHandler ota(client, ESHET_OTA_PATH);

  StateToGPIO out(client, "/esp32/led_2", GPIO_NUM_2);

  ESP_ERROR_CHECK(gpio_pullup_en(GPIO_NUM_21));
  GPIOToStateFilter in_state(client, "/esp32/input_21_state", GPIO_NUM_21,
                             true);

  ESP_ERROR_CHECK(gpio_pullup_en(GPIO_NUM_22));
  GPIOToEventFilter in_event(client, "/esp32/input_22_event", GPIO_NUM_22,
                             true);

  ExampleAction example_action(client, "/esp32");

  while (1)
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
