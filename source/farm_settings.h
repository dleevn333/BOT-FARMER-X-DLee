#pragma once
#include <array>
#include <string>
#include <sstream>
#include <map>

// Only preferences are stored: never window handles, running state or timers.
struct FarmPreferences {
    bool sell = false;
    bool harvest = false;
    bool purpleName = false;
    bool buySeeds = false;
    bool buyTools = false;
    bool showVision = false;
    int harvestMode = 0;
    std::array<bool, 31> crops{};
    std::array<bool, 29> seeds{};
    std::array<bool, 8> tools{};
    bool plant = false;
    std::array<bool, 34> plantingSeeds{};
    bool weather = false;
    int weatherMap = -1;
};

template<size_t N>
inline std::string FarmEncodeChoices(const std::array<bool, N>& values) {
    std::string result;
    for (bool value : values) result += value ? '1' : '0';
    return result;
}

inline std::string FarmEncodePreferences(const FarmPreferences& p) {
    std::ostringstream out;
    out << "version=2\n"
        << "sell=" << p.sell << "\n"
        << "harvest=" << p.harvest << "\n"
        << "purple_name=" << p.purpleName << "\n"
        << "buy_seeds=" << p.buySeeds << "\n"
        << "buy_tools=" << p.buyTools << "\n"
        << "show_vision=" << p.showVision << "\n"
        << "harvest_mode=" << p.harvestMode << "\n"
        << "crops=" << FarmEncodeChoices(p.crops) << "\n"
        << "seeds=" << FarmEncodeChoices(p.seeds) << "\n"
        << "tools=" << FarmEncodeChoices(p.tools) << "\n"
        << "plant=" << p.plant << "\n"
        << "planting_seeds=" << FarmEncodeChoices(p.plantingSeeds) << "\n"
        << "weather=" << p.weather << "\n"
        << "weather_map=" << p.weatherMap << "\n";
    return out.str();
}

template<size_t N>
inline bool FarmDecodeChoices(const std::string& text, std::array<bool, N>& values) {
    if (text.size() != N || text.find_first_not_of("01") != std::string::npos) return false;
    for (size_t i = 0; i < N; ++i) values[i] = text[i] == '1';
    return true;
}

// Apply the complete document only after validation; malformed data cannot
// partially enable operations or replace the current user's selections.
inline bool FarmDecodePreferences(const std::string& text, FarmPreferences& result) {
    if (text.size() > 4096) return false;
    std::map<std::string, std::string> fields;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        size_t separator = line.find('=');
        if (separator == std::string::npos || separator == 0) return false;
        if (!fields.emplace(line.substr(0, separator), line.substr(separator + 1)).second) return false;
    }
    if (fields["version"] != "1" && fields["version"] != "2") return false;
    FarmPreferences p;
    auto boolean = [&](const char* key, bool& value) {
        auto found = fields.find(key);
        if (found == fields.end() || (found->second != "0" && found->second != "1")) return false;
        value = found->second == "1";
        return true;
    };
    if (!boolean("sell", p.sell) || !boolean("harvest", p.harvest)
        || !boolean("purple_name", p.purpleName) || !boolean("buy_seeds", p.buySeeds)
        || !boolean("buy_tools", p.buyTools) || !boolean("show_vision", p.showVision)
        || !boolean("weather", p.weather)) return false;
    const auto mode = fields["harvest_mode"];
    if (mode != "0" && mode != "1" && mode != "2") return false;
    p.harvestMode = mode[0] - '0';
    const auto map = fields["weather_map"];
    if (map != "-1" && map != "1" && map != "3" && map != "4" && map != "5") return false;
    p.weatherMap = map == "-1" ? -1 : map[0] - '0';
    if (!FarmDecodeChoices(fields["crops"], p.crops)
        || !FarmDecodeChoices(fields["seeds"], p.seeds)
        || !FarmDecodeChoices(fields["tools"], p.tools)) return false;
    if (fields["version"] == "2" && (!boolean("plant", p.plant)
        || !FarmDecodeChoices(fields["planting_seeds"], p.plantingSeeds))) return false;
    result = p;
    return true;
}

// Keep a short prefix plus a hash of the entire title: profile filenames are
// bounded, distinguish long instance names, and cannot contain path separators.
inline std::string FarmProfileKey(const std::string& title) {
    if (title.empty()) return "untitled";
    const char hex[] = "0123456789abcdef";
    std::string key;
    unsigned long long hash = 14695981039346656037ull;
    for (unsigned char c : title) {
        hash = (hash ^ c) * 1099511628211ull;
        if (key.size() < 32) {
            key += hex[c >> 4];
            key += hex[c & 15];
        }
    }
    key += '-';
    for (int shift = 60; shift >= 0; shift -= 4) key += hex[(hash >> shift) & 15];
    return key;
}
