#pragma once

#ifndef __SERDEPP_ADAOTOR_TOML11_HPP__
#define __SERDEPP_ADAOTOR_TOML11_HPP__

#include <toml.hpp>
#include "serdepp/serializer.hpp"


namespace serde {
    using toml_v = toml::value;
    template<> struct serde_adaptor_helper<toml_v>: derive_serde_adaptor_helper<toml_v> {
        inline static bool is_null(toml_v& adaptor, std::string_view key) {
            return adaptor.is_table() ? !adaptor.contains(std::string{key}) : true;
        }
        inline static bool is_struct(toml_v& adaptor) {
            return adaptor.is_table();
        }
        inline static size_t size(toml_v& adaptor) {
            return adaptor.size();
        }
        static toml_v parse_file(const std::string& path) { return toml::parse(path); }
    };

    template<typename T> struct serde_adaptor<toml_v, T, type::struct_t> {
        inline static void from(toml_v& s, std::string_view key, T& data) {
            deserialize_to<T>(toml::find(s, std::string{key}), data);
        }
        inline static void into(toml_v &s, std::string_view key, const T& data) {
            s[std::string{key}] = serialize<toml_v>(data);
        } 
    };

    template<typename T> struct serde_adaptor<toml_v, T>  {
        inline constexpr static void from(toml_v& s, std::string_view key, T& data) {
            data = key.empty() ? toml::get<T>(s) : toml::find<T>(s, std::string{key});
        }
        inline constexpr static void into(toml_v& s, std::string_view key, const T& data) {
            if(key.empty()) { s = data; } else { s[key.data()] = data; }
        }
    };

    template<typename T> struct serde_adaptor<toml_v, T, type::seq_t> {
    using E = type::seq_e<T>;
        static void from(toml_v& s, std::string_view key, T& arr) {
            auto& table = key.empty() ? s : s[std::string{key}];
            if constexpr(is_arrayable_v<T>) arr.reserve(table.size());
            for(auto& value : table.as_array()) { arr.push_back(deserialize<E>(value)); }
        }
        static void into(toml_v& s, std::string_view key, const T& data) {
            toml::array arr;
            arr.reserve(data.size());
            if constexpr(type::is_struct_v<E>) {
                for(auto& value: data) { arr.push_back(std::move(serialize<toml_v>(value).as_table())); }
            } else {
                for(auto& value: data) { arr.push_back(std::move(serialize<toml_v>(value)));  }
            }
            (key.empty() ? s : s[std::string{key}]) = std::move(arr);
        }
    };

    template <typename Map> struct serde_adaptor<toml_v, Map, type::map_t> {
        using E = type::map_e<Map>;
        inline static void from(toml_v& s, std::string_view key, Map& map) {
            auto& table = key.empty() ? s : s[std::string{key}];
            for(auto& [key_, value_] : table.as_table()) { deserialize_to<E>(value_, map[key_]);}
        }
        inline static void into(toml_v& s, std::string_view key, const Map& data) {
            toml_v& map = key.empty() ? s : s[std::string{key}];
            for(auto& [key_, value] : data) { serialize_to<toml_v>(value, map[key_]); }
        }
    };
}

#endif
