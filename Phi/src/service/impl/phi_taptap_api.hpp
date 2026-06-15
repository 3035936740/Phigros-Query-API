/*
 * @File	  : phi_taptap_api.hpp
 * @Coding	  : utf-8
 * @Author    : Bing
 * @Time      : 2023/05/10 16:22
 * @Introduce : phi对应yuhao的python代码重写
*/


#pragma once

#define CPPHTTPLIB_OPENSSL_SUPPORT

#include <regex>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include "httplib.h"
#include "configuration/config.hpp"
#include "common/utils/other_util.hpp"
#include <cryptopp/cryptlib.h>
#include <Poco/Zip/ZipStream.h>
#include <Poco/Zip/ZipException.h>
#include <Poco/Zip/ZipArchive.h> 
#include <Poco/Zip/ZipLocalFileHeader.h> 
#include <Poco/StreamCopier.h>

#ifndef PHI_TAPTAP_API_HPP
#define PHI_TAPTAP_API_HPP  
#include <cpprest/http_client.h>
#include <cpprest/json.h>

using ubyte = unsigned char;

#if 0
void HexDebug(const auto& content) {
	std::uint32_t hits{ 0 };
	for (const auto& data : content)
	{
		if (hits != 0){
			std::cout << ',';
		}
		std::cout << std::format("0x{:02X}", data);
		hits++;
	};
	std::cout << std::endl;
}
#endif // DEBUG


namespace self {
	class BinaryReader {
	private:
		std::vector<uint8_t> data_;
		size_t pos_ = 0;
		bool big_endian_{ };

	public:
		BinaryReader(std::vector<uint8_t>& stream, bool big_endian = true) : big_endian_{ big_endian } {
			if (stream.empty()) {
				return;
			}

			if (stream.data() != nullptr) {
				data_ = stream;
				pos_ = 0;
			}
		}

		uint8_t ReadByte() {
			uint8_t value = data_[pos_];
			pos_ += 1;
			return value;
		}

		int16_t ReadInt16() {
			int16_t value;
			std::memcpy(&value, &data_[pos_], sizeof(value));
			if (big_endian_) {
				OtherUtil::littleBigEndianConversion<int16_t>(value);
			}
			pos_ += sizeof(value);
			return value;
		}

		int32_t ReadInt32() {
			int32_t value;
			std::memcpy(&value, &data_[pos_], sizeof(value));
			if (big_endian_) {
				OtherUtil::littleBigEndianConversion<int32_t>(value);
			}
			pos_ += sizeof(value);
			return value;
		}

		float ReadSingle() {
			float value;
			std::memcpy(&value, &data_[pos_], sizeof(value));
			if (big_endian_) {
				OtherUtil::littleBigEndianConversion<float>(value);
			}
			pos_ += sizeof(value);
			return value;
		}

		std::string ReadStr() {
			uint8_t strLength = ReadByte();
			std::string str(reinterpret_cast<const char*>(&data_[pos_]), strLength);
			pos_ += strLength;
			return str;
		}

		uint64_t ReadVarInt() {
			uint64_t result{ 0 };
			uint32_t shift{ 0 };
			while (pos_ < data_.size()) {
				const uint8_t byte{ ReadByte() };
				result |= static_cast<uint64_t>(byte & 0x7F) << shift;
				if ((byte & 0x80) == 0) {
					return result;
				}
				shift += 7;
			}
			throw std::out_of_range("Varint data read out of bounds");
		}

		void SkipVarInt() {
			ReadVarInt();
		}

		std::string ReadVarStr() {
			const auto strLength{ ReadVarInt() };
			if (pos_ + strLength > data_.size()) {
				throw std::out_of_range("String data read out of bounds");
			}
			std::string str(reinterpret_cast<const char*>(&data_[pos_]), strLength);
			pos_ += strLength;
			return str;
		}

		size_t remaining() const {
			return data_.size() - pos_;
		}

		const auto& getPosition() const {
			return this->pos_;
		}

		bool getBit(uint8_t data, uint8_t index) {
			return (data & (1 << index)) != 0;
		}

		void setPosition(size_t pos) {
			this->pos_ = pos;
		}
	};

	struct GameProgress {
		uint8_t isFirstRun{ 0 };
		uint8_t legacyChapterFinished{ 0 };
		uint8_t alreadyShowCollectionTip{ 0 };
		uint8_t alreadyShowAutoUnlockINTip{ 0 };
		uint8_t completed{ 0 };
		uint8_t songUpdateInfo{ 0 };
		short challengeModeRank{ 0 };
		std::array<short, 5> data{ 0,0,0,0,0 };
		uint8_t unlockFlagOfSpasmodic{};
		uint8_t unlockFlagOfIgallta{ 0 };
		uint8_t unlockFlagOfRrharil{ 0 };
		uint8_t flagOfSongRecordKey{ 0 };
		uint8_t randomVersionUnlocked{ 0 };
		uint8_t chapter8UnlockBegin{ 0 };
		uint8_t chapter8UnlockSecondPhase{ 0 };
		uint8_t chapter8Passed{ 0 };
		uint8_t chapter8SongUnlocked{ 0 };

#if 0
		void Print() {
			std::cout // << std::hex << std::uppercase << std::setw(2) << std::setfill('0') 
				<< static_cast<int>(isFirstRun) << ", "
				<< static_cast<int>(legacyChapterFinished) << ", "
				<< static_cast<int>(alreadyShowCollectionTip) << ", "
				<< static_cast<int>(alreadyShowAutoUnlockINTip) << ", "
				<< static_cast<int>(completed) << ", "
				<< static_cast<int>(songUpdateInfo) << ", "
				<< "challengeModeRank: " << challengeModeRank << ", \n"
				<< "data: " << data[0] << ", "
				<< data[1] << ", "
				<< data[2] << ", "
				<< data[3] << ", "
				<< data[4] << ", \n"
				<< static_cast<int>(unlockFlagOfSpasmodic) << ", "
				<< static_cast<int>(unlockFlagOfIgallta) << ", "
				<< static_cast<int>(unlockFlagOfRrharil) << ", "
				<< static_cast<int>(flagOfSongRecordKey) << ", "
				<< static_cast<int>(randomVersionUnlocked) << ", "
				<< static_cast<int>(chapter8UnlockBegin) << ", "
				<< static_cast<int>(chapter8UnlockSecondPhase) << ", "
				<< static_cast<int>(chapter8Passed) << ", "
				<< static_cast<int>(chapter8SongUnlocked) << std::endl;
		};
#endif
	};

	struct SongScore {
		float acc{};
		unsigned int score{};
		bool is_fc{};
		std::string difficulty{};
	};

	struct UserData {
		std::string profile{}, avatar{}, background{};
		uint16_t unlocked_count{ 0 };
	};

	struct SaveModel {
		std::string objectId;
		std::string gameObjectId;
		std::string userObjectId;
		std::string summary;
		std::string checksum;
	};

	class PhiTaptapAPI {
	public:

		class CloudSaveSummary {
		public:
			std::string updatedAt;
			std::time_t timestamp;
			float RankingScore;
			uint16_t ChallengeModeRank;
			std::vector<uint16_t> EZ;
			std::vector<uint16_t> HD;
			std::vector<uint16_t> IN;
			std::vector<uint16_t> AT;
			std::string nickname;

			CloudSaveSummary(std::string Summary, std::string UpdateTime, std::string nickname) {
				const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

				auto b64_to_int = [&](char c) -> int {
					size_t pos = base64_chars.find(c);
					
					/*
					if (pos == std::string::npos) {
						throw std::runtime_error("Invalid base64 character");
					}
					*/
					
					return pos;
				};

				auto decode_base64 = [&](const std::string& encoded_string) -> std::vector<uint8_t> {
					size_t in_len = encoded_string.size();
					size_t i = 0;
					size_t j = 0;
					int in_ = 0;
					uint8_t char_array_4[4]{}, char_array_3[3]{};
					std::vector<uint8_t> ret;

					while (in_len-- && (encoded_string[in_] != '=') && (isalnum(encoded_string[in_]) || encoded_string[in_] == '+'
						|| encoded_string[in_] == '/')) {
						char_array_4[i++] = encoded_string[in_]; in_++;
						if (i == 4) {
							for (i = 0; i < 4; i++) {
								char_array_4[i] = b64_to_int(char_array_4[i]);
							}
							char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
							char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
							char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

							for (i = 0; (i < 3); i++) {
								ret.push_back(char_array_3[i]);
							}
							i = 0;
						}
					}

					if (i) {
						for (j = i; j < 4; j++) {
							char_array_4[j] = 0;
						}
						for (j = 0; j < 4; j++) {
							char_array_4[j] = b64_to_int(char_array_4[j]);
						}
						char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
						char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
						char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

						for (j = 0; (j < i - 1); j++) {
							ret.push_back(char_array_3[j]);
						}
					}

					return ret;
				};

				std::vector<uint8_t> summary = decode_base64(Summary);
				if (summary.size() < 8) {
					throw std::runtime_error("Invalid cloud save summary");
				}

				BinaryReader reader(summary, false);
				reader.ReadByte(); // saveVersion
				ChallengeModeRank = reader.ReadInt16();
				RankingScore = reader.ReadSingle();
				reader.ReadVarInt(); // gameVersion
				reader.ReadVarStr(); // avatar

				std::array<std::vector<uint16_t>*, 4> levels{ &EZ, &HD, &IN, &AT };
				for (auto* level : levels) {
					level->push_back(static_cast<uint16_t>(reader.ReadInt16()));
					level->push_back(static_cast<uint16_t>(reader.ReadInt16()));
					level->push_back(static_cast<uint16_t>(reader.ReadInt16()));
				}

				std::tm tm = {};
				std::istringstream ss(UpdateTime);
				ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
				std::time_t t = std::mktime(&tm);
				t += 8L * 3600;

				std::tm* tm1{ std::localtime(&t) };
				char buffer[80];
				std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", tm1);
				

				this->updatedAt = buffer;
				this->timestamp = t;
				this->nickname = nickname;
			}
		};


	private:
		struct TapTapCloudConfig {
			std::string dataUrl;
			std::string uploadUrl;
			std::string saveListUri;
			std::string appId;
			std::string appKey;
			bool isInternational{ false };
		};

		httplib::Headers m_headers{};

		// progress信息
		GameProgress m_gameProgress{};

		// 曲目id/难度(0/1/2/3/4)/信息
		std::unordered_map<std::string, std::unordered_map<ubyte, SongScore>> m_player_record{};

		SaveModel m_saveModel{};

		std::string
			m_nickname{},
			m_userObjectId{},
			m_sessionToken{};

		std::unordered_map<std::string, std::vector<uint8_t>> m_decodedSaveData{};

		UserData m_user_data{};

		Json m_player_info, m_game_save_info;

		const std::string GAME_SAVE_URI{ "/1.1/classes/_GameSave" },
			ME_URI{ "/1.1/users/me" },
			FILE_TOKENS_URI{ "/1.1/fileTokens" },
			FILE_CALLBACK_URI{ "/1.1/fileCallback" },
			KEY_BASE64{ "6Jaa0qVAJZuXkZCLiOa/Ax5tIZVu+taKUN1V1nqwkks=" },
			IV_BASE64{ "Kk/wisgNYwcAV8WVGMgyUw==" };

		static std::string trimRightSlash(std::string value) {
			while (!value.empty() && value.back() == '/') {
				value.pop_back();
			}
			return value;
		}

		static std::string normalizeLeanCloudBaseUrl(std::string value) {
			value = trimRightSlash(std::move(value));
			const std::string api_path{ "/1.1" };
			if (value.size() >= api_path.size() && value.ends_with(api_path)) {
				value.erase(value.size() - api_path.size());
			}
			return value;
		}

		static TapTapCloudConfig loadCloudConfig() {
			auto& config{ Config::getConfig() };
			TapTapCloudConfig cloud_config;
			cloud_config.isInternational = config["server"]["is-international"] ?
				config["server"]["is-international"].as<bool>() : false;
			cloud_config.dataUrl = normalizeLeanCloudBaseUrl(
				cloud_config.isInternational ?
				config["server"]["data-international-url"].as<std::string>() :
				config["server"]["data-url"].as<std::string>());
			cloud_config.uploadUrl = trimRightSlash(config["server"]["upload-data-url"].as<std::string>());
			cloud_config.saveListUri = cloud_config.isInternational ? "/1.1/gamesaves/" : "/1.1/classes/_GameSave";
			cloud_config.appId = cloud_config.isInternational ?
				config["server"]["international-app-id"].as<std::string>() :
				config["server"]["app-id"].as<std::string>();
			cloud_config.appKey = cloud_config.isInternational ?
				config["server"]["international-app-key"].as<std::string>() :
				config["server"]["app-key"].as<std::string>();
			return cloud_config;
		}

		static httplib::Headers makeHeaders(const TapTapCloudConfig& cloud_config, const std::string& sessionToken) {
			return {
				{"X-LC-Id", cloud_config.appId},
				{"X-LC-Key", cloud_config.appKey},
				{"User-Agent", "LeanCloud-CSharp-SDK/1.0.3"},
				{"Accept", "application/json"},
				{"Content-Type", "application/json"},
				{"X-LC-Session", sessionToken}
			};
		}

		std::vector<uint8_t> unzip(std::istringstream& iss, const Poco::Zip::ZipLocalFileHeader& header) {
			Poco::Zip::ZipInputStream zipin(iss, header);
			std::ostringstream out(std::ios::binary);
			Poco::StreamCopier::copyStream(zipin, out);
			std::string new_str{ out.str() };

			std::vector<uint8_t> vec(new_str.begin() + 1, new_str.end());

			// 编码解码
			std::vector<ubyte> key{ OtherUtil::base64Decode(this->KEY_BASE64) };
			std::vector<ubyte> iv{ OtherUtil::base64Decode(this->IV_BASE64) };

			return OtherUtil::decrypt_AES_CBC(vec, key, iv);
		}

		std::vector<uint8_t> unzip(std::ifstream& ifs, const Poco::Zip::ZipLocalFileHeader& header) {
			Poco::Zip::ZipInputStream zipin(ifs, header);
			std::ostringstream out(std::ios::binary);
			Poco::StreamCopier::copyStream(zipin, out);
			std::string new_str{ out.str() };

			std::vector<uint8_t> vec(new_str.begin() + 1, new_str.end());

			// 编码解码
			std::vector<ubyte> key{ OtherUtil::base64Decode(this->KEY_BASE64) };
			std::vector<ubyte> iv{ OtherUtil::base64Decode(this->IV_BASE64) };

			return OtherUtil::decrypt_AES_CBC(vec, key, iv);
		}

		void getGameRecord(std::vector<ubyte>& data) {
			const std::string levels[]{ "EZ", "HD", "IN", "AT", "Legacy" };
			size_t data_size{ data.size() };
			std::unordered_map<std::string, std::unordered_map<ubyte, SongScore>> records;
			BinaryReader reader(data, false);

			uint16_t songcount{ static_cast<uint16_t>(reader.ReadVarInt()) };
			m_user_data.unlocked_count = songcount;

			// std::cout << "songcount" << songcount << std::endl;

			// auto fc{ reader.ReadByte() };

			for (size_t i = 0; i < songcount && reader.remaining() > 0; ++i) {
				auto songid{ reader.ReadVarStr() };
				reader.SkipVarInt();
				auto diffs{ reader.ReadByte() };
				auto fc = reader.ReadByte();

				//std::cout << songid << "\n";

				std::unordered_map<ubyte, SongScore> record;
				for (size_t j = 0; j < 5; ++j) {
					if (reader.getBit(diffs, j)) {
						SongScore song_score;
						song_score.score = reader.ReadInt32();
						song_score.acc = reader.ReadSingle();
						song_score.is_fc = (song_score.score == 1000000 && song_score.acc == 100.0f) || reader.getBit(fc, j);
						song_score.difficulty = levels[j];
						record[j] = std::move(song_score);
					}
				}

				// std::cout << "songid:" << songid << "list:" << i << "/" << songcount << std::endl;
				if (reader.getPosition() > data_size) {
					break;
					// records.clear();

					// std::cout << "Binary data read out of bounds\n";
					// continue;
					// throw std::out_of_range("Binary data read out of bounds");
				}

				records[songid] = std::move(record);
			}
			/*
			try{
				BinaryReader reader(data);
				auto songcount{ reader.ReadByte() };
				auto fc{ reader.ReadByte() };

				for (size_t i = 0; i < songcount; ++i) {
					auto songid{ reader.ReadStr() };
					auto length{ reader.ReadByte() };
					auto diffs{ reader.ReadByte() };
					fc = reader.ReadByte();

					//std::cout << songid << "\n";

					std::unordered_map<ubyte, SongScore> record;
					for (size_t j = 0; j < 5; ++j) {
						if (reader.getBit(diffs, j)) {
							SongScore song_score;
							song_score.score = reader.ReadInt32();
							song_score.acc = reader.ReadSingle();
							song_score.is_fc = reader.getBit(fc, j);
							song_score.difficulty = levels[j];
							record[j] = std::move(song_score);
						}
					}

					if (reader.getPosition() > data_size){
						records.clear();

						//std::cout << "Binary data read out of bounds\n";

						throw std::out_of_range("Binary data read out of bounds");
					}

					records[songid] = std::move(record);
				}
			}catch (const std::out_of_range& e) {
				BinaryReader reader(data);
				auto songcount{ reader.ReadByte() };

				for (size_t i = 0; i < songcount; ++i) {
					auto songid{ reader.ReadStr() };
					auto length{ reader.ReadByte() };
					auto diffs{ reader.ReadByte() };
					auto fc = reader.ReadByte();


					std::unordered_map<ubyte, SongScore> record;
					for (size_t j = 0; j < 5; ++j) {
						if (reader.getBit(diffs, j)) {
							SongScore song_score;
							song_score.score = reader.ReadInt32();
							song_score.acc = reader.ReadSingle();
							song_score.is_fc = reader.getBit(fc, j);
							song_score.difficulty = levels[j];
							record[j] = std::move(song_score);
						}
					}

					if (reader.getPosition() > data_size) {
						records.clear();
						throw std::out_of_range("Binary data read out of bounds");
					}

					records[songid] = std::move(record);
				}
			}*/

			//std::cout << "data size: " << data_size << ",position: " << reader.getPosition() << std::endl;
			this->m_player_record = std::move(records);
		}

		void selectControl(std::vector<uint8_t>& data,std::string_view key) {
			this->m_decodedSaveData[key.data()] = data;

			if (key == "gameKey") {
				// pass
			} else if (key == "gameProgress") {
				// HexDebug(data);
				BinaryReader reader(data, false);

				auto progress_flags{ reader.ReadByte() };
				this->m_gameProgress.isFirstRun = reader.getBit(progress_flags, 0);
				this->m_gameProgress.legacyChapterFinished = reader.getBit(progress_flags, 1);
				this->m_gameProgress.alreadyShowCollectionTip = reader.getBit(progress_flags, 2);
				this->m_gameProgress.alreadyShowAutoUnlockINTip = reader.getBit(progress_flags, 3);
				this->m_gameProgress.completed = static_cast<uint8_t>(!reader.ReadVarStr().empty());
				this->m_gameProgress.songUpdateInfo = static_cast<uint8_t>(reader.ReadVarInt());
				this->m_gameProgress.challengeModeRank = reader.ReadInt16();

				/*
				if (challengeModeRankFrontByte != 0) {
					this->m_gameProgress.challengeModeRank = static_cast<short>((reader.ReadByte() << 8) | challengeModeRankFrontByte);
				}
				*/

				for (auto& data : this->m_gameProgress.data) {
					data = static_cast<short>(reader.ReadVarInt());
					/*
					auto front_byte_data{ reader.ReadByte() };
					// std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(front_byte_data) << ",";
					if (front_byte_data != 0) {
						auto back_byte_data{ reader.ReadByte() };
						auto next_byte_data{ reader.ReadByte() }; // 获取到下一个data

						// std::cout << "(" << static_cast<int>(front_byte_data) << ", " << static_cast<int>(back_byte_data) << ", " << static_cast<int>(next_byte_data) << ")" << std::endl;

						reader.setPosition(reader.getPosition() - 1);
						// std::cout << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(back_byte_data) << std::endl;
						
						if (back_byte_data >= 1 and back_byte_data <= 8 and not(next_byte_data == 0 and back_byte_data == 1)) {
							data = (back_byte_data - 1) * 128 + front_byte_data;
						} else {
							data = front_byte_data;
							reader.setPosition(reader.getPosition() - 1);
						}
					}
					*/
				}

				this->m_gameProgress.unlockFlagOfSpasmodic = reader.ReadByte();
				this->m_gameProgress.unlockFlagOfIgallta = reader.ReadByte();
				this->m_gameProgress.unlockFlagOfRrharil = reader.ReadByte();
				this->m_gameProgress.flagOfSongRecordKey = reader.ReadByte();
				this->m_gameProgress.randomVersionUnlocked = reader.ReadByte();
				auto chapter8_flags{ reader.ReadByte() };
				this->m_gameProgress.chapter8UnlockBegin = reader.getBit(chapter8_flags, 0);
				this->m_gameProgress.chapter8UnlockSecondPhase = reader.getBit(chapter8_flags, 1);
				this->m_gameProgress.chapter8Passed = reader.getBit(chapter8_flags, 2);
				this->m_gameProgress.chapter8SongUnlocked = reader.ReadByte();
			} else if (key == "gameRecord") {
				getGameRecord(data);
			} else if (key == "settings") {
				// pass
			} else if (key == "user") {
				try {
					BinaryReader reader(data, false);
					reader.ReadByte(); // showPlayerId
					this->m_user_data.profile = reader.ReadVarStr();
					this->m_user_data.avatar = reader.ReadVarStr();
					this->m_user_data.background = reader.ReadVarStr();
				}
				catch (const std::out_of_range& e) {
					LogSystem::logError(e.what());
				}
				catch (const std::runtime_error& e) {
					LogSystem::logError(e.what());
				}
			} else {
				// pass
			}
		};

		void collectSave(const std::string& body) {
			std::istringstream iss(body, std::ios::binary);
			Poco::Zip::ZipArchive PocoZipArchive(iss);

			for (auto iterator{ PocoZipArchive.headerBegin() }; iterator != PocoZipArchive.headerEnd(); ++iterator) {
				const auto& header = iterator->second;
				std::string filename = header.getFileName();
				// Poco::UInt32 fileSize = header.getUncompressedSize();
				// std::cout << "filename: " << filename << "\nsize: " << fileSize << std::endl;

				iss.clear(); // 清除流的错误状态
				iss.seekg(0); // 将读取位置移动到流的开头

				auto data{ unzip(iss, header) };
				selectControl(data, filename);
			}
		};

		void collectSave(const std::filesystem::path& path) {
			std::ifstream ifs(path, std::ios::binary);

			Poco::Zip::ZipArchive PocoZipArchive(ifs);

			for (auto iterator{ PocoZipArchive.headerBegin() }; iterator != PocoZipArchive.headerEnd(); ++iterator) {
				const auto& header = iterator->second;
				std::string filename = header.getFileName();
				// Poco::UInt32 fileSize = header.getUncompressedSize();
				// std::cout << "filename: " << filename << "\nsize: " << fileSize << std::endl;

				ifs.clear(); // 清除流的错误状态
				ifs.seekg(0); // 将读取位置移动到流的开头

				auto data{ unzip(ifs, header) };
				selectControl(data, filename);
			}
		};
	public:
		PhiTaptapAPI(std::string_view sessionToken) {
			std::string
				filename{ "save" };
			
			this->m_sessionToken = sessionToken.data();
			const TapTapCloudConfig cloud_config{ loadCloudConfig() };
			m_headers = makeHeaders(cloud_config, m_sessionToken);

			std::filesystem::path dir_path{ Global::PlayerSavePath + "/" + m_sessionToken + "/" },
				file_path{ dir_path / filename };


			httplib::Error err{ httplib::Error::Success };

			web::http::client::http_client client(U(cloud_config.dataUrl));

			try{
				// 异步获取玩家自己信息的JSON
				std::future<void> get_player_info_thread = std::async(std::launch::async, [&] {
					web::http::http_request request_add_index(web::http::methods::GET);
					request_add_index.set_request_uri(ME_URI);
					for (auto& [key, value] : m_headers) {
						request_add_index.headers().add(key, value);
					}
					auto response = client.request(request_add_index).get();

					auto resp_status_code{ response.status_code() };

					if (resp_status_code == 200) {
						auto user_me_resp_str = response.extract_json().get().serialize();
						auto user_me_resp_json = Json::parse(user_me_resp_str);

						this->m_nickname = user_me_resp_json["nickname"].get<std::string>();
						this->m_userObjectId = user_me_resp_json["objectId"].get<std::string>();
					}
					else if (resp_status_code < 500 && resp_status_code >= 400) {
						uint16_t status_code = 1;
						switch (resp_status_code)
						{
						case 400:
							status_code = 4;
							break;
						default:
							break;
						}
						throw HTTPException("", resp_status_code, status_code);
					}
					else {
						throw HTTPException(""s, 500, 1);
					}
					});

				get_player_info_thread.get();

				std::string where_clause = "{\"user\":{\"__type\":\"Pointer\",\"className\":\"_User\",\"objectId\":\""
					+ this->m_userObjectId + "\"}}";

				std::string encoded_where = web::uri::encode_data_string(where_clause);

				std::string save_request_uri = cloud_config.saveListUri
					+ "?skip=0&limit=100&where=" + encoded_where
					+ "&include=cover,gameFile";

				web::http::http_request request_add_index(web::http::methods::GET);

				request_add_index.set_request_uri(save_request_uri);
				for (auto& [key, value] : m_headers) {
					request_add_index.headers().add(key, value);
				}
				auto response = client.request(request_add_index).get();
				auto resp_status_code{ response.status_code() };
				// 获取玩家存档的JSON
				if (resp_status_code == 200) {
					Json game_save_response = Json::parse(response.extract_json().get().serialize());
					if (!game_save_response.contains("results") || game_save_response["results"].empty()) {
						throw HTTPException("Cloud save list is empty", 404, 9);
					}
					auto selected_save = std::find_if(
						game_save_response["results"].begin(),
						game_save_response["results"].end(),
						[](const Json& item) { return item.contains("gameFile") && !item["gameFile"].is_null(); });
					if (selected_save == game_save_response["results"].end()) {
						throw HTTPException("Cloud save file doesn't exist", 404, 9);
					}
					this->m_game_save_info = *selected_save;

					// ======================================================================
					this->m_saveModel.userObjectId = this->m_game_save_info["user"]["objectId"].get<std::string>();
					this->m_saveModel.summary = this->m_game_save_info["summary"].get<std::string>();
					this->m_saveModel.objectId = this->m_game_save_info["objectId"].get<std::string>();
					this->m_saveModel.gameObjectId = this->m_game_save_info["gameFile"]["objectId"].get<std::string>();
					this->m_saveModel.checksum = this->m_game_save_info["gameFile"]["metaData"]["_checksum"].get<std::string>();
				} else {
					throw HTTPException(""s, 500, 1);
				}
				
			} catch (const nlohmann::json::type_error&) {
				// plan B启动
				if (Global::IsPlanB)
				{
					LogSystem::logInfo("悲观主义的一套planB");

					collectSave(file_path);
					// 突然终止
					return;
				}
				throw;
			} 
			catch (const HTTPException&) {
				throw;
			}
			catch (const std::exception&) {
				throw;
			};

			std::string
				save_url{ this->m_game_save_info["gameFile"]["url"].get<std::string>() },
				save_domain{},
				save_uri{};

			// 正则表达式解析域名和url
			std::regex url_regex("(https?://[^/]+)(/.*)");

			if (std::smatch url_match; std::regex_match(save_url, url_match, url_regex)) {
				save_domain = std::move(url_match[1]);
				save_uri = std::move(url_match[2]);
			}
			else throw std::runtime_error("Invalid URL");

			// 发送HTTP GET请求
			httplib::Client game_save_zip(save_domain);
			httplib::Result result_zip{ game_save_zip.Get(save_uri) };
			err = result_zip.error();
			if (err != httplib::Error::Success) {
				throw HTTPException(httplib::to_string(err), 500, 1);
			}
			else if (result_zip->status != 200) {
				throw HTTPException(httplib::to_string(err), 500, 1);
			}
			const std::string actual_checksum{ OtherUtil::stringToMD5(result_zip->body) };
			if (!this->m_saveModel.checksum.empty() && this->m_saveModel.checksum != actual_checksum) {
				throw HTTPException(
					"Cloud save checksum mismatch: expected "s + this->m_saveModel.checksum + ", actual " + actual_checksum,
					500,
					1);
			}

			// ==========================================
			// 存储文件(临时用)
			bool is_exists{ false }, do_save{false};
			SQL_Util::LocalDB << "select COUNT(sessionToken) from PlayerKey where sessionToken = ?;" << m_sessionToken >> is_exists;
			std::string key{ this->m_game_save_info["gameFile"]["key"].get<std::string>() };
			// 如果不存在字段
			if (!is_exists){
				// 第一个用户新建字段
				SQL_Util::LocalDB  << "insert into PlayerKey(sessionToken,key,nickname) values (?,?,?);"
					<< m_sessionToken
					<< key
					<< this->m_nickname;
				do_save = true;
				LogSystem::logInfo(std::format("已新增{}字段", m_sessionToken));
			}
			else {
				std::string key_old{};
				SQL_Util::LocalDB << "select key from PlayerKey where sessionToken = ?;" << m_sessionToken >> key_old;
				if (key_old != key) {
					// key不相同更新字段
					SQL_Util::LocalDB << "UPDATE PlayerKey SET key = ? WHERE sessionToken = ?"
						<< key
						<< m_sessionToken;
					do_save = true;
					LogSystem::logInfo(std::format("已更新{}的字段", m_sessionToken));
				}
			}
			// 是否需要存储呢
			if (do_save){
				namespace fs = std::filesystem;
				
				// 如果目录不存在，则创建它
				if (!fs::exists(dir_path)) {
					fs::create_directories(dir_path);
				}

				std::ofstream file(file_path, std::ios::binary | std::ios::trunc);
				if (file.is_open()) {
					// 将响应体写入文件
					file.write(result_zip->body.c_str(), result_zip->body.length());
					file.close();
					LogSystem::logInfo(std::format("已将{}存储到SQLite中", m_sessionToken));
				}
				else {
					file.close();
					throw HTTPException("Failed to save file to "s + file_path.string(), 500, 1);
				}
			}
			// ==========================================
			
			std::string saveData{ result_zip->body };
			collectSave(saveData);
		};

		~PhiTaptapAPI() noexcept {
			m_player_info.clear();
			m_game_save_info.clear();
		};

		std::string getNickname() const {
			return this->m_nickname;
		};

		auto GetSummary() const {
			return CloudSaveSummary(
				this->m_game_save_info["summary"].get<std::string>(),
				this->m_game_save_info["updatedAt"].get<std::string>(),
				this->m_nickname);
		};

		auto& getPlayerRecord() const {
			return this->m_player_record;
		}

		auto& getUserData() const {
			return this->m_user_data;
		}

		auto& getGameProgress() const {
			return this->m_gameProgress;
		}

		auto downloaDecodeZipFile() {
			return 0;
		}

		auto upload(const std::string& body) {
			const TapTapCloudConfig cloud_config{ loadCloudConfig() };
			std::size_t dataSize = body.size();
			std::string dataMD5 = OtherUtil::stringToMD5(body);
			// this->m_dataSize = body.size();
			// this->m_dataMD5 = OtherUtil::stringToMD5(body);
			Json resFileToken{};

			{
				web::http::client::http_client client(U(cloud_config.dataUrl));
				web::http::http_request request_add_index(web::http::methods::POST);
				request_add_index.set_request_uri(FILE_TOKENS_URI);
				for (auto& [key, value] : m_headers){
					request_add_index.headers().add(key, value);
				}
				Json contentBody{
					{"name", ".save"},
					{"__type", "File"},
					{"ACL", {{
					this->m_saveModel.userObjectId, {
						{"read", true},
						{"write", true}
					}
				}}},
					{"prefix", "gamesaves"},
					{"metaData", {
						{"size", dataSize},
						{"_checksum", dataMD5},
						{"prefix", "gamesaves"}
				  }}
				};

				request_add_index.set_body(web::json::value::parse(contentBody.dump()));

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				resFileToken = Json::parse(response.extract_json().get().serialize());
			}

			std::string tokenKey{ resFileToken["key"].get<std::string>() };

			std::string tokenKeyB64{ crow::utility::base64encode(tokenKey, tokenKey.length()) };
			std::string newGameObjectId{ resFileToken["objectId"].get<std::string>() };
			std::string authorization{ "UpToken " + resFileToken["token"].get<std::string>() };

			// fmt::print("tokenKey:{}\nnewGameObjectId:{}\nauthorization:{}\n", tokenKeyB64, newGameObjectId, authorization);

			// Json

			std::string uploadId{};
			{
				web::http::client::http_client client(U(cloud_config.uploadUrl));
				web::http::http_request request_add_index(web::http::methods::POST);
				request_add_index.set_request_uri(std::format("/buckets/rAK3Ffdi/objects/{}/uploads", tokenKeyB64));
				request_add_index.headers().add("Authorization", authorization);

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };
				std::exchange(resBody, resBody[0]);
				uploadId = resBody["uploadId"].get<std::string>();
			}

			// fmt::print("\nuploadId: {}\n", uploadId);
			
			std::string etag{};
			{
				web::http::client::http_client client(U(cloud_config.uploadUrl));
				web::http::http_request request_add_index(web::http::methods::PUT);
				request_add_index.set_request_uri(std::format("/buckets/rAK3Ffdi/objects/{}/uploads/{}/1", tokenKeyB64, uploadId));
				request_add_index.headers().add("Authorization", authorization);
				request_add_index.headers().add("Content-Type", "application/octet-stream");

				request_add_index.set_body(body);

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };
				std::exchange(resBody, resBody[0]);
				etag = resBody["etag"].get<std::string>();
			}
			// fmt::print("\netag: {}\n", etag);

			{
				web::http::client::http_client client(U(cloud_config.uploadUrl));
				web::http::http_request request_add_index(web::http::methods::POST);
				request_add_index.set_request_uri(std::format("/buckets/rAK3Ffdi/objects/{}/uploads/{}", tokenKeyB64, uploadId));
				request_add_index.headers().add("Authorization", authorization);
				request_add_index.headers().add("Content-Type", "application/json");

				request_add_index.set_body(web::json::value::parse("{\"parts\":[{\"partNumber\":1,\"etag\":\"" + etag + "\"}]}"));

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };
				// std::cout << resBody << std::endl;
			}

			{
				web::http::client::http_client client(U(cloud_config.dataUrl));
				web::http::http_request request_add_index(web::http::methods::POST);
				request_add_index.set_request_uri(FILE_CALLBACK_URI);
				for (auto& [key, value] : m_headers) {
					request_add_index.headers().add(key, value);
				}
				request_add_index.headers().add("Content-Type", "application/json");

				request_add_index.set_body(web::json::value::parse("{\"result\":true,\"token\":\"" + tokenKeyB64 + "\"}"));

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };

				// std::cout << resBody << std::endl;
			}

			{
				// 获取时间相关
				auto currentTimePoint = std::chrono::system_clock::now();
				std::time_t time = std::chrono::system_clock::to_time_t(currentTimePoint);
				auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(currentTimePoint.time_since_epoch()) % 1000;
				std::tm tmInfo = *std::gmtime(&time);
				std::ostringstream oss;
				oss << std::put_time(&tmInfo, "%Y-%m-%dT%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << milliseconds.count() << "Z";

				// oss.str()

				// 发送请求
				web::http::client::http_client client(U(cloud_config.dataUrl));
				web::http::http_request request_add_index(web::http::methods::PUT);
				request_add_index.set_request_uri("/1.1/classes/_GameSave/" + this->m_saveModel.objectId + "?");
				for (auto& [key, value] : m_headers) {
					request_add_index.headers().add(key, value);
				}
				request_add_index.headers().add("Content-Type", "application/json");

				request_add_index.set_body(web::json::value::parse("{\"summary\":\"" + this->m_saveModel.summary + "\",\"modifiedAt\":{\"__type\":\"Date\",\"iso\":\"" + oss.str() + "\"},\"gameFile\":{\"__type\":\"Pointer\",\"className\":\"_File\",\"objectId\":\"" + newGameObjectId + "\"},\"ACL\":{\"" + this->m_saveModel.userObjectId + "\":{\"read\":true,\"write\":true}},\"user\":{\"__type\":\"Pointer\",\"className\":\"_User\",\"objectId\":\"" + this->m_saveModel.userObjectId + "\"}}"));

				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };

				// std::cout << resBody << std::endl;
			}

			{
				web::http::client::http_client client(U(cloud_config.dataUrl));
				web::http::http_request request_add_index(web::http::methods::DEL);
				request_add_index.set_request_uri("/1.1/files/" + this->m_saveModel.gameObjectId);
				for (auto& [key, value] : m_headers) {
					request_add_index.headers().add(key, value);
				}
				auto response = client.request(request_add_index).get();

				if (response.status_code() >= 300 or response.status_code() < 200) {
					auto error = response.extract_string().get();
					throw self::HTTPException(error, 500, 12);
				}

				Json resBody{ Json::parse(response.extract_json().get().serialize()) };

				// std::cout << resBody << std::endl;
			}

			return true;
		}
	};
}

#endif
