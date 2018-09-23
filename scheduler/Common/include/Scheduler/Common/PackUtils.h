#pragma once

#include <cstddef>
#include <utility>

namespace Scheduler {
    template<typename Arg, typename... Args>
    struct PackUtils
    {
        static const constexpr size_t size = 1U + sizeof...(Args);

        template<typename Fn>
        inline constexpr static Fn ForEach(Fn fn, Arg&& value, Args&& ...values)
        {
            fn(std::forward<Arg>(value));
            PackUtils<Args...>::ForEach(fn, std::forward<Args>(values)...);
            return fn;
        }

        template<typename Fn>
        inline constexpr Fn operator()(Fn fn, Arg&& value, Args&& ...values)
        {
            return ForEach(fn, std::forward<Arg>(value), std::forward<Args>(values)...);
        }
    };

    template<typename Arg>
    struct PackUtils<Arg>
    {
        static const constexpr size_t size = 1U;

        template<typename T>
        inline constexpr static T ForEach(T callback, Arg&& value)
        {
            callback(std::forward<Arg>(value));
            return callback;
        }

        template<typename T>
        inline constexpr T operator()(T callback, Arg&& value) const
        {
            return ForEach(callback, std::forward<T>(value));
        }
    };
}
