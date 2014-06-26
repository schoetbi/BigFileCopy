#include <iostream>
#include "checksumfile.h"

using namespace std;

void DisplayHelp()
{
    cout << "usage is: bfcp <cmd> <param1> <...>" << endl;
    cout << "Commands:\n";
    //cout << "  - help : Display this help message\n";
    cout << "  - hash fileName       : Create hash file\n";
    cout << "  - snapshot src target : Create a snapshot file\n";
    // more will follow
}

int main(int argc, char **argv)
{
    // usage is
    // bfcp <cmd> <param1> <...>
    if (argc <= 1)
    {
        DisplayHelp();
        return 1;
    }

    string command(argv[1]);
    if (command == "hash")
    {
        if (argc < 3)
        {
            DisplayHelp();
            return 2;
        }
        string fileName = argv[2];
        cout << "Creating hashfile for " << fileName << "...";
        CheckSumFile csf(fileName);
        csf.WriteHashFile();
        cout << "Ok\n";
        return 0;
    }
    else if (command == "snapshot")
    {
        if (argc < 4)
        {
            DisplayHelp();
            return 3;
        }
        string src = argv[2];
        string target = argv[3];
        cout << "Creating snapshot " << src << "->" << target << " ...";
        CheckSumFile csf(src);
        csf.UpdateSnapshot(target);
        cout << "Ok\n";
        return 0;
    }
    else
    {
        DisplayHelp();
        return 3;
    }
}
