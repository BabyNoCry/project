#include "Jarvis.hpp"

using namespace std;
int main()
{
	/*Retard r;
	string strin;
	volatile bool quit = false;
	while(!quit){
	cout<<"��$*";
	cin >> strin;
	std::string s = r.Talk(strin);*/
	Jarvis *rb = new Jarvis();
	if (!rb->LoadEtc())
	{
		return 1;
	}

	rb->Run();
	//cout<<"�˹����ϣ�"<<s<<endl;

	return 0;

}