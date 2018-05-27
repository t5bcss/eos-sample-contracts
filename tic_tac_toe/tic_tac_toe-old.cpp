#include "tic_tac_toe.hpp"
using namespace eosio;

namespace tic_tac_toe
{
struct impl
{
    void on(const create &c)
    {
        require_auth(c.host);
        eosio_assert(c.challenger != c.host, "challenger shouldn't be the same as host");

        // Check if game already exists
        games existing_host_games(code_account, c.host);
        auto itr = existing_host_games.find(c.challenger);
        eosio_assert(itr == existing_host_games.end(), "game already exists");

        existing_host_games.emplace(c.host, [&](auto &g) {
            g.challenger = c.challenger;
            g.host = c.host;
            g.turn = c.host;
        });
    }
    void on(const restart &r)
    {
        require_auth(r.by);

        // Check if game exists
        games existing_host_games(code_account, r.host);
        auto itr = existing_host_games.find(r.challenger);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

        // Check if this game belongs to the action sender
        eosio_assert(r.by == itr->host || r.by == itr->challenger, "this is not your game!");

        // Reset game
        existing_host_games.modify(itr, itr->host, [](auto &g) {
            g.reset_game();
        });
    }
    void on(const close &c)
    {
        require_auth(c.host);

        // Check if game exists
        games existing_host_games(code_account, c.host);
        auto itr = existing_host_games.find(c.challenger);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

        // Remove game
        existing_host_games.erase(itr);
    }
    bool is_empty_cell(const uint8_t &cell)
    {
        return cell == 0;
    }
    bool is_valid_movement(const movement &mvt, const game &game_for_movement)
    {
        uint32_t movement_location = mvt.row * 3 + mvt.column;
        bool is_valid = movement_location < board_len && is_empty_cell(game_for_movement.board[movement_location]);
        return is_valid;
    }

    account_name get_winner(const game &current_game)
    {
        if ((current_game.board[0] == current_game.board[4] && current_game.board[4] == current_game.board[8]) ||
            (current_game.board[1] == current_game.board[4] && current_game.board[4] == current_game.board[7]) ||
            (current_game.board[2] == current_game.board[4] && current_game.board[4] == current_game.board[6]) ||
            (current_game.board[3] == current_game.board[4] && current_game.board[4] == current_game.board[5]))
        {
            //  - | - | x    x | - | -    - | - | -    - | x | -
            //  - | x | -    - | x | -    x | x | x    - | x | -
            //  x | - | -    - | - | x    - | - | -    - | x | -
            if (current_game.board[4] == 1)
            {
                return current_game.host;
            }
            else if (current_game.board[4] == 2)
            {
                return current_game.challenger;
            }
        }
        else if ((current_game.board[0] == current_game.board[1] && current_game.board[1] == current_game.board[2]) ||
                 (current_game.board[0] == current_game.board[3] && current_game.board[3] == current_game.board[6]))
        {
            //  x | x | x       x | - | -
            //  - | - | -       x | - | -
            //  - | - | -       x | - | -
            if (current_game.board[0] == 1)
            {
                return current_game.host;
            }
            else if (current_game.board[0] == 2)
            {
                return current_game.challenger;
            }
        }
        else if ((current_game.board[2] == current_game.board[5] && current_game.board[5] == current_game.board[8]) ||
                 (current_game.board[6] == current_game.board[7] && current_game.board[7] == current_game.board[8]))
        {
            //  - | - | -       - | - | x
            //  - | - | -       - | - | x
            //  x | x | x       - | - | x
            if (current_game.board[8] == 1)
            {
                return current_game.host;
            }
            else if (current_game.board[8] == 2)
            {
                return current_game.challenger;
            }
        }
        else
        {
            bool is_board_full = true;
            for (uint8_t i = 0; i < board_len; i++)
            {
                if (is_empty_cell(current_game.board[i]))
                {
                    is_board_full = false;
                    break;
                }
            }
            if (is_board_full)
            {
                return N(draw);
            }
        }
        return N(none);
    }
    void on(const move &m)
    {
        require_auth(m.by);

        // Check if game exists
        games existing_host_games(code_account, m.host);
        auto itr = existing_host_games.find(m.challenger);
        eosio_assert(itr != existing_host_games.end(), "game doesn't exists");

        // Check if this game hasn't ended yet
        eosio_assert(itr->winner == N(none), "the game has ended!");
        // Check if this game belongs to the action sender
        eosio_assert(m.by == itr->host || m.by == itr->challenger, "this is not your game!");
        // Check if this is the  action sender's turn
        eosio_assert(m.by == itr->turn, "it's not your turn yet!");

        // Check if user makes a valid movement
        eosio_assert(is_valid_movement(m.mvt, *itr), "not a valid movement!");

        // Fill the cell, 1 for host, 2 for challenger
        const auto cell_value = itr->turn == itr->host ? 1 : 2;
        const auto turn = itr->turn == itr->host ? itr->challenger : itr->host;
        existing_host_games.modify(itr, itr->host, [&](auto &g) {
            g.board[m.mvt.row * 3 + m.mvt.column] = cell_value;
            g.turn = turn;

            //check to see if we have a winner
            g.winner = get_winner(g);
        });
    }

    /// The apply method implements the dispatch of events to this contract
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {

        if (code == code_account)
        {
            if (action == N(create))
            {
                impl::on(eosio::unpack_action_data<tic_tac_toe::create>());
            }
            else if (action == N(restart))
            {
                impl::on(eosio::unpack_action_data<tic_tac_toe::restart>());
            }
            else if (action == N(close))
            {
                impl::on(eosio::unpack_action_data<tic_tac_toe::close>());
            }
            else if (action == N(move))
            {
                impl::on(eosio::unpack_action_data<tic_tac_toe::move>());
            }
        }
    }
};
} // namespace tic_tac_toe

extern "C"
{

    using namespace tic_tac_toe;
    /// The apply method implements the dispatch of events to this contract
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        impl().apply(receiver, code, action);
    }
}