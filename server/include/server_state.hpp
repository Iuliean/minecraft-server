#ifndef SERVER_STATE_HPP
#define SERVER_STATE_HPP
#include <optional>
#include <utility>
#include <string_view>
#include <format>
namespace mc
{
    enum class state
    {
        idle,
        status,
        login,
        config,
        play
    };

    inline constexpr std::optional<state> state_from(int value)
    {
        switch(value)
        {
            using enum state;
            case 0: return idle;
            case 1: return status;
            case 2: return login;
            case 3: return config;
            case 4: return play;
            default:
                return std::nullopt;
        }
        std::unreachable();
    }

    inline constexpr std::string_view state_to_str(state value)
    {
        switch (value)
        {
            using enum state;
            case idle: return "idle";
            case status: return "status";
            case login: return "login";
            case config: return "config";
            case play: return "play";
        }
        std::unreachable();
    }

    class server_state
    {
    public:
        server_state() noexcept
            : m_state(state::idle) {}

        virtual ~server_state() noexcept = default;

        void set_state(state state) noexcept { m_state = state; }
        state get_state()const noexcept { return m_state; }

    protected:
        state m_state;
    };
}

template <>
struct std::formatter<mc::state> : public std::formatter<std::string>
{
  auto format(mc::state state, format_context& ctx) const {
    return std::format_to(ctx.out(), "state:{{{}}}", mc::state_to_str(state));
  }
};


#endif //SERVER_STATE_HPP
