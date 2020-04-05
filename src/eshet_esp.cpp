#include "eshet_esp.hpp"
#include "esp_system.h"
#include "sdkconfig.h"

namespace eshet {

msgpack::object_handle get_id_from_mac() {
  std::array<uint8_t, 6> mac;
  esp_efuse_mac_get_default(mac.data());

  auto zone = std::make_unique<msgpack::zone>();
  msgpack::object obj(mac, *zone);
  return {obj, std::move(zone)};
}

std::pair<std::string, int> get_default_hostport() {
  return {CONFIG_ESHET_HOST, CONFIG_ESHET_PORT};
}

} // namespace eshet
