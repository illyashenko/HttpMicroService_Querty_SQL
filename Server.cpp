#include "IServer.h"

vector<AppSettings> vector_users;

HttpServer::HttpServer()
{
}

HttpServer::~HttpServer()
{
}

int GetAuth(string& login, string& pass) 
{
	auto nAppCount = boolinq::from(vector_users).
		where([&](const AppSettings& appSetting)
			      { return appSetting.login == login && appSetting.pass == pass;
			      }).count();

	return nAppCount;
}

string GetSerializeString(vector<map<string, string>>& vectorResponse)
{
	string str_array;
	int a = 1;
	str_array += '[';
	for (map<string, string> it_vector : vectorResponse)
	{
		string struct_map;
		int s = 1;
		struct_map += '{';
		for (const auto& it : it_vector)
		{
			struct_map += '"' + it.first + '"' + " : " + '"' + it.second + '"';
			struct_map += (it_vector.size() != s) ? "," : "";
			s++;
		}
		struct_map += '}';
		str_array += struct_map;
		str_array += (vectorResponse.size() != a) ? "," : "";
		a++;
	}
	str_array += ']';
	return str_array;
}

void FillConnectionData(json& j, string& connectionString, string& stringRequest)
{
	for (json::iterator it = j.begin(); it != j.end(); ++it)
	{
		if (it.key() == "ConnectionString")
		{
			connectionString = it.value();
		}
		if (it.key() == "StringRequest")
		{
			stringRequest = it.value();
		}
	}
}

void Controller_api(const Request& req, Response& res)
{
	bool mistake = false;
	auto j = json::parse(req.body);

	//*****authorization*******
	string login = req.get_param_value("log");
	string pass = req.get_param_value("pass");

	if (GetAuth(login, pass) == 0)
	{
		res.status = 401;
		return;
	}

	string connectionString;
	string stringRequest;

	FillConnectionData(j, connectionString, stringRequest);

	wstring conString(connectionString.begin(), connectionString.end());
	wstring reqString(stringRequest.begin(), stringRequest.end());

	vector<map<string, string>> listData = main_sql(&conString[0], &reqString[0], mistake);

	if (mistake)
	{
		res.status = 400;
		return;
	}

	res.set_header("Content-Type", "application/json; charset=Windows-1251");
	res.body = GetSerializeString(listData);
}

void HttpServer::Start()
{
	Server serv;
	serv.Post("/", Controller_api);

	string path = GetPath();
	int port = GetPort();

	serv.listen(path.c_str(), port);
}

void HttpServer::FillAppSetting()
{
	pugi::xml_document doc;
	doc.load_file(L"..\\API.SQL.version\\ServerSettings.xml");

	for (pugi::xml_node el : doc.child("Users").children("user"))
	{
		AppSettings app;

		app.login = el.attribute("login").value();
		app.pass = el.attribute("pass").value();
		
		vector_users.push_back(app);
	}

	for (pugi::xml_node el : doc.child("Users").children("ServerPath"))
	{
		string path = el.attribute("path").value();
		string port_str = el.attribute("port").value();
		int port = stoi(port_str);

		SetServerPath(path);
		SetServerPort(port);
	}
}

string HttpServer::GetPath()
{
	return path_;
}

int HttpServer::GetPort()
{
	return port_;
}

void HttpServer::SetServerPath(string path)
{
	this->path_ = path;
}

void HttpServer::SetServerPort(int port)
{
	this->port_ = port;
}
