# Json - C++ 11

### 一个性能不强，但是较为轻量好用的Json，没有内存泄漏问题。

> 经过google benchmark测试，借鉴腾讯大佬miloyip的json测试思路

> 使用twitter.json,canada.json,citm_catalog.json进行解析速度测试
 
- 大神的使用的是 clang 7.0 64位 macbook pro Core i7-4980HQ@ 2.80GHz

- 我使用的是 g++  11.3.0 ubuntu virtual-machine 4核心 Core i7-10875H 2.30GHz

> 解析三个文件的平均时间:          483ms (略高于基准(568ms) 1.18x)
> 
> 字符串化三个文件的平均时间为:    95ms (排名适中)

我认为还是比较满意的，因为我为了方便的使用，几乎将我的json做成弱类型，
与各位大佬追求的极致性能方向不同。

### 使用

- ``` git clone https://github.com/OoShawnoO/json.git ```
- copy json.h 到项目头文件文件夹
- ```#include "json.h"```

- ```using hzd; 或 hzd::json \ hzd::json_val 使用json与json_val```

### 详细使用

整体使用与python的 dic 差不多

- 数据类型 json_val { int32_t , double , array, json, char* } 几乎所有类型都重载过，可直接显式或隐式转换。
- 输出流重载过，可以直接使用 std::cout <<  json_val 或  std::cout << json 进行输出
- = 重载过，可以在确定json某个数据类型的情况下 int x / double x / char* x = json_val; 直接取值。
- json_val里的array可以装不同类型的数据，因为都会被构造成json_val类型。使用 json_val.push_back(int / double / char* / json / array)可以直接在array末尾添加数据。

#### 接口

> <h5>简单的构造与多种构造</h5>

```c++
hzd::json t = {
                {"key1","value1"},
                {"array",{5,6,7,"str",hzd::json() = {{"pair","value"}}}}
};
std::cout << t.dump() <<std::endl;
    terminal:
    {"array" : [5,6,7,"str",{"pair" : "value"}],"key1" : "value1"}
```

> <h5> bool load(std::string&) </h5>

> <h5> bool load(std::ifstream&)</h5>

> <h5>std::string dump()</h5>

```c++
hzd::json j;
std::string = "{\"msg\": null, \"code\": \"OK\", \"data\": {\"to_deliver_order\": {\"pubg\": 0, \"dota2\": 0}, \"unread_message\": {\"total\": 0}, \"unread_social_message\": {\"total\": 0}, \"to_pay_order\": {\"pubg\": 0, \"dota2\": 0}, \"updated_at\": {}, \"to_pay_buy_order\": {\"pubg\": 0, \"dota2\": 0}, \"to_pay_bargain\": {\"pubg\": 0, \"dota2\": 0}, \"to_send_offer_order\": {\"pubg\": 0, \"dota2\": 0}, \"unread_system_message\": {\"total\": 0}, \"to_receive_order\": {\"pubg\": 0, \"dota2\": 0}, \"to_accept_offer_order\": {\"pubg\": 0, \"dota2\": 0}, \"to_handle_bargain\": {\"pubg\": 0, \"dota2\": 0}, \"unread_feedback_replay\": {\"total\": 0}, \"new_roll_room\": {\"has_new\": false}}}";
if(j.load(test))
{
    cout << j.dump() << endl;
}

terminal:
{"msg" : null,"code" : "OK","data" : {"to_send_offer_order" : {"pubg" : 0,"dota2" : 0},"unread_feedback_replay" : {"total" : 0},"new_roll_room" : {"has_new" : false},"to_deliver_order" : {"pubg" : 0,"dota2" : 0},"to_pay_order" : {"pubg" : 0,"dota2" : 0},"to_accept_offer_order" : {"pubg" : 0,"dota2" : 0},"unread_message" : {"total" : 0},"to_receive_order" : {"pubg" : 0,"dota2" : 0},"updated_at" : {},"to_handle_bargain" : {"pubg" : 0,"dota2" : 0},"unread_social_message" : {"total" : 0},"unread_system_message" : {"total" : 0},"to_pay_buy_order" : {"pubg" : 0,"dota2" : 0},"to_pay_bargain" : {"pubg" : 0,"dota2" : 0}}}
```



> <h5>json_val& operator[](json_key& key)</h5>

```c++
/* 索引值 */
std::cout << j["msg"] << std::endl;
    terminal:
    null
/* 索引值 */        
std::cout << j["code"] << std::endl;
    terminal:
    "OK"
/* 索引值 */        
std::cout << j["data"] << std::endl;
    terminal:
    {"to_send_offer_order" : {"pubg" : 0,"dota2" : 0},"unread_feedback_replay" : {"total" : 0},"new_roll_room" : {"has_new" : false},"to_deliver_order" : {"pubg" : 0,"dota2" : 0},"to_pay_order" : {"pubg" : 0,"dota2" : 0},"to_accept_offer_order" : {"pubg" : 0,"dota2" : 0},"unread_message" : {"total" : 0},"to_receive_order" : {"pubg" : 0,"dota2" : 0},"updated_at" : {},"to_handle_bargain" : {"pubg" : 0,"dota2" : 0},"unread_social_message" : {"total" : 0},"unread_system_message" : {"total" : 0},"to_pay_buy_order" : {"pubg" : 0,"dota2" : 0},"to_pay_bargain" : {"pubg" : 0,"dota2" : 0}}
/* 修改值 */
j["msg"] = 123;
std::cout << j["msg"] << std::endl;
    terminal:
    123
/* 插入值 */
j["newkey"] = {2,3,4,"json_test"};
std::cout << j["newkey"] << std::endl;
	terminal:
	[2,3,4,"json_test"]
```

> <h5>void push_back(json_val&& / json_val& val)</h5>

```c++
/* 由于 json_val 有各种类型的构造函数会进行隐式转换 */
/* 所以可以如此使用 */
j["newkey"].push_back(1000);
	terminal:
	[2,3,4,"json_test",1000]
```

