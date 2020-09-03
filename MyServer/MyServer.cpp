#include <iostream>
#include <uwebsockets/App.h>
#include <thread>
#include <algorithm>
#include <string>
using namespace std;

struct UserConnection
{
	unsigned long user_id;
	string user_name="x";
};



int main()
{
	atomic_ulong lastest_user_id = 10; //тип даннных atomic созданы для работы с неколькью потоками

	vector<thread*> threads(thread::hardware_concurrency());

	transform(threads.begin(), threads.end(), threads.begin(), [&lastest_user_id](auto* thr)
	{
		return new thread([&lastest_user_id]()
		{
			// "ws://127.0.0.1/"
			uWS::App().ws<UserConnection>("/*", //путь к серверу
			{
				.open = [&lastest_user_id](auto* ws) //что делать при подключении пользователя
				{
					UserConnection* data = (UserConnection*)ws->getUserData();
					lastest_user_id++;
					data->user_id = lastest_user_id;
					cout << "\n New youser connecte, id this youser is "<< lastest_user_id <<"\n If u want choose name enter command 'My_Name' and your name";
					ws->subscribe("newusers"); // подписка на канал, отдельно канал создавать не нужно
					ws->subscribe("user#" + to_string(data->user_id));
					ws->publish("newusers", "Hey, we have new user. His ID:" + to_string(data->user_id));
				},
				.message = [&lastest_user_id](auto* ws, string_view message, uWS::OpCode opCode) //что делать при получении сообщения
				{
					UserConnection* data = (UserConnection*)ws->getUserData();
					auto start = message.substr(0,7);

					if (start.compare("My_Name")==0) // проверка регистрации юзера
					{
						auto name = message.substr(8);
						if (name.compare("x") == 0) //&& start.find(",")>-1
						{
							cout << "\nYouser ID " << data->user_id << ", please choosee enother name ";
						}
						else
						{
							data->user_name = name;
							cout << "\n U choose name: " << data->user_name;
							string z = "\nYouser ID " + to_string(data->user_id) + " choosee name, his new name: " + data->user_name;
							ws->publish("newusers",z,opCode, false); // отпрака сообщения z в канал newusers
						}
					}
					else if (start.compare("Send_To") == 0)
					{						
						auto mail = message.substr(8);
						int pos = mail.find(",");
						if (pos>0)
						{
							auto id_str = mail.substr(0, pos);
							auto usr_mess = mail.substr(pos + 1);
							ws->publish("user#" + string(id_str), usr_mess, opCode, false);
						}
					}
					else
					{
						if (data->user_name.compare("x")==0)
						{
							cout << "\nUser number " << data->user_id << " say: " << message;

						}
						else
						{
							cout << "\nUser " << data->user_name << " say: " << message;
						}
					}

				}
			}).listen(666, [](auto* token) //адрес в этом случае 666
			{
					if (token) // проверка подключения
					{
						cout << "\nServer started on port 666" ; // вывод сообщения о начале работы

					}
					else 
					{
						cout << "\nServer critical error ;(";
					}
			}).run();
		});
	});

	for_each (threads.begin(), threads.end(), [](auto* thr) {thr->join();});  //завершение потоков
}



