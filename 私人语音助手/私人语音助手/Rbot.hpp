#pragma once
#include <map>
#include <cstdio>
#include <sstream>
#include <json/json.h>
#include <iostream>
#include <string>
#include <memory>
#include "base/http.h"
#include <unistd.h>

#include "speech.h"
#include <unordered_map>
#define TTS_PATH "temp_file/tts.mp3"
#define ASR_PATH "temp_file/asr.wav"
#define CMD_ETC "command.etc"
//加一个时间戳
class Util {
public:
	static bool Exec(std::string command, bool is_print)
	{
		if (!is_print)
		{
			command += ">/dev/null 2>&1";
		}

		FILE *fp = popen(command.c_str(), "r");//popen作用
		if (nullptr == fp)
		{
			std::cerr << "popen exec" << command << "\'Error" << std::endl;
			return false;
		}
		if (is_print)
		{
			char ch;
			while (fread(&ch, 1, 1, fp) > 0)
			{
				fwrite(&ch, 1, 1, stdout);
			}
		}
		pclose(fp);
		return true;

	}

};

class Retard {
private:
	std::string api_key;//机器人编号
	std::string user_id;//用户id
	aip::HttpClient client;
	std::string url;


	bool IsCodeLegal(int code)
	{
		bool result = false;
		switch (code) {
		case 5000: break;
		case 10004:result = true;
			break;
		default:
			result = true;
			break;
		}

		return result;

	}

	std::string Message2json(std::string &message)
	{

		Json::Value root;
		Json::StreamWriterBuilder rb;
		std::ostringstream ss;

		Json::Value _item;

		_item["text"] = message;

		Json::Value item;
		item["inputText"] = _item;


		root["reqType"] = 0;
		root["perception"] = item;
		item.clear();

		item["apiKey"] = api_key;
		item["userId"] = user_id;

		root["userInfo"] = item;
		std::unique_ptr<Json::StreamWriter> hb(rb.newStreamWriter());
		hb->write(root, &ss);
		//std::cout<<"message2josn:"<<ss.str()<<std::endl;
		return ss.str();

	}

	std::string json2Rerard(std::string &json)
	{
		std::string response;
		int status_code = client.post(url, nullptr, json, nullptr, &response);//发起请求
		if (status_code != CURLcode::CURLE_OK)
		{
			std::cerr << "失败请求" << std::endl;
			return "";
		}

		return response;
	}

	std::string ToMessage(std::string &str)
	{
		JSONCPP_STRING errs;
		Json::Value root;
		Json::CharReaderBuilder rb;
		std::unique_ptr<Json::CharReader> const cr(rb.newCharReader());
		bool res = cr->parse(str.data(), str.data() + str.size(), &root, &errs);//反序列化
		if (!res || !errs.empty())
		{
			std::cerr << "反序列化失败" << std::endl;
			return "";
		}
		int code = root["intent"]["code"].asInt();
		if (!IsCodeLegal(code))
		{
			std::cerr << "response code 失败" << std::endl;
			return "";
		}
		Json::Value item = root["results"][0];
		std::string msg = item["values"]["text"].asString();
		return msg;

	}

public:
	Retard(std::string id = "1")
	{
		url = "http://openapi.tuling123.com/openapi/api/v2";
		api_key = "b247db8503af4248afa3ff84b62f273a";
		user_id = id;
	}
	std::string Talk(std::string message)
	{
		//std::cout<<"空的？？"<<message<<std::endl;

		std::string json = Message2json(message);//message -> json 完成
		//  std::cout<<"空的？？"<<json<<std::endl;

		std::string response = json2Rerard(json);//json ->Retard  发起请求RequestTL
		// std::cout<<"空的？？"<<response<<std::endl;
		std::string echo_response = ToMessage(response);//Retard ->message json转成消息 JsonToEchomessage
		//std::cout<<"空的？？"<<echo_response<<std::endl;
		return echo_response;
	}

	~Retard()
	{
	}



};

class SpeechRec {
private:
	std::string app_id;
	std::string api_key;
	std::string secret_key;
	aip::Speech* client;
private:

	bool IsCodeLegal(int code)
	{
		bool result = false;
		switch (code) {
		case 0: result = true;
			break;
		default:
			result = false;//自己加上的
			break;
		}
		return result;
	}
public:
	SpeechRec()
	{
		app_id = "16870310";
		api_key = "zyo8EhFRp4IIqo6wlqWFEz0b";
		secret_key = "pVRWZOLkcBuOe4FrZIdBLUehjblHxkgk";
		client = new aip::Speech(app_id, api_key, secret_key);
	}
	bool ASR(std::string path, std::string &out)
	{
		std::map<std::string, std::string> options;
		options["dev_pid"] = "1536";
		std::string file_content;
		aip::get_file_content(ASR_PATH, &file_content);
		Json::Value result = client->recognize(file_content, "wav", 16000, options);

		//std::cout<<"debug"<<result.toStyledString()<<std::endl;
		//std::cout<<result["result"]<<std::endl;
		int code = result["err_no"].asInt();
		if (!IsCodeLegal(code))
		{
			std::cerr << "识别失败" << std::endl;
			return false;
		}



		out = result["result"][0].asString();//??

		return true;

	}

	bool TTS(std::string message)
	{
		bool ret;
		std::ofstream ofile;
		std::string ret_file;
		std::map<std::string, std::string> options;
		ofile.open(TTS_PATH, std::ios::out | std::ios::binary);
		options["spd"] = "10";//语速0-15
		options["pit"] = "5";//语调 0-15
		options["vol"] = "10";//0-15
		options["per"] = "5";//1 0 3 4 博文 106 小童110 小萌 111 米朵 103 小娇 5
		options["aue"] = "3";//3为map3 6 为wav
		Json::Value result = client->text2audio(message, options, ret_file);
		if (ret_file.empty()) {
			ofile << ret_file;
			ofile.close();
			ret = true;
		}
		else {
			std::cerr << result.toStyledString() << std::endl;
			ret = false;
			ofile.close();
		}
		return ret;
	}

	~SpeechRec()
	{
	}

};

class Jarvis {
private:
	Retard rt;
	SpeechRec sr;
	std::unordered_map<std::string, std::string>	commands;

	bool Record()
	{
		std::cout << "debug: " << "Record...." << std::endl;
		std::string command = "sudo arecord -t wav -c 1 -r 16000 -d 5 -f S16_LE ";
		command += ASR_PATH;
		bool ret = Util::Exec(command, false);//这里换上我的命令

		std::cout << "debug over" << std::endl;
		return ret;
	}

	bool Play()
	{
		std::string command = "aplay ";//cvlc没法用 用aplay顶一下
		command += TTS_PATH;
		bool ret = Util::Exec(command, false);
		return ret;
	}
public:
	Jarvis() {

	}

	bool LoadEtc()//配置etc文件
	{
		std::ifstream in(CMD_ETC);
		if (!in.is_open()) {
			std::cerr << "打开etc文件失败" << std::endl;
			return false;
		}
		std::string sep = ":";
		char line[1024];
		while (in.getline(line, sizeof(line))) {
			std::string str = line;
			std::size_t pos = str.find(sep);
			if (std::string::npos == pos)
			{
				std::cerr << "找不到字符串" << std::endl;
				continue;
			}
			std::string k = str.substr(0, pos);
			std::string v = str.substr(pos + sep.size());
			k += "。";
			commands.insert(std::make_pair(k, v));

		}
		std::cerr << "Load command etc doen ...... sucess" << std::endl;

		in.close();
		return	true;
	}


	bool IsCommand(std::string message, std::string &cmd)
	{
		auto iter = commands.find(message);
		if (iter == commands.end())
		{
			return false;
		}

		cmd = iter->second;

	}
	void Run()
	{
		volatile  bool quit = false;
		while (!quit)
		{
			if (this->Record())
			{
				std::string message;
				if (sr.ASR(ASR_PATH, message))
				{
					std::string cmd = "";
					if (IsCommand(message, cmd)) {
						std::cout << "[Jarvis@localhost]%" << cmd << std::endl;
						Util::Exec(cmd, true);
						continue;
					}
					std::cout << "我%    " << message << std::endl;
					if (message == "退出。")
					{
						std::string quit_message = "确定退出，本次对话文件副本保存于/usr/local/lib/sence.txt。";
						std::cout << "人工智障%%" << quit_message << std::endl;
						if (sr.TTS(quit_message))
						{
							this->Play();
						}

						exit(0);
					}

					//1.是否是command
					std::string echo = rt.Talk(message);
					std::cout << "人工智障%%" << echo << std::endl;
					if (sr.TTS(echo))
					{
						this->Play();
					}
					//std::cout<<"人工智障%%"<<echo<<std::endl;
				//2.给智障机器人
				}

				//else{
				//std::cerr<<"Recognize fault"<<std::endl;}

			}
			else {
				std::cerr << "Record faill" << std::endl;
			}
			sleep(2);
		}

	}
	~Jarvis() {
	}
};