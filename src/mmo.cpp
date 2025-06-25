#include "mmo.hpp"

#include <cstddef>
#include <cstdint>
#include <map>

#include "secure_transport.hpp"
#include "session.hpp"

namespace mmo {

static void
on_time_(unsigned short tick_in_ms, std::map<size_t, std::shared_ptr<player>> &players, game_cycle &cycle,
         std::shared_ptr<secure_transport> transport)
{
    auto order = cycle(tick_in_ms, players);
    if (order == order::stop)
        transport->stop();
}

static void
on_message_(std::shared_ptr<tls_session> session, std::string const &message,
            std::map<size_t, std::shared_ptr<player>> &players, std::map<tls_session *, size_t> &session_to_player_id)
{
    if (!session_to_player_id.contains(session.get()))
    {
        size_t player_id                    = players.size() + 1;
        auto   player                       = std::make_shared<mmo::player>();
        players[player_id]                  = player;
        session_to_player_id[session.get()] = player_id;
    }
    auto player_id = session_to_player_id[session.get()];
    players[player_id]->in.push(message);
}

void
start(game_cycle &&cycle, std::vector<uint8_t> const &cert_pem, std::vector<uint8_t> const &key_pem,
      unsigned short tick_in_ms, unsigned short port)
{
    if (tick_in_ms == 0)
    {
        throw std::invalid_argument("Tick interval must be greater than 0");
    }

    std::map<size_t, std::shared_ptr<player>> players;
    std::map<tls_session *, size_t>           session_to_player_id;

    auto transport =
        std::make_shared<secure_transport>(port, cert_pem, key_pem,
                                           std::bind(on_message_, std::placeholders::_1, std::placeholders::_2,
                                                     std::ref(players), std::ref(session_to_player_id)));

    transport->set_timer(tick_in_ms, std::bind(on_time_, tick_in_ms, players, cycle, transport));
    transport->start();
}

}   // namespace mmo
