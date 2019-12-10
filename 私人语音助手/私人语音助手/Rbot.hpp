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
//��һ��ʱ���
class Util {
public:
	static bool Exec(std::string command, bool is_print)
	{
		if (!is_print)
		{
			command += ">/dev/null 2>&1";
		}

		FILE *fp = popen(command.c_str(), "r");//popen����
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
	std::string api_key;//�����˱��
	std::string user_id;//�û�id
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
		int status_code = client.post(url, nullptr, json, nullptr, &response);//��������
		if (status_code != CURLcode::CURLE_OK)
		{
			std::cerr << "ʧ������" << std::endl;
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
		bool res = cr->parse(str.data(), str.data() + str.size(), &root, &errs);//�����л�
		if (!res || !errs.empty())
		{
			std::cerr << "�����л�ʧ��" << std::endl;
			return "";
		}
		int code = root["intent"]["code"].asInt();
		if (!IsCodeLegal(code))
		{
			std::cerr << "response code ʧ��" << std::endl;
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
		//std::cout<<"�յģ���"<<message<<std::endl;

		std::string json = Message2json(message);//message -> json ���
		//  std::cout<<"�յģ���"<<json<<std::endl;

		std::string response = json2Rerard(json);//json ->Retard  ��������RequestTL
		// std::cout<<"�յģ���"<<response<<std::endl;
		std::string echo_response = ToMessage(response);//Retard ->message jsonת����Ϣ JsonToEchomessage
		//std::cout<<"�յģ���"<<echo_response<<std::endl;
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
			result = false;//�Լ����ϵ�
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
			std::cerr << "ʶ��ʧ��" << std::endl;
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
		options["spd"] = "10";//����0-15
		options["pit"] = "5";//��� 0-15
		options["vol"] = "10";//0-15
		options["per"] = "5";//1 0 3 4 ���� 106 Сͯ110 С�� 111 �׶� 103 С�� 5
		options["aue"] = "3";//3Ϊmap3 6 Ϊwav
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
		bool ret = Util::Exec(command, false);//���ﻻ���ҵ�����

		std::cout << "debug over" << std::endl;
		return ret;
	}

	bool Play()
	{
		std::string command = "aplay ";//cvlcû���� ��aplay��һ��
		command += TTS_PATH;
		bool ret = Util::Exec(command, false);
		return ret;
	}
public:
	Jarvis() {

	}

	bool LoadEtc()//����etc�ļ�
	{
		std::ifstream in(CMD_ETC);
		if (!in.is_open()) {
			std::cerr << "��etc�ļ�ʧ��" << std::endl;
			return false;
		}
		std::string sep = ":";
		char line[1024];
		while (in.getline(line, sizeof(line))) {
			std::string str = line;
			std::size_t pos = str.find(sep);
			if (std::string::npos == pos)
			{
				std::cerr << "�Ҳ����ַ���" << std::endl;
				continue;
			}
			std::string k = str.substr(0, pos);
			std::string v = str.substr(pos + sep.size());
			k += "��";
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
					std::cout << "��%    " << message << std::endl;
					if (message == "�˳���")
					{
						std::string quit_message = "ȷ���˳������ζԻ��ļ�����������/usr/local/lib/sence.txt��";
						std::cout << "�˹�����%%" << quit_message << std::endl;
						if (sr.TTS(quit_message))
						{
							this->Play();
						}

						exit(0);
					}

					//1.�Ƿ���command
					std::string echo = rt.Talk(message);
					std::cout << "�˹�����%%" << echo << std::endl;
					if (sr.TTS(echo))
					{
						this->Play();
					}
					//std::cout<<"�˹�����%%"<<echo<<std::endl;
				//2.�����ϻ�����
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