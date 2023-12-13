#include "CLI11.hpp"
#include "eshet.hpp"
#include "utils.hpp"
#include <sstream>
#include <string>

using namespace eshet;

struct error_message : public std::exception {
  error_message(std::string message) : message(std::move(message)) {}
  const char *what() const throw() { return message.c_str(); }
  std::string message;
};

/// check that a result is not an error, throwing otherwise
void check_result(const Success &r) {}
void check_result(const Error &r) { throw error_message(r.what()); }

int main(int argc, char **argv) {
  CLI::App app{"eshet CLI"};

  {
    CLI::App *call = app.add_subcommand("flash", "flash some firmware");
    std::string path;
    std::string fname;
    size_t chunk_size = 8 * 1024;
    call->add_option("path", path)->required();
    call->add_option("fname", fname)->required();
    call->add_option("-c,--chunksize", chunk_size)->capture_default_str();
    call->callback([&]() {
      std::ifstream firmware(fname, std::ios::binary);
      if (!firmware.is_open())
        throw error_message("could not open " + fname);

      ESHETClient client(get_host_port());

      std::vector<char> buf(std::istreambuf_iterator<char>(firmware), {});
      std::cout << "size: " << buf.size() << std::endl;

      Channel<Result> call_result;
      client.action_call_pack(path + "/begin", call_result, std::make_tuple());
      std::visit([](const auto &r) { check_result(r); }, call_result.read());

      for (size_t offset = 0; offset < buf.size(); offset += chunk_size) {
        size_t this_chunk_size = std::min(chunk_size, buf.size() - offset);
        std::cout << "chunk " << offset << "-" << offset + this_chunk_size
                  << std::endl;

        // XXX: raw_ref doesn't own its data, so bad things can happen if this
        // fails
        client.action_call_pack(path + "/write", call_result,
                                std::make_tuple(msgpack::type::raw_ref(
                                    buf.data() + offset, this_chunk_size)));
        std::visit([](const auto &r) { check_result(r); }, call_result.read());
      }

      client.action_call_pack(path + "/end", call_result, std::make_tuple());
      std::visit([](const auto &r) { check_result(r); }, call_result.read());

      return 0;
    });
  }

  {
    CLI::App *call =
        app.add_subcommand("mark_valid", "mark the current image as valid");
    std::string path;
    call->add_option("path", path)->required();
    call->callback([&]() {
      ESHETClient client(get_host_port());

      Channel<Result> call_result;
      client.action_call_pack(path + "/mark_valid", call_result,
                              std::make_tuple(true));
      std::visit([](const auto &r) { check_result(r); }, call_result.read());
      return 0;
    });
  }

  {
    CLI::App *call = app.add_subcommand(
        "mark_invalid",
        "mark the current image as invalid, reverting to the previous image");
    std::string path;
    call->add_option("path", path)->required();
    call->callback([&]() {
      ESHETClient client(get_host_port());

      Channel<Result> call_result;
      client.action_call_pack(path + "/mark_valid", call_result,
                              std::make_tuple(false));
      auto result = call_result.read();
      if (result == Result(Error("client_exited"))) {
        std::cerr << "got client_exited error as expected from restart\n";
        return 0;
      }
      std::visit([](const auto &r) { check_result(r); }, result);
      return 0;
    });
  }

  {
    CLI::App *call = app.add_subcommand("restart", "restart the device");
    std::string path;
    call->add_option("path", path)->required();
    call->callback([&]() {
      ESHETClient client(get_host_port());

      Channel<Result> call_result;
      client.action_call_pack(path + "/restart", call_result,
                              std::make_tuple());
      std::visit([](const auto &r) { check_result(r); }, call_result.read());
      return 0;
    });
  }

  app.require_subcommand(1);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    return app.exit(e);
  } catch (const error_message &e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
