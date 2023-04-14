# ErrorLog
一个自用的错误日志

```C++
/* 提供两个宏接口 */
/**
* LOG_MSG()
* @params:
* msg  - string
*/
LOG_MSG("信息");

/**
* LOG(error,msg)
* error - enum ERROR
* msg - string
*/
LOG(Out_of_Memory,"内存溢出");

/**
* LOG_FMT()
* @params:
* error - enum ERROR
* msg - string
* format - string format
* args ...
*/
LOG(Out_of_Memory,"内存溢出","xxx使用了地址为%x的内存",address);
