#include "CoreConfig.hpp"
#include "Exstension.hpp"
#include <amtl/os/am-path.h>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;

CoreConfig config;

void CoreConfig::Init() {
  char filePath[PLATFORM_MAX_PATH];

  ke::path::Format(filePath, sizeof(filePath), "%s/addons/sourcemod/configs/rpc.json", smutils->GetGamePath());

  std::ifstream t(filePath);
  auto begin = std::istreambuf_iterator<char>(t);
  std::string body(begin, std::istreambuf_iterator<char>());
  t.close();

  try {
    json config = json::parse(body);
    if (config["servers"].is_null()) {
      LOG_MESSAGE("WARNING - No servers configured");
    } else if (!config["servers"].is_object()) {
      LOG_MESSAGE("WARNING - Servers must be an object");
    } else {
      for (auto it = config["servers"].begin(); it != config["servers"].end(); ++it) {
        auto serverConfig = std::shared_ptr<ServerConfig>(new ServerConfig());

        auto value = it.value();
        auto key = it.key();

        if (!value["port"].is_null() && value["port"].is_number_integer()) {
          serverConfig->port = value["port"].get<int>();
        } else {
          LOG_MESSAGE("Warning - server %s has no port configured, skipping", it.key().c_str());
          continue;
        }

        if (!value["address"].is_null() && value["address"].is_string()) {
          serverConfig->address = value["address"].get<std::string>();
        } else {
          LOG_MESSAGE("Warning - server %s has no address configured, skipping", it.key().c_str());
          continue;
        }

        this->servers[it.key()] = serverConfig;
      }
    }

    if (!config["port"].is_null() && config["port"].is_number_integer()) {
      this->port = config["port"].get<int>();
    }

    if (!config["address"].is_null() && config["address"].is_string()) {
      this->address = config["address"].get<std::string>();
    }

    if (!config["secret"].is_null() && config["secret"].is_string()) {
      this->secret = config["secret"].get<std::string>();
    }
  } catch (std::invalid_argument e) {
    smutils->LogError(myself, "Error parsing config file %s", e.what());
  }
}