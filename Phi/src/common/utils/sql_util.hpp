/*
 * @File	  : sql_utils.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/03/11 14:35
 * @Introduce : SQLite工具
*/

#pragma once

#include <memory>
#include <regex>
#include <filesystem>
#include <sqlite_modern_cpp.h>
#include "common/utils/log_system.hpp"
#include "configuration/config.hpp"
#include "common/exception/self_exception.hpp"
#include "sqlite3.h"

#ifndef SQL_UTIL_HPP
#define SQL_UTIL_HPP  

namespace SQL_Util {
	inline sqlite::database LocalDB(Config::getConfig()["db"]["path"]["local"].as<std::string>());
	inline sqlite::database PhiDB(Config::getConfig()["db"]["path"]["phi"].as<std::string>());
	inline sqlite::database PlayerRdDB(Config::getConfig()["db"]["path"]["phi-player"].as<std::string>());
	inline void initialized(void) {
        // 初始化phigros

        Global::PhigrosSongInfo.clear();
        Global::Phis.clear();
        Global::PhigrosPlayerAvatar.clear();


        // 曲目消息
        SQL_Util::PhiDB << "select sid,id,title,song_illustration_path,rating_ez,rating_hd,rating_in,rating_at,rating_lg,artist from phigros;"
            >> [&](std::string sid, int id, std::string title, std::string song_illustration_path,
                float rating_ez, float rating_hd, float rating_in, float rating_at, float rating_lg, std::string artist) {
                    DefinedStruct::PhiSongInfo phi;
                    if (!sid.empty())
                    {
                        phi.sid = sid;
                        phi.id = id;
                        phi.title = title;
                        phi.artist = artist;
                        phi.song_illustration_path = song_illustration_path;
                        phi.rating[0] = rating_ez;
                        phi.rating[1] = rating_hd;
                        phi.rating[2] = rating_in;
                        phi.rating[3] = rating_at;
                        phi.rating[4] = rating_lg;

                        Global::PhigrosSongInfo[sid] = std::move(phi);
                        Global::Phis[id] = sid;
                    }
            };

        // 玩家头像
        SQL_Util::PhiDB << "select sid,avatar_id,avatar_path from avatar;"
            >> [&](uint32_t sid, std::string avatar_id, std::string avatar_path) {
            DefinedStruct::PhiAvatar avatar;
            if (!avatar_path.empty() and !avatar_id.empty())
            {
                avatar.avatar_path = avatar_path;
                avatar.sid = sid;
                Global::PhigrosPlayerAvatar[avatar_id] = avatar;
            }
            };
	};
};
#endif