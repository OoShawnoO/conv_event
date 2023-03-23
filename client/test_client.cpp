#include "conv_client.h"
#include <iostream>

using namespace std;
using namespace hzd;

int main()
{
    conv_client c("127.0.0.1",9999);
    c.connect();
    string s = "i am client";
    bool x = c.send_with_header(s);
    cout << x << endl;
    x = c.recv_with_header(s);
    cout << s << endl;
    if(c.recv_with_header(s))
    {
        cout << s << endl;
    }
    else
    {
        cout << "false" << endl;
    }
}