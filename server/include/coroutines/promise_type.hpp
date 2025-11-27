#ifndef PROMISE_TYPE_HPP
#define PROMISE_TYPE_HPP

#include "SFW/LoggerManager.h"

#include <concepts>
#include <coroutine>
#include <type_traits>
#include <utility>

namespace mc
{
    template<typename T, typename CoroutineT>
        requires std::default_initializable<T>
    class promise_type
    {
    public:
        promise_type() = default;

        CoroutineT get_return_object() noexcept(std::is_nothrow_constructible<CoroutineT, typename CoroutineT::handle_t>())
        {
            return CoroutineT{CoroutineT::handle_t::from_promise(*this)};
        }

        constexpr std::suspend_always initial_suspend() const noexcept
        {
            return {};
        }

        constexpr std::suspend_always final_suspend() const noexcept
        {
            return {};
        }

        void return_value(T&& value)
        {
            m_value = std::forward<T&&>(value);
        }

        std::suspend_always yield_value(T&& value)
        {
            m_value = std::forward<T&&>(value);
            return {};
        }

        void unhandled_exception()
        {
                SFW_LOG_ERROR("coro", "UNHANDLED EXCEPTION!!!!!!");
        }

    private:
        T m_value;
    };
}

#endif //PROMISE_TYPE_HPP
