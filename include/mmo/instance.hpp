#pragma once

#include "config.hpp"

#include <concepts>
#include <cstddef>
#include <string>
#include <thread>

#include "persistence/sqlite_database.hpp"
#include "transport/secure_websocket.hpp"

namespace mmo {

template <class T>
concept Persistence = requires(T t, std::string const &s, float f) {
    {
        t.save_user(s, s)
    } -> std::same_as<std::expected<std::tuple<>, error>>;
    {
        t.load_user_player(s, s)
    } -> std::same_as<std::expected<player, error>>;
    {
        t.set_init_position(f, f)
    } -> std::same_as<void>;
};

template <class T>
concept Transport = requires(T t) {
    {
        t.start()
    } -> std::same_as<void>;
    {
        t.stop()
    } -> std::same_as<void>;
};

template <class T>
concept Registrar = requires(T t, std::string const &s) {
    {
        t.register_user(s, s)
    } -> std::same_as<std::expected<std::tuple<>, error>>;
    {
        t.authenticate_user(s, s)
    } -> std::same_as<std::expected<size_t, error>>;
};

template <class T>
concept Scheduler = requires(T t) {
    {
        t.launch()
    } -> std::same_as<void>;
};

template <class T>
concept Cycle = requires(T t) {
    {
        t.cycle()
    } -> std::same_as<void>;
};

template <class T>
concept Authenticator = requires(T t, std::string const &s) {
    {
        t.authenticate_user(s, s)
    } -> std::same_as<std::expected<size_t, error>>;
};

template <class T>
concept Implementation = Persistence<T> && Transport<T> && Registrar<T> && Authenticator<T> && Cycle<T> && Scheduler<T>;

class mono_threaded
{
   public:
    void launch()
    {}
};

class secure_websocket
{
   public:
    void start()
    {}

    void stop()
    {}
};

class email_confirm
{
   public:
    std::expected<std::tuple<>, error> register_user(std::string const &nickname, std::string const &password)
    {
        return std::tuple{};
    }

    // std::expected<size_t, error> authenticate_user(std::string const &nickname, std::string const &password)
    // {
    //     return std::make_expected<size_t, error>(0);
    // }
};

template <Cycle C, Registrar R, Authenticator A, Persistence P, Transport T, Scheduler S>
class MMO_API game
{
   private:
    C &cycle_;
    R &registrar_;
    A &authenticator_;
    P &persistence_;
    T &transport_;
    S &scheduler_;

   public:
    template <Implementation I>
    game(I &implem)
        : game(implem, implem, implem, implem, implem, implem)
    {}

    game(C &cycle, R &registrar, A &authenticator, P &persistence, T &transport, S &scheduler)
        : cycle_(cycle)
        , registrar_(registrar)
        , authenticator_(authenticator)
        , persistence_(persistence)
        , transport_(transport)
        , scheduler_(scheduler)
    {}

    template <class Rep, class Period = std::ratio<1>>
    void run(std::chrono::duration<Rep, Period> const &tick_duration)
    {
        transport_.start();
        std::this_thread::sleep_for(tick_duration);
        transport_.stop();
    }
};

}   // namespace mmo
