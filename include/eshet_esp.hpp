#pragma once
#include "msgpack.hpp"

namespace eshet {
msgpack::object_handle get_id_from_mac();
std::pair<std::string, int> get_default_hostport();
} // namespace eshet
