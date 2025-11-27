#ifndef SERVER_STATE_HPP
#define SERVER_STATE_HPP

namespace mc
{
    enum class state
    {
        idle,
        status,
        config,
        login,
        play
    };

    class server_state
    {
    public:
        server_state()
            : m_state(state::idle) {}

        virtual ~server_state() = default;

        void set_state(state state);
        state get_state()const { return m_state; }

    protected:
        state m_state;
    };
}

#endif //SERVER_STATE_HPP
