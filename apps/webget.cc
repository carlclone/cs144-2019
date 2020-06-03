#include "socket.hh"
#include "util.hh"
#include "tcp_sponge_socket.hh"

#include <cstdlib>
#include <iostream>

using namespace std;

void get_URL(const string &host, const string &path) {

    // Your code here.

    // You will need to connect to the "http" service on
    // the computer whose name is in the "host" string,
    // then request the URL path given in the "path" string.

    // Then you'll need to print out everything the server sends back,
    // (not just one call to read() -- everything) until you reach
    // the "eof" (end of file).


    CS144TCPSocket sock{};
    sock.connect(Address(host, "http"));
    string input("GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\n\r\n");
    sock.write(input);
    sock.shutdown(SHUT_WR);
    while (!sock.eof())
        cout << sock.read();
    sock.close();
    sock.wait_until_closed();


    //大约10行代码..
    //测试 ./apps/webget cs144.keithw.org /hello          //make check webget       //make check

    // How
    // does this compare to what you see when visiting http://cs144.keithw.org/hello in a
    // Web browser? How does it compare to the results from Section 2.1? Feel free to
    // experiment—test it with any http URL you like!

    //实现webget ,  HTTP请求体的格式 , TCPSocket , Address class , 每行结束\r\n
    //构建http请求体
//    std::string httpString;
//    httpString = "GET "+path+" HTTP/1.0" +"\r\n" +
//                 "Host:"+host +"\r\n";
//    cout <<httpString;
//
//
//    //建立tcp连接
//    TCPSocket tcpSocket = TCPSocket{};
//    Address address = Address{host, "http"};
//    tcpSocket.connect(address);
//
//
//    //发送(write)字节流
//    // write完之后要调用TCPSocket的SHUT WR关掉这个方向的发送 , 否则另一端会等待一段时间才开始处理
//    tcpSocket.write(httpString);
//    tcpSocket.shutdown(SHUT_WR);
//
//    //接收(read)字节流
//    // read要read到EOF才算结束 , 不能只read一次
//    //eof是一个信号,不是字符串结尾的特殊字符
//    string bytes;
//    while (!tcpSocket.eof()) {
//        bytes += tcpSocket.read();
//    }
//    cout << bytes;
//    tcpSocket.close();

//    cerr << "Function called: get_URL(" << host << ", " << path << ").\n";
//    cerr << "Warning: get_URL() has not been implemented yet.\n";
}

int main(int argc, char *argv[]) {
    try {
        if (argc <= 0) {
            abort();  // For sticklers: don't try to access argv[0] if argc <= 0.
        }

        // The program takes two command-line arguments: the hostname and "path" part of the URL.
        // Print the usage message unless there are these two arguments (plus the program name
        // itself, so arg count = 3 in total).
        if (argc != 3) {
            cerr << "Usage: " << argv[0] << " HOST PATH\n";
            cerr << "\tExample: " << argv[0] << " stanford.edu /class/cs144\n";
            return EXIT_FAILURE;
        }

        // Get the command-line arguments.
        const string host = argv[1];
        const string path = argv[2];

        // Call the student-written function.
        get_URL(host, path);
    } catch (const exception &e) {
        cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
