#ifndef USE_CONV_HTTP_CONN_H
#define USE_CONV_HTTP_CONN_H

#include "server/include/conn.h"
#include "server/conv_single.h"
#include "server/conv_multi.h"
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include <exception>

namespace hzd {
    enum http_Methods {  GET, POST, PUT, PATCH, DELETE, TRACE, HEAD, OPTIONS, CONNECT    };
    /* region http_method_map */
    const std::unordered_map<http_Methods, std::string> http_method_map
            {
                    {GET,     "GET"},
                    {POST,    "POST"},
                    {PUT,     "PUT"},
                    {PATCH,   "PATCH"},
                    {DELETE,  "DELETE"},
                    {HEAD,    "HEAD"},
                    {TRACE,   "TRACE"},
                    {OPTIONS, "OPTIONS"},
                    {CONNECT, "CONNECT"}
            };
    /* endregion */
    /* region http_method_map( string to method )*/
    const std::unordered_map<std::string,http_Methods> http_method_map_string_to_method
            {
                    {"GET",GET},
                    {"POST",POST},
                    {"PUT",PUT},
                    {"PATCH",PATCH},
                    {"DELETE",DELETE},
                    {"TRACE",TRACE},
                    {"HEAD",HEAD},
                    {"OPTIONS",OPTIONS},
                    {"CONNECT",CONNECT},
            };
    /* endregion */
    enum class http_Status : int32_t {
        Continue = 100,
        Switching_Protocols = 101,
        Processing = 102,
        OK = 200,
        Created = 201,
        Accepted = 202,
        Non_Authoritative_information = 203,
        No_Content = 204,
        Reset_Content = 205,
        Partial_Content = 206,
        Multi_Status = 207,
        Multiple_Choice = 300,
        Moved_Permanently = 301,
        Move_Temporarily = 302,
        See_Other = 303,
        Not_Modified = 304,
        Use_Proxy = 305,
        Switch_Proxy = 306,
        Temporary_Redirect = 307,
        Bad_Request = 400,
        Unauthorized = 401,
        Payment_Required = 402,
        Forbidden = 403,
        Not_Found = 404,
        Method_Not_Allowed = 405,
        Not_Acceptable = 406,
        Proxy_Authentication_Required = 407,
        Request_Timeout = 408,
        Conflict = 409,
        Gone = 410,
        Length_Required = 411,
        Precondition_Failed = 412,
        Request_Entity_Too_Large = 413,
        Request_URI_Too_Long = 414,
        Unsupported_Media_Type = 415,
        Requested_Range_Not_Satisfiable = 416,
        Expectation_Failed = 417,
        I_Am_A_Teapot = 418,
        Misdirected_Request = 421,
        Unprocessable_Entity = 422,
        Locked = 423,
        Failed_Dependency = 422,
        Too_Early = 425,
        Upgrade_Required = 426,
        Retry_With = 449,
        Unavailable_For_Legal_Reasons = 451
    };
    /* region http_status_map */
    const std::unordered_map<http_Status, std::string> http_status_map
            {
                    {http_Status::Continue,                        "Continue"},
                    {http_Status::Switching_Protocols,             "Switching Protocols"},
                    {http_Status::Processing,                      "Processing"},
                    {http_Status::OK,                              "OK"},
                    {http_Status::Created,                         "Created"},
                    {http_Status::Accepted,                        "Accepted"},
                    {http_Status::Non_Authoritative_information,   "Non-Authoritative Information"},
                    {http_Status::No_Content,                      "No Content"},
                    {http_Status::Reset_Content,                   "Reset Content"},
                    {http_Status::Partial_Content,                 "Partial Content"},
                    {http_Status::Multi_Status,                    "Multi-Status"},
                    {http_Status::Multiple_Choice,                 "Multiple Choices"},
                    {http_Status::Moved_Permanently,               "Moved Permanently"},
                    {http_Status::Move_Temporarily,                "Moved Temporarily"},
                    {http_Status::See_Other,                       "See Other"},
                    {http_Status::Not_Modified,                    "Not Modified"},
                    {http_Status::Use_Proxy,                       "Use Proxy"},
                    {http_Status::Switch_Proxy,                    "Switch Proxy"},
                    {http_Status::Temporary_Redirect,              "Temporary Redirect"},
                    {http_Status::Bad_Request,                     "Bad Request"},
                    {http_Status::Unauthorized,                    "Unauthorized"},
                    {http_Status::Payment_Required,                "Payment Required"},
                    {http_Status::Forbidden,                       "Forbidden"},
                    {http_Status::Not_Found,                       "Not Found"},
                    {http_Status::Method_Not_Allowed,              "Method Not Allowed"},
                    {http_Status::Not_Acceptable,                  "Not Acceptable"},
                    {http_Status::Proxy_Authentication_Required,   "Proxy Authentication Required"},
                    {http_Status::Request_Timeout,                 "Request Timeout"},
                    {http_Status::Conflict,                        "Conflict"},
                    {http_Status::Gone,                            "Gone"},
                    {http_Status::Length_Required,                 "Length Required"},
                    {http_Status::Precondition_Failed,             "Precondition Failed"},
                    {http_Status::Request_Entity_Too_Large,        "Request Entity Too Large"},
                    {http_Status::Request_URI_Too_Long,            "Request-URI Too Long"},
                    {http_Status::Unsupported_Media_Type,          "Unsupported Media Type"},
                    {http_Status::Requested_Range_Not_Satisfiable, "Requested Range Not Satisfiable"},
                    {http_Status::Expectation_Failed,              "Expectation Failed"},
                    {http_Status::I_Am_A_Teapot,                   "I'm a teapot"},
                    {http_Status::Misdirected_Request,             "Misdirected Request"},
                    {http_Status::Unprocessable_Entity,            "Unprocessable Entity"},
                    {http_Status::Locked,                          "Locked"},
                    {http_Status::Failed_Dependency,               "Failed Dependency"},
                    {http_Status::Too_Early,                       "Too Early"},
                    {http_Status::Upgrade_Required,                "Upgrade Required"},
                    {http_Status::Retry_With,                      "Retry With"},
                    {http_Status::Unavailable_For_Legal_Reasons,   "Unavailable For Legal Reasons"}
            };
    /* endregion */
    enum class http_Header {
        Accept,
        AcceptCharset,
        AcceptEncoding,
        AcceptLanguage,
        AcceptDatetime,
        Authorization,
        CacheControl,
        Connection,
        ContentLength,
        ContentMD5,
        ContentType,
        Cookie,
        Date,
        Expect,
        Forwarded,
        From,
        Host,
        IfMatch,
        IfModifiedSince,
        IfNoneMatch,
        IfRange,
        IfUnmodifiedSince,
        MaxForwards,
        Origin,
        Pragma,
        ProxyAuthorization,
        Range,
        Referer,
        TE,
        Trailer,
        TransferEncoding,
        UserAgent,
        Upgrade,
        Via,
        Warning,
        XRequestedWith,
        DNT,
        XForwardedFor,
        XForwardedHost,
        XForwardedProto,
        XATTDeviceId,
        XWapProfile,
        ProxyConnection,
        XUIDH,
        XCsrfToken,
        XRequestID,
        XCorrelationID,
        SaveData,
        AcceptPatch,
        AcceptRanges,
        AccessControlAllowCredentials,
        AccessControlAllowHeaders,
        AccessControlAllowMethods,
        AccessControlAllowOrigin,
        AccessControlExposeHeaders,
        AccessControlMaxAge,
        Age,
        Allow,
        AltSvc,
        CacheControlHeader,
        ClearSiteData,
        ConnectionHeader,
        ContentDisposition,
        ContentEncoding,
        ContentLanguage,
        ContentLocation,
        ContentRange,
        CookieHeader,
        CrossOriginResourcePolicy,
        DNTHeader,
        DPR,
        EarlyData,
        ETag,
        ExpectCT,
        Expires,
        FeaturePolicy,
        ForwardedHeader,
        FromHeader,
        HostHeader,
        IfMatchHeader,
        IfModifiedSinceHeader,
        IfNoneMatchHeader,
        IfRangeHeader,
        IfUnmodifiedSinceHeader,
        IM,
        LastModified,
        Link,
        Location,
        OriginHeader,
        ProxyAuthenticate,
        PublicKeyPins,
        PublicKeyPinsReportOnly,
        RangeHeader,
        RefererHeader,
        ReferrerPolicy,
        RetryAfter,
        Server,
        ServerTiming,
        SetCookie,
        SourceMap,
        StrictTransportSecurity,
        TEHeader,
        TimingAllowOrigin,
        TK,
        TrailerHeader,
        TransferEncodingHeader,
        UpgradeHeader,
        Vary,
        ViaHeader,
        WWWAuthenticate,
        WarningHeader,
        XContentDuration,
        XContentSecurityPolicy,
        XContentSecurityPolicyReportOnly,
        XContentTypeOptions,
        XDNSPrefetchControl,
        XDownloadOptions,
        XFrameOptions,
        XForwardedProtoHeader,
        XFrameOptionsHeader,
        XPoweredBy,
        XRequestedWithHeader,
        XUACompatible,
        XUserIP,
        XXSSProtection,
    };
    /* region http_header_map */
    const std::unordered_map<http_Header,std::string> http_header_map = {
            {http_Header::Accept, "Accept"},
            {http_Header::AcceptCharset, "Accept-Charset"},
            {http_Header::AcceptEncoding, "Accept-Encoding"},
            {http_Header::AcceptLanguage, "Accept-Language"},
            {http_Header::AcceptDatetime, "Accept-Datetime"},
            {http_Header::Authorization, "Authorization"},
            {http_Header::CacheControl, "Cache-Control"},
            {http_Header::Connection, "Connection"},
            {http_Header::ContentLength, "Content-Length"},
            {http_Header::ContentMD5, "Content-MD5"},
            {http_Header::ContentType, "Content-Type"},
            {http_Header::Cookie, "Cookie"},
            {http_Header::Date, "Date"},
            {http_Header::Expect, "Expect"},
            {http_Header::Forwarded, "Forwarded"},
            {http_Header::From, "From"},
            {http_Header::Host, "Host"},
            {http_Header::IfMatch, "If-Match"},
            {http_Header::IfModifiedSince, "If-Modified-Since"},
            {http_Header::IfNoneMatch, "If-None-Match"},
            {http_Header::IfRange, "If-Range"},
            {http_Header::IfUnmodifiedSince, "If-Unmodified-Since"},
            {http_Header::MaxForwards, "Max-Forwards"},
            {http_Header::Origin, "Origin"},
            {http_Header::Pragma, "Pragma"},
            {http_Header::ProxyAuthorization, "Proxy-Authorization"},
            {http_Header::Range, "Range"},
            {http_Header::Referer, "Referer"},
            {http_Header::TE, "TE"},
            {http_Header::Trailer, "Trailer"},
            {http_Header::TransferEncoding, "Transfer-Encoding"},
            {http_Header::UserAgent, "User-Agent"},
            {http_Header::Upgrade, "Upgrade"},
            {http_Header::Via, "Via"},
            {http_Header::Warning, "Warning"},
            {http_Header::XRequestedWith, "X-Requested-With"},
            {http_Header::DNT, "DNT"},
            {http_Header::XForwardedFor, "X-Forwarded-For"},
            {http_Header::XForwardedHost, "X-Forwarded-Host"},
            {http_Header::XForwardedProto, "X-Forwarded-Proto"},
            {http_Header::XATTDeviceId, "X-ATT-DeviceId"},
            {http_Header::XWapProfile, "X-Wap-Profile"},
            {http_Header::ProxyConnection, "Proxy-Connection"},
            {http_Header::XUIDH, "X-UIDH"},
            {http_Header::XCsrfToken, "X-Csrf-Token"},
            {http_Header::XRequestID, "X-Request-ID"},
            {http_Header::XCorrelationID, "X-Correlation-ID"},
            {http_Header::SaveData, "Save-Data"},
            {http_Header::AcceptPatch, "Accept-Patch"},
            {http_Header::AcceptRanges, "Accept-Ranges"},
            {http_Header::AccessControlAllowCredentials, "Access-Control-Allow-Credentials"},
            {http_Header::AccessControlAllowHeaders, "Access-Control-Allow-Headers"},
            {http_Header::AccessControlAllowMethods, "Access-Control-Allow-Methods"},
            {http_Header::AccessControlAllowOrigin, "Access-Control-Allow-Origin"},
            {http_Header::AccessControlExposeHeaders, "Access-Control-Expose-Headers"},
            {http_Header::AccessControlMaxAge, "Access-Control-Max-Age"},
            {http_Header::Age, "Age"},
            {http_Header::Allow, "Allow"},
            {http_Header::ContentDisposition, "Content-Disposition"},
            {http_Header::ContentEncoding, "Content-Encoding"},
            {http_Header::ContentLanguage, "Content-Language"},
            {http_Header::ContentLocation, "Content-Location"},
            {http_Header::ContentRange, "Content-Range"},
            {http_Header::ETag, "ETag"},
            {http_Header::Expires, "Expires"},
            {http_Header::LastModified, "Last-Modified"},
            {http_Header::Link, "Link"},
            {http_Header::Location, "Location"},
            {http_Header::ProxyAuthenticate, "Proxy-Authenticate"},
            {http_Header::RetryAfter, "Retry-After"},
            {http_Header::Server, "Server"},
            {http_Header::SetCookie, "Set-Cookie"},
            {http_Header::StrictTransportSecurity, "Strict-Transport-Security"},
            {http_Header::Vary, "Vary"},
            {http_Header::WWWAuthenticate, "WWW-Authenticate"}
    };
    /* endregion */
    /* region http_header_map( string to http_header )*/
    const std::unordered_map<std::string, http_Header> http_header_map_string_to_header = {
            {"Accept", http_Header::Accept},
            {"Accept-Charset", http_Header::AcceptCharset},
            {"Accept-Encoding", http_Header::AcceptEncoding},
            {"Accept-Language", http_Header::AcceptLanguage},
            {"Accept-Datetime", http_Header::AcceptDatetime},
            {"Authorization", http_Header::Authorization},
            {"Cache-Control", http_Header::CacheControl},
            {"Connection", http_Header::Connection},
            {"Content-Length", http_Header::ContentLength},
            {"Content-Type", http_Header::ContentType},
            {"Cookie", http_Header::Cookie},
            {"Date", http_Header::Date},
            {"Expect", http_Header::Expect},
            {"Forwarded", http_Header::Forwarded},
            {"From", http_Header::From},
            {"Host", http_Header::Host},
            {"If-Match", http_Header::IfMatch},
            {"If-Modified-Since", http_Header::IfModifiedSince},
            {"If-None-Match", http_Header::IfNoneMatch},
            {"If-Range", http_Header::IfRange},
            {"If-Unmodified-Since", http_Header::IfUnmodifiedSince},
            {"Max-Forwards", http_Header::MaxForwards},
            {"Origin", http_Header::Origin},
            {"Pragma", http_Header::Pragma},
            {"Proxy-Authorization", http_Header::ProxyAuthorization},
            {"Range", http_Header::Range},
            {"Referer", http_Header::Referer},
            {"TE", http_Header::TE},
            {"User-Agent", http_Header::UserAgent},
            {"Upgrade", http_Header::Upgrade},
            {"Via", http_Header::Via},
            {"Warning", http_Header::Warning},
            {"Access-Control-Allow-Origin", http_Header::AccessControlAllowOrigin},
            {"Access-Control-Allow-Credentials", http_Header::AccessControlAllowCredentials},
            {"Access-Control-Allow-Methods", http_Header::AccessControlAllowMethods},
            {"Access-Control-Allow-Headers", http_Header::AccessControlAllowHeaders},
            {"Access-Control-Max-Age", http_Header::AccessControlMaxAge},
            {"Access-Control-Expose-Headers", http_Header::AccessControlExposeHeaders},
            {"Content-Disposition", http_Header::ContentDisposition},
            {"Content-Encoding", http_Header::ContentEncoding},
            {"Content-Language", http_Header::ContentLanguage},
            {"Content-Location", http_Header::ContentLocation},
            {"Content-Range", http_Header::ContentRange},
            {"ETag", http_Header::ETag},
            {"Expires", http_Header::Expires},
            {"Last-Modified", http_Header::LastModified},
            {"Link", http_Header::Link},
            {"Location", http_Header::Location},
            {"Proxy-Authenticate", http_Header::ProxyAuthenticate},
            {"Retry-After", http_Header::RetryAfter},
            {"Server", http_Header::Server},
            {"Set-Cookie", http_Header::SetCookie},
            {"Strict-Transport-Security", http_Header::StrictTransportSecurity},
            {"Transfer-Encoding", http_Header::TransferEncoding},
            {"Vary", http_Header::Vary},
            {"WWW-Authenticate", http_Header::WWWAuthenticate}
    };
    /* endregion */
    enum http_Version { HTTP_1_0,HTTP_1_1,HTTP_2_0};
    /* region http_version_map */
    const std::unordered_map<http_Version,std::string> http_version_map
            {
                    {HTTP_1_0,"HTTP/1.0"},
                    {HTTP_1_1,"HTTP/1.1"},
                    {HTTP_2_0,"HTTP/2.0"},
            };
    /* endregion */
    /* region http_content_type_map */
    std::unordered_map<std::string,std::string> http_content_type_map
            {
                    {"txt","text/plain"},
                    {"plain","text/plain"},
                    {"html","text/html"},
                    {"css","text/css"},
                    {"js","text/javascript"},
                    {"json","application/json"},
                    {"xml","application/xml"},
                    {"pdf","application/pdf"},
                    {"jpeg","image/jpeg"},
                    {"png","image/png"},
                    {"gif","image/gif"}
            };
    /* endregion */
    class http_conn : public conn {
        const static std::string base_path;
        struct request_header
        {
            http_Methods method;
            std::string url;
            http_Version version;
            std::unordered_map<std::string,std::string> request_headers;
        } req_header;

        struct response_header
        {
            http_Version version;
            http_Status status;
            std::unordered_map<std::string,std::string> response_headers;
            std::string header_text;
            std::string to_string()
            {
                std::ostringstream buffer;
                buffer << http_version_map.at(version) << " " << std::to_string((int32_t)status) << " " << http_status_map.at(status) << "\r\n";
                for(const auto & p : response_headers)
                {
                    buffer << p.first << ":" << p.second << "\r\n";
                }
                buffer << "\r\n";
                return buffer.str();
            }
        } res_header;

        struct response_body
        {
            std::string file_name;
            char* file_address{nullptr};
            struct stat file_stat{};
        } res_body;
        iovec iov[2]{{nullptr,0},{nullptr,0}};

        bool http_send()
        {
            size_t count = 0;
            while(write_cursor < write_total_bytes)
            {
                count = writev(socket_fd,iov,2);
                if(count <= 0)
                {
                    if(errno == EAGAIN)
                    {
                        next(EPOLLOUT);
                        return false;
                    }
                    munmap(res_body.file_address,res_body.file_stat.st_size);
                    iov[0].iov_base = nullptr;
                    iov[0].iov_len = 0;
                    iov[1].iov_base = nullptr;
                    iov[1].iov_len = 0;
                    notify_close();
                    return true;
                }
                write_cursor += count;
                if(write_cursor >= iov[0].iov_len)
                {
                    iov[0].iov_len = 0;
                    iov[1].iov_base = res_body.file_address + write_cursor - res_header.header_text.size();
                    iov[1].iov_len = write_total_bytes - write_cursor;
                }
                else
                {
                    iov[0].iov_base = (char*)res_header.header_text.c_str() + write_cursor;
                    iov[0].iov_len = iov[0].iov_len - write_cursor;
                }
            }
            next(EPOLLIN);
            munmap(res_body.file_address,res_body.file_stat.st_size);
            iov[0].iov_base = nullptr;
            iov[0].iov_len = 0;
            iov[1].iov_base = nullptr;
            iov[1].iov_len = 0;
            return true;
        }

        bool parse_header(std::string& data)
        {
            size_t request_line_pos = data.find("\r\n");
            size_t p1 = data.find(' ');
            if(p1 != std::string::npos)
            {
                std::string m = data.substr(0,p1);
                try
                {
                    req_header.method = http_method_map_string_to_method.at(m);
                    size_t p2 = data.find(' ',p1+1);
                    if(p2 != std::string::npos)
                    {
                        req_header.url = data.substr(p1 + 1,p2 - p1 -1);
                        std::string v = data.substr(p2+1,request_line_pos-p2-1);
                        if(v == "HTTP/1.0") req_header.version = HTTP_1_0;
                        else if(v == "HTTP/1.1") req_header.version = HTTP_1_1;
                        else if(v == "HTTP/2.0") req_header.version = HTTP_2_0;
                        else
                        {
                            throw std::exception();
                        }
                    }
                }
                catch(...)
                {
                    return false;
                }
            }

            size_t position = request_line_pos + 2;
            while(position < data.size())
            {
                size_t eol = data.find("\r\n",position);
                if(eol == std::string::npos) break;
                size_t colon = data.find(':',position);
                if(colon == std::string::npos || colon > eol) break;
                req_header.request_headers.emplace(std::pair<std::string,std::string>(data.substr(position,colon-position),
                                                                           data.substr(colon+1,eol-colon-1)));
                position = eol + 2;
            }
            return true;
        }
        bool load_file()
        {
            res_header.response_headers["Content-Length"] = std::to_string(0);
            if(stat(res_body.file_name.c_str(),&res_body.file_stat) < 0)
            {
                res_header.status = http_Status::Not_Found;
                return false;
            }
            if(!(res_body.file_stat.st_mode & S_IROTH))
            {
                res_header.status = http_Status::Forbidden;
                return false;
            }
            if(S_ISDIR(res_body.file_stat.st_mode))
            {
                res_header.status = http_Status::Bad_Request;
                return false;
            }
            int fd = open(res_body.file_name.c_str(),O_RDONLY);
            res_body.file_address = (char*)mmap(nullptr,res_body.file_stat.st_size,PROT_READ,MAP_PRIVATE,fd,0);
            res_header.response_headers["Content-Length"] = std::to_string(res_body.file_stat.st_size);
            size_t dot = res_body.file_name.find_last_of('.');
            if(dot == std::string::npos) res_header.response_headers["Content-Type"] = http_content_type_map["plain"];
            std::string t = res_body.file_name.substr(dot+1);
            res_header.response_headers["Content-Type"] = http_content_type_map.at(t);
            res_header.status = http_Status::OK;
            return true;
        }

        bool process_in() override
        {
            std::string data;
            if(!recv_all(data))
            {
                notify_close();
                return false;
            }
            size_t divide = data.find("\r\n\r\n");
            std::string header = data.substr(0,divide + 2);
            if(!parse_header(header))
            {
                notify_close();
                return false;
            }
            next(EPOLLOUT);
            return true;
        }
        bool process_out() override
        {
            signal(SIGPIPE,SIG_IGN);
            if(already)
            {
                res_header.version = req_header.version;
                res_body.file_name = base_path + req_header.url;
                load_file();
                res_header.header_text = res_header.to_string();
                write_total_bytes = res_header.header_text.size() + res_body.file_stat.st_size;
                write_cursor = 0;
                iov[0].iov_base = (char*)res_header.header_text.c_str();
                iov[0].iov_len = res_header.header_text.size();
                iov[1].iov_base = res_body.file_address;
                iov[1].iov_len = res_body.file_stat.st_size;
            }
            already = http_send();
            return true;
        }
    };
    const std::string http_conn::base_path = "resource";
}

#endif