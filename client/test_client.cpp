#include "conv_client.h"
#include <iostream>

using namespace std;
using namespace hzd;

int main()
{
    conv_client c("127.0.0.1",9999);
    c.connect();
    string s = "i am client";
    bool x = c.send(s);
    cout << x << endl;
    header_type type;
    x = c.recv(s,type);
    cout << s << endl;
}