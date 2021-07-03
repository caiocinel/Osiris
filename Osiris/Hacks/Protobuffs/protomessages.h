#pragma once
#define msg_matchmaking_client_ceserve 9107
#define msg_client_welcome 4004
#define msg_client_hello 4006
#define msg_item_equipped_state 1059
#define msg_matchmaking_client_hello 9109
#define msg_matchmaking_gc_client_hello 9110
#define msg_client_rank_update  9194

#include <stdint.h>
#include "pbwrap.h"

enum rank_type : int {
    mm = 6,
    wingmans = 7,
    danger_zone = 10
};

enum mode_settigns : int {
    matchmaking = 0,
    wingman = 1,
    dangerzone = 2
};

using namespace pbwrap;

struct msg_client_hello_t : pbmsg<8> {
    PBMSG_CTOR;
    PBFIELD(3, types::Uint32, client_session_need);
};

struct matchmaking_client_hello_t : pbmsg<20> {
    struct player_ranking_info_t : pbmsg<6> {
        PBMSG_CTOR;
        PBFIELD(1, types::Uint32, account_id);
        PBFIELD(2, types::Uint32, rank_id);
        PBFIELD(3, types::Uint32, wins);
        PBFIELD(6, types::Uint32, rank_type_id);
    };
    struct player_commendation_info_t : pbmsg<4> {
        PBMSG_CTOR;
        PBFIELD(1, types::Uint32, cmd_friendly);
        PBFIELD(2, types::Uint32, cmd_teaching);
        PBFIELD(4, types::Uint32, cmd_leader);
    };

    PBMSG_CTOR;
    PBFIELD(4, types::Int32, penalty_seconds);
    PBFIELD(5, types::Int32, penalty_reason);
    PBFIELD(7, player_ranking_info_t, ranking);
    PBFIELD(8, player_commendation_info_t, commendation);
    PBFIELD(17, types::Int32, player_level);
    PBFIELD(18, types::Int32, player_level_xp);
};

struct msg_client_rank_update_t : pbmsg<1> {
    PBMSG_CTOR;
    PBFIELD(1, matchmaking_client_hello_t::player_ranking_info_t, ranking);
};

