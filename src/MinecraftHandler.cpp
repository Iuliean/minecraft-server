#include <SFW/LoggerManager.h>
#include <SFW/Connection.h>
#include <algorithm>
#include <bits/stdint-uintn.h>
#include <string>
#include <sys/types.h>
#include <fstream>


#include "MinecraftHandler.h"
#include "PlayerHandler.h"
#include "SFW/Serializer.h"
#include "nlohmann/json.hpp"
#include "DataTypes/Identifier.h"
#include "DataTypes/nbt.h"
#include "utils.h"

namespace mc
{

    namespace 
    {
        constexpr static int PACKET_SIZE = 1024;
    }

    MinecraftHanlder::MinecraftHanlder()
        : m_logger(iu::LoggerManager::GetLogger("McHandler")),
        m_stop(false)
    {
        BuildRegistryPackets();
    }

    void MinecraftHanlder::OnConnected(iu::Connection& connection)
    {
        m_logger.info("New connection from: {}:{}", connection.GetAdress(), connection.GetPort());
    }

    void MinecraftHanlder::HandleConnection(iu::Connection &connection)
    {
        PlayerHandler h(connection, m_context);
        std::vector<uint8_t> data;
        data.resize(PACKET_SIZE);
        std::stringstream ss;
        while(!m_stop)
        {
            size_t recv = connection.Receive(data, PACKET_SIZE);
            for(size_t i = 0; i < recv; ++i)
            {
                ss << std::hex << (int)data[i] << ", ";
            }

            m_logger.debug("{}",ss.str());
            ss.clear();
            ss.str("");
            if (recv == 0 )
                return;
            h.Execute(data);
        }
    }

    void MinecraftHanlder::Stop()
    {
        return;
    }

    //Private

    //These are semi hardcoded and inflexible for now in the name of progress
    void MinecraftHanlder::BuildRegistryPackets()
    {
        std::ifstream f("registry.json");
        nlohmann::json registryJson = nlohmann::json::parse(f);
        iu::Serializer<Identifier> identifierSerializer;
        auto removeNameBytesFromRootNBT = [] (std::vector<std::uint8_t>& out, const NBT::NBT& data)
        {
            std::vector<std::uint8_t> intermediate;
            NBT::NBTSerializer().Serialize(intermediate, data);
            intermediate.erase(intermediate.begin() + 1);
            intermediate.erase(intermediate.begin() + 1);
            out.insert(out.end(), intermediate.begin(), intermediate.end());
        };

        {
            std::vector<std::uint8_t>& packet = m_context.registry_packets[0];
            nlohmann::json trimPattern = registryJson["minecraft:trim_pattern"];

            identifierSerializer.Serialize(packet, Identifier("trim_pattern"));
            util::writeVarInt(packet, trimPattern["value"].size());

            for (const auto& value : trimPattern["value"])
            {
                util::writeStringToBuff(packet, value["name"].get<NBT::String>());
                util::ByteSerializer().Serialize(packet, 0);

                /*NBT::NBT data("");

                data->Insert("asset_id", value["element"]["asset_id"].get<NBT::String>());
                data->Insert("template_item", value["element"]["template_item"].get<NBT::String>());
                NBT::NBT description ("description");
                description->Insert("translate", value["element"]["description"]["translate"].get<NBT::String>());

                data->Insert(std::move(description));
                data->Insert("decal", value["element"]["decal"].get<NBT::Byte>());
                //Intermediate is a temporary solution since network NBT has name bytes removed from root nbt
                removeNameBytesFromRootNBT(packet, data);
                */

            }
            util::writeVarInt(packet, 0, 0x07);
            util::writeVarInt(packet, 0, packet.size());
        }
        {
            std::vector<std::uint8_t>& packet = m_context.registry_packets[1];
            nlohmann::json wolf_variant = registryJson["minecraft:wolf_variant"];

            identifierSerializer.Serialize(packet, Identifier("wolf_variant"));
            util::writeVarInt(packet, wolf_variant.size());

            for(const auto& [key , value] : wolf_variant.items())
            {
                util::writeStringToBuff(packet, key);
                util::ByteSerializer().Serialize(packet, 1);

                NBT::NBT data("");
                data->Insert("wild_texture", value["wild_texture"].get<NBT::String>());
                data->Insert("tame_texture", value["tame_texture"].get<NBT::String>());
                data->Insert("angry_texture", value["angry_texture"].get<NBT::String>());
                data->Insert("biomes", value["biomes"].get<NBT::String>());

                //Intermediate is a temporary solution since network NBT has name bytes removed from root nbt
                removeNameBytesFromRootNBT(packet, data);
            }
            util::writeVarInt(packet, 0, 0x07);
            util::writeVarInt(packet, 0, packet.size());
        }
        {
            std::vector<std::uint8_t>& packet = m_context.registry_packets[2];
            nlohmann::json worldgen_biome = registryJson["minecraft:worldgen/biome"];

            identifierSerializer.Serialize(packet, Identifier("worldgen/biome"));
            util::writeVarInt(packet,worldgen_biome.size());

            for(const auto& [key, value] : worldgen_biome.items())
            {
                util::writeStringToBuff(packet, key);
                util::ByteSerializer().Serialize(packet, 1);

                NBT::NBT data("");
                data->Insert("has_precipitation", value["has_precipitation"].get<NBT::Byte>());
                data->Insert("temperature", value["temperature"].get<NBT::Float>());
                data->Insert("downfall", value["downfall"].get<NBT::Float>());

                if(value.contains("temperature_modifier"))
                    data->Insert("temperature_modifier", value["temperature_modifier"].get<NBT::String>());
                NBT::NBT effects("effects");

                effects->Insert("fog_color", value["effects"]["fog_color"].get<NBT::Int>());
                effects->Insert("water_color", value["effects"]["water_color"].get<NBT::Int>());
                effects->Insert("water_fog_color", value["effects"]["water_fog_color"].get<NBT::Int>());
                effects->Insert("sky_color", value["effects"]["sky_color"].get<NBT::Int>());

                if(value["effects"].contains("foliage_color"))
                    effects->Insert("foliage_color", value["effects"]["foliage_color"].get<NBT::Int>());
                if(value["effects"].contains("grass_color"))
                    effects->Insert("grass_color", value["effects"]["grass_color"].get<NBT::Int>());
                if(value["effects"].contains("grass_color_modifier"))
                    effects->Insert("grass_color_modifier", value["effects"]["grass_color_modifier"].get<NBT::String>());
                if(value["effects"].contains("particle"))
                {
                    NBT::NBT particle ("particle");
                    NBT::NBT options("options");
                    options->Insert("type", value["effects"]["particle"]["options"]["type"].get<NBT::String>());

                    //This should probably take into account the optional value type at somepoint
                    particle->Insert("probability", value["effects"]["particle"]["probability"].get<NBT::Float>());
                    particle->Insert(std::move(options));

                    effects->Insert(std::move(particle));
                }
                //This takes in either compound or String. The default has only string so for now this is ok
                if(value["effects"].contains("ambient_sound"))
                    effects->Insert("ambient_sound", value["effects"]["ambient_sound"].get<NBT::String>());
                if(value["effects"].contains("mood_sound"))
                {
                    NBT::NBT mood_sound("mood_sound");

                    mood_sound->Insert("sound", value["effects"]["mood_sound"]["sound"].get<NBT::String>());
                    mood_sound->Insert("tick_delay", value["effects"]["mood_sound"]["tick_delay"].get<NBT::Int>());
                    mood_sound->Insert("block_search_extent", value["effects"]["mood_sound"]["block_search_extent"].get<NBT::Int>());
                    mood_sound->Insert("offset", value["effects"]["mood_sound"]["offset"].get<NBT::Double>());

                    effects->Insert(std::move(mood_sound));
                }
                if(value["effects"].contains("additions_sound"))
                {
                    NBT::NBT additions_sound("additions_sound");

                    additions_sound->Insert("sound", value["effects"]["additions_sound"]["sound"].get<NBT::String>());
                    additions_sound->Insert("tick_chance", value["effects"]["additions_sound"]["tick_chance"].get<NBT::Double>());

                    effects->Insert(std::move(additions_sound));
                }
                if(value["effects"].contains("music"))
                {
                    NBT::NBT music("music");

                    music->Insert("sound", value["effects"]["music"]["sound"].get<NBT::String>());
                    music->Insert("min_delay", value["effects"]["music"]["min_delay"].get<NBT::Int>());
                    music->Insert("max_delay", value["effects"]["music"]["max_delay"].get<NBT::Int>());
                    music->Insert("replace_current_music", value["effects"]["music"]["replace_current_music"].get<NBT::Byte>());
                }

                data->Insert(std::move(effects));
                removeNameBytesFromRootNBT(packet, data);
            }
            util::writeVarInt(packet, 0, 0x07);
            util::writeVarInt(packet, 0, packet.size());
        }
        {
            std::vector<std::uint8_t>& packet = m_context.registry_packets[3];
            nlohmann::json dimension_type = registryJson["minecraft:dimension_type"];

            identifierSerializer.Serialize(packet, Identifier("dimension_type"));
            util::writeVarInt(packet,dimension_type.size());

            for(const auto& [key, value] : dimension_type.items())
            {
                util::writeStringToBuff(packet, key);
                util::ByteSerializer().Serialize(packet, 1);

                NBT::NBT data("");

                data->Insert("has_skylight", (NBT::Byte)value["has_skylight"]);
                data->Insert("has_ceiling", (NBT::Byte)value["has_ceiling"]);
                data->Insert("ultrawarm", (NBT::Byte)value["ultrawarm"]);
                data->Insert("natural", (NBT::Byte)value["natural"]);
                data->Insert("coordinate_scale", (NBT::Double)value["coordinate_scale"]);
                data->Insert("bed_works", (NBT::Byte)value["bed_works"]);
                data->Insert("respawn_anchor_works", (NBT::Byte)value["respawn_anchor_works"]);
                data->Insert("min_y", (NBT::Int)value["min_y"]);
                data->Insert("height", (NBT::Int)value["height"]);
                data->Insert("logical_height", (NBT::Int)value["logical_height"]);
                data->Insert("effects", (NBT::String)value["effects"]);
                data->Insert("infiniburn", (NBT::String)value["infiniburn"]);
                data->Insert("ambient_light", (NBT::Float)value["ambient_light"]);
                data->Insert("piglin_safe", (NBT::Byte)value["piglin_safe"]);
                data->Insert("has_raids", (NBT::Byte)value["has_raids"]);

                if (value["monster_spawn_light_level"].is_object())
                {
                    NBT::NBT light_level("monster_spawn_light_level");
                    light_level->Insert("min_inclusive", (NBT::Int)value["monster_spawn_light_level"]["min_inclusive"]);
                    light_level->Insert("max_inclusive", (NBT::Int)value["monster_spawn_light_level"]["max_inclusive"]);
                    light_level->Insert("type", (NBT::String)value["monster_spawn_light_level"]["type"]);
                    data->Insert(std::move(light_level));
                }
                else
                {
                    data->Insert("monster_spawn_light_level", (NBT::Int)value["monster_spawn_light_level"]);
                }

                data->Insert("monster_spawn_block_light_limit", (NBT::Int)value["monster_spawn_block_light_limit"]);

                removeNameBytesFromRootNBT(packet, data);
            }
            util::writeVarInt(packet, 0, 0x07);
            util::writeVarInt(packet, 0, packet.size());
        }
        {
            std::vector<std::uint8_t>& packet = m_context.registry_packets[4];
            nlohmann::json damage_type = registryJson["minecraft:damage_type"];

            identifierSerializer.Serialize(packet, Identifier("damage_type"));
            util::writeVarInt(packet, damage_type.size());

            for(const auto& [key, value] : damage_type.items())
            {
                util::writeStringToBuff(packet, key);
                util::ByteSerializer().Serialize(packet, 1);

                NBT::NBT data("");

                data->Insert("message_id", (NBT::String)value["message_id"]);
                data->Insert("scaling", (NBT::String)value["scaling"]);
                data->Insert("exhaustion", (NBT::Float)value["exhaustion"]);
                if (value.contains("effects"))
                {
                    data->Insert("effects", (NBT::String)value["effects"]);
                }
                if (value.contains("death_message_type"))
                {
                    data->Insert("death_message_type", (NBT::String)value["death_message_type"]);
                }


                removeNameBytesFromRootNBT(packet, data);
            }
            util::writeVarInt(packet, 0, 0x07);
            util::writeVarInt(packet, 0, packet.size());
        }
    }

}