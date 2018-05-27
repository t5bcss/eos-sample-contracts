// Import necessary library
#include <eosiolib/eosio.hpp> // Generic eosio library, i.e. print, type, math, etc

using namespace eosio;
namespace tic_tac_toe
{
static const account_name games_account = N(games);
static const account_name code_account = N(tic.tac.toe);
// Your code here
typedef eosio::multi_index<games_account, game> games;
static const uint32_t board_len = 9;
struct game
{
    game() {}
    game(account_name challenger, account_name host) : challenger(challenger), host(host), turn(host)
    {
        // Initialize board
        initialize_board();
    }
    account_name challenger;
    account_name host;
    account_name turn;             // = account name of host/ challenger
    account_name winner = N(none); // = none/ draw/ account name of host/ challenger
    uint8_t board[9];              //

    // Initialize board with empty cell
    void initialize_board()
    {
        for (uint8_t i = 0; i < board_len; i++)
        {
            board[i] = 0;
        }
    }

    // Reset game
    void reset_game()
    {
        initialize_board();
        turn = host;
        winner = N(none);
    }

    auto primary_key() const { return challenger; }

    EOSLIB_SERIALIZE(game, (challenger)(host)(turn)(winner)(board))
};

struct create
{
    account_name challenger;
    account_name host;

    EOSLIB_SERIALIZE(create, (challenger)(host))
};

struct restart
{
    account_name challenger;
    account_name host;
    account_name by;

    EOSLIB_SERIALIZE(restart, (challenger)(host)(by))
};

struct close
{
    account_name challenger;
    account_name host;

    EOSLIB_SERIALIZE(close, (challenger)(host))
};

struct movement
{
    uint32_t row;
    uint32_t column;

    EOSLIB_SERIALIZE(movement, (row)(column))
};

struct move
{
    account_name challenger;
    account_name host;
    account_name by; // the account who wants to make the move
    movement mvt;

    EOSLIB_SERIALIZE(move, (challenger)(host)(by)(mvt))
};


} // namespace tic_tac_toe