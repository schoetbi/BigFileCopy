#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
	if (argc != 3)
	{
		cout << "wrong usage" << endl;
		return -1;
	}

	path in(argv[1]);
	if (!exists(in))
	{
		cout << "input file does not exist " << endl;
		return -2;
	}

	cout << "creating hash file for " << in.string() << endl;
	createHashFile(in);


}
