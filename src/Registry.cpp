#include "Registry.h"
#include "BlockState.h"
#include "DataTypes/Identifier.h"
#include "SFW/LoggerManager.h"
#include "SFW/utils.h"
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>


namespace mc
{
    //  #############################
    //  # BlockStateRegistry Static #
    //  #############################
    void BlockStateRegistry::Init(std::filesystem::path registryPath)
    {
        if (!s_registryInstance)
        {
            s_registryInstance.reset(new BlockStateRegistry());
            LoadRegistryFile(registryPath);
        }
        else
        {
            ASSERT(false, "Double initialization of BlockStateRegistry is not allowed");
        }
    }

    void BlockStateRegistry::Deinit()
    {
        if (s_registryInstance)
        {
            s_registryInstance.reset();
        }
    }

    void BlockStateRegistry::LoadRegistryFile(std::filesystem::path registryPath)
    {
        std::ifstream file(registryPath);
        const auto registryJson = nlohmann::json::parse(file);

        for (const auto& [blockName, block] : registryJson.items())
        {
            const auto delimiterPos = blockName.find(':');
            const mc::Identifier blockIdentifier(blockName.substr(delimiterPos + 1));
            
            for (const auto& state : block["states"])
            {
                mc::BlockState blockState(blockIdentifier);

                if (!block.contains("properties"))
                {
                    s_registryInstance->MapState(blockState, state["id"].get<int>());
                    continue;
                }

                for (const auto& [propertyName, propertyValue] : state["properties"].items())
                {
                    if (propertyValue.is_string())
                    {
                        blockState.AddProperty(propertyName, propertyValue.get<std::string>());
                    }
                    else if (propertyValue.is_number_integer())
                    {
                        blockState.AddProperty(propertyName, propertyValue.get<int>());
                    }
                    else if (propertyValue.is_boolean())
                    {
                        blockState.AddProperty(propertyName, propertyValue.get<bool>());
                    }
                    else
                    {
                        iu::LoggerManager::GlobalLogger().warn("Failed to parse property {} unsuppoerted type", propertyName);
                    }
                }

                s_registryInstance->MapState(blockState, state["id"].get<int>());

            }
        }
    }

    //  ######################
    //  # BlockStateRegistry #
    //  ######################

    BlockStateRegistry::BlockStateRegistry()
        : m_logger(iu::LoggerManager::GetLogger("BlockStateRegistry")),
        m_stateToIdMap(),
        m_idToBlockState()
    {}

    void BlockStateRegistry::MapState(BlockState state, int stateId)
    {
        m_stateToIdMap.emplace(state, stateId);
        m_idToBlockState.emplace(stateId, state);
    }

    std::optional<int> BlockStateRegistry::GetBlockStateId(const BlockState& state) const
    {
        const auto iter = m_stateToIdMap.find(state);
        if (iter != m_stateToIdMap.cend())
            return iter->second;
        else
            return {};
    }

    std::optional<BlockState> BlockStateRegistry::GetBlockState(int id) const
    {
        const auto iter = m_idToBlockState.find(id);
        if (iter != m_idToBlockState.cend())
            return iter->second;
        else
            return {};
    }
}