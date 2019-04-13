#include <iostream>

using namespace std;

int main()
{
    string info("hello world");
    cout << "HTTP/1.1 200 OK\r\n";
    cout << "Content-Type: text/html\r\n";
    cout << "Content-Length: " << info.size() << "\r\n";
    cout << "\r\n";
    cout << info;
}