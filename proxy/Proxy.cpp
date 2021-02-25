#include <cstring>
#include <sstream>
#include <queue>
#include <unordered_map>

#include "looped_task.hpp"
#include "httplib.h"
#include "zmq.hpp"

using messages_queue = std::queue<std::string>;
using dpls_messages  = std::unordered_map<std::string, messages_queue>;

/* ADDRESSES */
constexpr char DPLS_BROADCASTER_ADDRESS[] = "ipc:///tmp/broker";
constexpr char PROXY_ADDRESS[]            = "ipc:///tmp/proxy";

/* ENDPOINTS */
constexpr char AVAILABLE_DEVICES_ENDPOINT[] = "/available-devices";
constexpr char INSPECTED_DATA_ENDPOINT[]    = "/inspected-data";
constexpr char STOP_INSPECTION_ENDPOINT[]   = "/stop";
constexpr char SELECT_DEVICES_ENDPOINT[]    = "/select-devices";
constexpr char SELECT_ALL_ENDPOINT[]        = "/select-all";

/* TIMEOUTS */
constexpr long DPLS_BROADCASTER_POLL_TIMEOUT = 1000;  
constexpr long PROXY_POLL_TIMEOUT            = 1000;

/* HTTP HANDLERS */
void get_inspected_data(
  const httplib::Request& input,
  httplib::Response& output,
  dpls_messages& messages
) {
  if (input.has_header("devices") && input.has_header("count")) {
    std::string devices = input.get_header_value("devices");
    int count = std::stoi(input.get_header_value("count"));

    std::stringstream response;
    std::string::size_type start = 0, delim = devices.find(",");
    while (true) {
      std::string::size_type end =
	delim == std::string::npos ? devices.size() : delim;
      std::string device = devices.substr(start, end - start);
      if (messages.find(device) != messages.end()) {
	for (int i = 0; i < count && messages.at(device).size(); i++) {
	  response << "," << std::move(messages.at(device).front());
	  messages[device].pop();
	}
      }
      if (delim == std::string::npos) {
	break;
      }
      start = end + 1, delim = devices.find(",", start);
    }
    std::string data = response.str();
    if (!data.empty()) {
      data = data.substr(1);
    }
    output.set_content("[" + data + "]", "application/json");
  }
}

void select_devices(
  const httplib::Request& input,
  httplib::Response& output,
  zmq::socket_t& dpls_broadcaster
) {
  if (input.has_header("devices")) {
    std::string devices = input.get_header_value("devices");

    zmq::message_t message{devices.length()};
    std::memcpy(message.data(), devices.c_str(), devices.length());
    dpls_broadcaster.send(message);
  }
}

/* OTHERS  */
void receive_dpls_subscriptions(
  zmq::socket_t& dpls_broadcaster,
  zmq::pollitem_t& poller,
  dpls_messages& messages
) {
  zmq::poll(&poller, 1, DPLS_BROADCASTER_POLL_TIMEOUT);
  if (poller.revents & ZMQ_POLLIN) {
    zmq::message_t buffer;
    dpls_broadcaster.recv(&buffer);
    std::string message;
    message.assign(static_cast<char*>(buffer.data()), buffer.size());
    if (message.length() > 0 && message[0] != 0x01 && message[0] != 0x00) {
      if (messages.find(message) == messages.end()) {
	messages.emplace(message, messages_queue {});
      }
    }
  }
}

void pull_inspected_data(
  zmq::socket_t& puller,
  zmq::pollitem_t& poller,
  dpls_messages& messages
) {
  zmq::poll(&poller, 1, PROXY_POLL_TIMEOUT);
  if (poller.revents & ZMQ_POLLIN) {
    zmq::message_t buffer;
    puller.recv(&buffer);
    std::string data;
    data.assign(static_cast<char*>(buffer.data()), buffer.size());
    std::string::size_type start = data.find(':'), end = data.find(',');
    if (start != std::string::npos && end != std::string::npos) {
      std::string sender = data.substr(start + 2, end - start - 3);
      messages[sender].emplace(std::move(data));
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Please provide address and port" << std::endl;
  }
  
  const char* address = argv[1];
  int port = std::stoi(argv[2]);

  zmq::context_t context{1};
  zmq::socket_t puller{context, ZMQ_PULL};
  puller.bind(PROXY_ADDRESS);

  dpls_messages messages;

  zmq::pollitem_t poller{static_cast<void*>(puller), 0, ZMQ_POLLIN, 0};
  looped_task pulling{
    pull_inspected_data,
    std::move(puller),
    std::move(poller),
    messages
  };

  zmq::socket_t dpls_broadcaster{context, ZMQ_XPUB};
  dpls_broadcaster.bind(DPLS_BROADCASTER_ADDRESS);

  zmq::pollitem_t dpls_poller{static_cast<void*>(dpls_broadcaster), 0, ZMQ_POLLIN, 0};
  looped_task receiving_subscriptions{
    receive_dpls_subscriptions,
    std::ref(dpls_broadcaster),
    std::move(dpls_poller),
    std::ref(messages)
  };

  httplib::Server handle;

  handle.Get(AVAILABLE_DEVICES_ENDPOINT,
    [&messages](const httplib::Request& input, httplib::Response& output) {
      std::string joined_names;
      for (const auto& [name, messages_] : messages) {
	joined_names += name + "\n";
      }
      joined_names.pop_back();
      output.set_content(joined_names, "text/plain");
    }
  );
  handle.Get(INSPECTED_DATA_ENDPOINT,
    [&messages](const httplib::Request& input, httplib::Response& output) {
      get_inspected_data(input, output, messages);
    }
  );
  handle.Get(STOP_INSPECTION_ENDPOINT,
    [&handle](const httplib::Request& input, httplib::Response& output) {
       handle.stop();
    }
  );
  handle.Get(SELECT_DEVICES_ENDPOINT,
    [&dpls_broadcaster](const httplib::Request& input, httplib::Response& output) {
      select_devices(input, output, dpls_broadcaster);
    }
  );
  handle.Get(SELECT_ALL_ENDPOINT,
    [&messages, &dpls_broadcaster](const httplib::Request& input, httplib::Response& output) {
      std::string devices;
      for (const auto& [name, messages_] : messages) {
	devices += name + ",";
      }
      devices.pop_back();
      httplib::Request input_with_all_devices {input};
      input_with_all_devices.set_header("devices", devices);
      select_devices(input_with_all_devices, output, dpls_broadcaster);
    }
  );
  handle.Options("/(.*)",
    [&](const httplib::Request& input, httplib::Response& output) {
      output.set_header("Access-Control-Allow-Methods", " POST, GET, OPTIONS");
      output.set_header("Content-Type", "text/html; charset=utf-8");
      output.set_header("Access-Control-Allow-Headers", "X-Requested-With, Content-Type, Accept");
      output.set_header("Access-Control-Allow-Origin", "*");
      output.set_header("Connection", "close");
  });

  std::cout << "STARTING PROXY ON " << address << ":" << port << std::endl;
  handle.listen(address, port);
}
