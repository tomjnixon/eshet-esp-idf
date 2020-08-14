#pragma once
#include "eshet.hpp"
#include <esp_ota_ops.h>

namespace eshet {
using namespace actorpp;

class OTAHanderActor : public Actor {
public:
  OTAHanderActor(ESHETClient &client, const std::string &base)
      : begin_chan(*this), write_chan(*this), end_chan(*this),
        mark_valid_chan(*this), exit_chan(*this) {
    Channel<Result> result_chan(*this);

    client.action_register(base + "/begin", result_chan, begin_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));

    client.action_register(base + "/write", result_chan, write_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));

    client.action_register(base + "/end", result_chan, end_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));

    client.action_register(base + "/mark_valid", result_chan, mark_valid_chan);
    assert(std::holds_alternative<Success>(result_chan.read()));
  }

  void run() {
    while (true) {
      switch (
          wait(exit_chan, begin_chan, write_chan, end_chan, mark_valid_chan)) {
      case 0: {
        return;
      } break;
      case 1: {
        auto call = begin_chan.read();

        update_partition = esp_ota_get_next_update_partition(NULL);
        assert(update_partition != NULL);

        esp_err_t err =
            esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
        assert(err == ESP_OK);

        // XXX: return write partition so that we can query it to check if it's
        // rebooted into it before saving
        call.reply(Success());
      } break;
      case 2: {
        auto call = write_chan.read();
        msgpack::type::raw_ref data;
        call.convert(std::tie(data));

        esp_err_t err = esp_ota_write(update_handle, data.ptr, data.size);
        assert(err == ESP_OK);
        call.reply(Success());
      } break;
      case 3: {
        auto call = end_chan.read();

        esp_err_t err = esp_ota_end(update_handle);
        assert(err == ESP_OK);

        err = esp_ota_set_boot_partition(update_partition);
        assert(err == ESP_OK);

        call.reply(Success());
        vTaskDelay(500 / portTICK_PERIOD_MS);
        esp_restart();
      } break;
      case 4: {
        auto call = mark_valid_chan.read();
        bool valid;
        call.convert(std::tie(valid));

        if (valid) {
          esp_ota_mark_app_valid_cancel_rollback();
        } else {
          esp_ota_mark_app_invalid_rollback_and_reboot();
          // may never return
        }

        call.reply(Success());
      } break;
      }
    }
  }

  void exit() { exit_chan.push(true); }

private:
  Channel<Call> begin_chan;
  Channel<Call> write_chan;
  Channel<Call> end_chan;
  Channel<Call> mark_valid_chan;
  Channel<bool> exit_chan;

  esp_ota_handle_t update_handle;
  const esp_partition_t *update_partition;
};

using OTAHandler = ActorThread<OTAHanderActor>;
} // namespace eshet
