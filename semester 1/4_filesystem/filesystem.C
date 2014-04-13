#include <unistd.h>
#include <string>
#include <iostream>
#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <csignal>
#include <sys/fcntl.h>


using std::list;
using std::string;
using std::map;
using std::set;
using std::cin;
using std::cout;
using std::endl;
using std::signal;
using std::max_element;


const size_t BLOCK_SIZE = 10;
const size_t MAX_THREADS_COUNT = 1000;

class File;
typedef map<string, File*> files_map;


sig_atomic_t ctrl_c_pressed = 0;

void sighandler(int s)
{
    ctrl_c_pressed = 1;
}


class Backery
{
public:
    Backery()
    {
        for (size_t i = 0; i < MAX_THREADS_COUNT; ++i)
        {
            choosing[i] = false;
            turn[i] = 0;
        }
    }

    void Lock(const size_t threadNum)
    {
        choosing[threadNum] = true; 
        turn[threadNum] = *max_element(turn, turn + MAX_THREADS_COUNT) + 1; 
        choosing[threadNum] = false; 

        for (size_t i = 0; i < MAX_THREADS_COUNT; ++i)
        {
            while (choosing[i]);
            while (turn[i] != 0 && (turn[i] < turn[threadNum] || (turn[i] == turn[threadNum] && i < threadNum)));
        }
    }

    void Unlock(const size_t threadNum)
    {
        turn[threadNum] = 0;
    }

private:
    bool choosing[MAX_THREADS_COUNT];
    size_t turn[MAX_THREADS_COUNT];
};


class File : public Backery
{
public:
    File()
        : Backery()
    {
        blocks.push_back(new FileBlock());
    }

    void Print() const
    {
        for (list<FileBlock*>::const_iterator i = blocks.cbegin(); i != blocks.cend(); ++i)
            (*i)->Print();
        cout << endl;
    }
    
    void AddData(const string& data)
    {
        for (size_t i = 0; i < data.length(); ++i)
        {
            if (blocks.back()->IsFull())
                blocks.push_back(new FileBlock());
        
            blocks.back()->AddByte(data[i]);
        }
    }

    size_t GetSize() const
    {
        size_t fullSize = 0;
        for (list<FileBlock*>::const_iterator i = blocks.cbegin(); i != blocks.cend(); ++i)
            fullSize += (*i)->GetSize();

        return fullSize;
    }

    ~File()
    {
        for (list<FileBlock*>::const_iterator i = blocks.cbegin(); i != blocks.cend(); ++i)
            delete *i;
    }

private:
    class FileBlock
    {
    public:    
        FileBlock()
            : bytes_filled(0)
        {
            for (size_t i = 0; i < BLOCK_SIZE + 1; ++i)
                data[i] = 0;
        }

        void AddByte(const char c)
        {
            data[bytes_filled] = c;
            ++bytes_filled;
        }

        bool IsFull() const
        {
            return bytes_filled == BLOCK_SIZE;
        }

        size_t GetSize() const
        {
            return bytes_filled;
        }

        void Print() const
        {
            cout << data;
        }

    private:
        size_t bytes_filled;
        char data[BLOCK_SIZE];
    }; 


    list<FileBlock*> blocks;
};



class FileSystem
{
public:
    void PrintFile(const string& filename) const
    {
        if (FileExists(filename))
            files.find(filename)->second->Print();
        else
            cout << "File not found\n";
    }
    
    bool FileExists(const string& filename) const
    {
        return files.find(filename) != files.end();
    }
    
    void CreateFile(const string& filename)
    {
        if (!FileExists(filename))
            files[filename] = new File();
    }

    void RemoveFile(const string& filename)
    {
        if (FileExists(filename))
            files.erase(filename);
    }

    void WriteToFile(const string& filename)
    {        
        string str = "";

        int flag = fcntl(stdin->_fileno, F_GETFL);
        fcntl(stdin->_fileno, F_SETFL, flag | O_NONBLOCK);
        
        ctrl_c_pressed = 0;
        while (!ctrl_c_pressed)
        {
            char c;
            if (read(0, &c, 1) > 0)
                str += c;
        }
        ctrl_c_pressed = 0;
        
        fcntl(stdin->_fileno, F_SETFL, flag);

        File* newFile = new File();
        newFile->AddData(str);
        files[filename] = newFile;
    }

    void PrintAvailibleFiles() const
    {
        for (files_map::const_iterator iter = files.cbegin();
             iter != files.cend();
             ++iter)
        {
            cout << iter->first << endl;
        }
    }

    void PrintFileSize(const string& filename) const
    {
        if (FileExists(filename))
            cout << files.find(filename)->second->GetSize() << endl;
        else
            cout << "File not found\n";
    }

    friend void* Traverse(void*);
    
    ~FileSystem()
    {
        for (files_map::iterator i = files.begin(); i != files.end(); ++i)
        {
            File* file = i->second;
            delete file;
        }
    }

private:
    files_map files;
};


struct TraverseArgs
{
public:
    TraverseArgs(const FileSystem& fs, const size_t threadNum)
        : fs(fs)
        , threadNum(threadNum)
    { }

    const FileSystem& fs;
    const size_t threadNum;
};


void* Traverse(void* arg)
{
    TraverseArgs* args = static_cast<TraverseArgs*>(arg); 
    const size_t threadNum = args->threadNum;
    const FileSystem& fs = args->fs;
    delete args;

    for (files_map::const_iterator iter = fs.files.begin();
        iter != fs.files.end();
        ++iter)
    {
        const string& filename = iter->first;
        File* file = iter->second;
        file->Lock(threadNum);
        printf("\"%s\": %zu\n", filename.c_str(), file->GetSize());
        file->Unlock(threadNum);
    }
}


void TestSystem(FileSystem &fs, const size_t threadsCount)
{
    if (threadsCount > MAX_THREADS_COUNT)
    {
        cout << "You can't create more than " << MAX_THREADS_COUNT << " threads" << endl;
        return;
    }

    pthread_t* threads = new pthread_t[threadsCount];
    for (size_t i = 0; i < threadsCount; ++i)
        pthread_create(&threads[i], NULL, Traverse, new TraverseArgs(fs, i));

    for (size_t i = 0; i < threadsCount; ++i)
        pthread_join(threads[i], NULL);

    delete[] threads;
}


void PrintHelp()
{
    cout << "Usage:\n"
            "  ls\n"
            "  cat> %filename% (finish input by ctrl-C)\n"
            "  cat %filaneme%\n"
            "  rm %filename%\n"
            "  touch %filename%\n"
            "  getsize %filename% (prints file size in bytes)\n"
            "  test %threads_count%\n"
            "  exit\n"
            "  help\n"
            "\n";
}


int main(void)
{
    signal(SIGINT, sighandler);

    FileSystem fs;
    
    set<string> validCommands = {"ls", "cat>", "cat", "rm", "touch", "rm", "exit", "getsize", "test", "help"};

    PrintHelp();

    while (true)
    {
        cout << ">";
    
        string command;
        cin >> command;

        if (validCommands.find(command) == validCommands.end())
        {
            cout << "Can't recognize a command" << endl;
            continue;
        }

        if (command == "exit")
            break;

        if (command == "help")
        {
            PrintHelp();
        }
        else if (command == "ls")
        {
            fs.PrintAvailibleFiles();
        }
        else if (command == "test")
        {
            size_t threadsCount;
            cin >> threadsCount;
            TestSystem(fs, threadsCount);
        }
        else
        {
            string arg;
            cin >> arg;
            if (command == "cat>")
            {
                fs.WriteToFile(arg);
                cout << endl;
            }
            else if (command == "cat")
                fs.PrintFile(arg);
            else if (command == "touch")
                fs.CreateFile(arg);
            else if (command == "rm")
                fs.RemoveFile(arg);
            else if (command == "getsize")
                fs.PrintFileSize(arg);
        }
    }

    return EXIT_SUCCESS;
}


