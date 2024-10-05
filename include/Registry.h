#ifndef REGITSTRY_H
#define REGITSTRY_H

#include "BlockState.h"
#include "SFW/Logger.h"
#include "SFW/utils.h"
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace mc
{
    //For now this shouldn't require thread safety
    //The class is written to at initialization
    //then threads only perform reads (or should)
    class BlockStateRegistry
    {
    public:
    
        ~BlockStateRegistry()= default;
        
        void MapState(BlockState state, int stateId);
        
        std::optional<int> GetBlockStateId(const BlockState& state)const;
        std::optional<BlockState> GetBlockState(int id)const;

        static void Init(std::filesystem::path registryPath);
        static void Deinit();
        static const BlockStateRegistry& Instance() noexcept
        {
            ASSERT(s_registryInstance != nullptr, "BlockRegistry not initialized");
            return *s_registryInstance;
        }
    private:
        BlockStateRegistry();
        
        iu::Logger m_logger;
        std::unordered_map<BlockState, int> m_stateToIdMap;
        std::unordered_map<int, BlockState> m_idToBlockState;

        static inline std::unique_ptr<BlockStateRegistry> s_registryInstance = nullptr;

        static void LoadRegistryFile(std::filesystem::path registryPath);
    };
}

#endif