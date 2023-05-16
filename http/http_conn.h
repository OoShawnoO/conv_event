#ifndef CONV_EVENT_HTTP_CONN_H
#define CONV_EVENT_HTTP_CONN_H

#include "core/include/conn.h"  /* conn */
#include "core/conv_single.h"   /* conv_single*/
#include "core/conv_multi.h"    /* conv_multi */
#include <sys/stat.h>           /* fstat */
#include <exception>            /* exception */
#include <sys/sendfile.h>       /* sendfile */
#include <memory>               /* shared_ptr */
#include <utility>              /* types */

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
                    {"html", "text/html"},
                    {"htm", "text/html"},
                    {"shtml", "text/html"},
                    {"css", "text/css"},
                    {"xml", "text/xml"},
                    {"gif", "image/gif"},
                    {"jpeg", "image/jpeg"},
                    {"jpg", "image/jpeg"},
                    {"js", "application/javascript"},
                    {"atom", "application/atom+xml"},
                    {"rss", "application/rss+xml"},
                    {"mml", "text/mathml"},
                    {"txt", "text/plain"},
                    {"jad", "text/vnd.sun.j2me.app-descriptor"},
                    {"wml", "text/vnd.wap.wml"},
                    {"htc", "text/x-component"},
                    {"png", "image/png"},
                    {"tif", "image/tiff"},
                    {"tiff", "image/tiff"},
                    {"wbmp", "image/vnd.wap.wbmp"},
                    {"ico", "image/x-icon"},
                    {"jng", "image/x-jng"},
                    {"bmp", "image/x-ms-bmp"},
                    {"svg", "image/svg+xml"},
                    {"svgz", "image/svg+xml"},
                    {"webp", "image/webp"},
                    {"woff", "application/font-woff"},
                    {"woff2","application/font-woff"},
                    {"jar", "application/java-archive"},
                    {"war", "application/java-archive"},
                    {"ear", "application/java-archive"},
                    {"json", "application/json"},
                    {"hqx", "application/mac-binhex40"},
                    {"doc", "application/msword"},
                    {"pdf", "application/pdf"},
                    {"ps", "application/postscript"},
                    {"eps", "application/postscript"},
                    {"ai", "application/postscript"},
                    {"rtf", "application/rtf"},
                    {"m3u8", "application/vnd.apple.mpegurl"},
                    {"kml", "application/vnd.google-earth.kml+xml"},
                    {"kmz", "application/vnd.google-earth.kmz"},
                    {"xls", "application/vnd.ms-excel"},
                    {"eot", "application/vnd.ms-fontobject"},
                    {"ppt", "application/vnd.ms-powerpoint"},
                    {"odg", "application/vnd.oasis.opendocument.graphics"},
                    {"odp", "application/vnd.oasis.opendocument.presentation"},
                    {"ods", "application/vnd.oasis.opendocument.spreadsheet"},
                    {"odt", "application/vnd.oasis.opendocument.text"},
                    {"pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
                    {"xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
                    {"docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
                    {"wmlc", "application/vnd.wap.wmlc"},
                    {"7z", "application/x-7z-compressed"},
                    {"cco", "application/x-cocoa"},
                    {"jardiff", "application/x-java-archive-diff"},
                    {"jnlp", "application/x-java-jnlp-file"},
                    {"run", "application/x-makeself"},
                    {"pl", "application/x-perl"},
                    {"pm", "application/x-perl"},
                    {"prc", "application/x-pilot"},
                    {"pdb", "application/x-pilot"},
                    {"rar", "application/x-rar-compressed"},
                    {"rpm", "application/x-redhat-package-manager"},
                    {"sea", "application/x-sea"},
                    {"sit", "application/x-stuffit"},
                    {"tcl", "application/x-tcl"},
                    {"tk", "application/x-tcl"},
                    {"der", "application/x-x509-ca-cert"},
                    {"pem", "application/x-x509-ca-cert"},
                    {"crt", "application/x-x509-ca-cert"},
                    {"xpi", "application/x-xpinstall"},
                    {"xhtml", "application/xhtml+xml"},
                    {"xspf", "application/xspf+xml"},
                    {"zip", "application/zip"},
                    {"bin", "application/octet-stream"},
                    {"exe", "application/octet-stream"},
                    {"dll", "application/octet-stream"},
                    {"deb", "application/octet-stream"},
                    {"dmg", "application/octet-stream"},
                    {"iso", "application/octet-stream"},
                    {"img", "application/octet-stream"},
                    {"msi", "application/octet-stream"},
                    {"msp", "application/octet-stream"},
                    {"msm", "application/octet-stream"},
                    {"mid", "audio/midi"},
                    {"midi", "audio/midi"},
                    {"kar", "audio/midi"},
                    {"mp3", "audio/mpeg"},
                    {"ogg", "audio/ogg"},
                    {"m4a", "audio/x-m4a"},
                    {"ra", "audio/x-realaudio"},
                    {"3gpp", "video/3gpp"},
                    {"3gp", "video/3gpp"},
                    {"ts", "video/mp2t"},
                    {"mp4", "video/mp4"},
                    {"mpeg", "video/mpeg"},
                    {"mpg", "video/mpeg"},
                    {"mov", "video/quicktime"},
                    {"webm", "video/webm"},
                    {"flv", "video/x-flv"},
                    {"m4v", "video/x-m4v"},
                    {"mng", "video/x-mng"},
                    {"asx", "video/x-ms-asf"},
                    {"asf", "video/x-ms-asf"},
                    {"wmv", "video/x-ms-wmv"},
                    {"avi", "video/x-msvideo"},
            };
    /* endregion */
    class http_conn : public conn {
    public:
        struct request_header
        {
            http_Methods method;
            std::string url;
            http_Version version;
            std::unordered_map<std::string,std::string> parameters;
            std::unordered_map<std::string,std::vector<std::string>> request_headers;
            void clear()
            {
                url.clear();
                parameters.clear();
                request_headers.clear();
            }
        } req_header;

        struct request_body{
            std::string boundary;
            std::unordered_map<std::string,std::vector<std::string>> request_body_headers;
            std::unordered_map<std::string,std::string> form;
            std::unordered_map<std::string,std::pair<std::string,std::string>> files;
            void clear()
            {
                boundary.clear();
                request_body_headers.clear();
                form.clear();
                files.clear();
            }
        } req_body;

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
            void clear()
            {
                response_headers.clear();
                header_text.clear();
            }
        } res_header;

        struct response_body
        {
            std::string file_name;
            std::string body_text;
            int file_fd;
            struct stat file_stat{};
            void clear()
            {
                file_name.clear();
                body_text.clear();
                file_stat = {};
                file_fd = -1;
            }
        } res_body;

        #define ROUTER(r) r static_##r;
        class router
        {
            std::vector<http_Methods> _allow_;
        public:
            std::string url;

            explicit router(std::string&& _url,std::vector<http_Methods>&& _allow) {
                url = std::move(_url);
                _allow_ = std::move(_allow);
                register_router(this);
            }
            inline bool allow(http_Methods m) {
                return find(_allow_.begin(),_allow_.end(),m) != _allow_.end();
            }
            virtual bool method_get(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_post(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_put(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_patch(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_delete(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_trace(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_head(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_options(http_conn* c) { return c->method_not_allow(); }
            virtual bool method_connect(http_conn* c) { return c->method_not_allow(); }
        };
        bool redirect(std::string url)
        {
            res_header.status = http_Status::See_Other;
            res_header.response_headers["Location"] = std::move(url);
            if(!send_response_header()) {return false;}
            http_1_0_close();
            return true;
        }
        bool render(std::string file_path)
        {
            req_header.url = std::move(file_path);
            load_file();
            if(!send_response_header()) { return false; }
            write_total_bytes = res_body.file_stat.st_size;
            write_cursor = 0;
            if(!send_response_body()) { return false; }
            http_1_0_close();
            return true;
        }
        bool forward(std::string  url,http_Methods method = GET){
            switch(method)
            {
                case GET : {
                    return routers[url]->method_get(this);
                }
                case POST : {
                    return routers[url]->method_post(this);
                }
                case PUT : {
                    return routers[url]->method_put(this);
                }
                case PATCH : {
                    return routers[url]->method_patch(this);
                }
                case DELETE : {
                    return routers[url]->method_delete(this);
                }
                case TRACE : {
                    return routers[url]->method_trace(this);
                }
                case HEAD : {
                    return routers[url]->method_head(this);
                }
                case OPTIONS : {
                    return routers[url]->method_options(this);
                }
                case CONNECT : {
                    return routers[url]->method_connect(this);
                }
            }
        }
        bool send_str(const std::string& str,const std::string& type = "text/html"){
            res_header.response_headers["Content-Length"] = std::to_string(str.size());
            res_header.response_headers["Content-Type"] = type;
            res_header.status = http_Status::OK;

            if(!send_response_header()) return false;
            while(!send((std::string&)str,str.size()))
            {
                if(errno == EAGAIN) continue;
                ::close(res_body.file_fd);
                res_body.file_fd = -1;
                notify_close();
                return false;
            }
            http_1_0_close();
            return true;
        }

        #define FILTER(f) f static_##f;
        class filter{
            std::vector<http_Methods> _not_allow_;
        public:
            struct node
            {
                std::unordered_map<std::string,std::shared_ptr<node>> children;
                filter* _filter;

                node()
                {
                    _filter = nullptr;
                }
            };

            std::string url;
            explicit filter(std::string&& _url,std::vector<http_Methods>&& not_allow = {})
            {
                url = std::move(_url);
                _not_allow_ = std::move(not_allow);
                register_filter(this);
            }
            virtual bool allow(http_conn* con) {
                return find(_not_allow_.begin(),_not_allow_.end(),con->req_header.method) == _not_allow_.end();
            }
        };


        static void register_router(router* r) {
            if(!r) return;
            routers[r->url] = r;
        }
        static void register_filter(filter* f)
        {
            if(!f) return;
            _register_filter(std::forward<filter*>(f));
        }
    protected:

        const static std::string base_path;

        static std::unordered_map<std::string,router*> routers;
        static std::unordered_map<std::string,std::shared_ptr<filter::node>> filters;

        static void _register_filter(filter* f)
        {
            if(f->url.size() < 2 || f->url[0] != '/' || f->url[f->url.size()-1] == '/') {
                LOG_FMT(General_Error,"","拦截器注册错误 url = %s",f->url.c_str());
                return;
            }
            auto cur = filters["/"];
            if(f->url[1] == '*')
            {
                cur->_filter = f;
                return;
            }
            size_t pre = 1;
            size_t pos = f->url.find('/',pre);
            std::string sub_str;
            while(pos != std::string::npos)
            {
                sub_str = f->url.substr(pre,pos-pre);
                cur->children[sub_str] = std::make_shared<filter::node>();
                cur = cur->children[sub_str];
                pre = pos+1;
                pos = f->url.find('/',pre);
            }
            sub_str = f->url.substr(pre);
            if(sub_str == "*")
            {
                cur->_filter = f;
                return;
            }
            cur->children[sub_str] = std::make_shared<filter::node>();
            cur = cur->children[sub_str];
            cur->_filter = f;
        }

        static filter* match(const std::string& url)
        {
            if(url.empty()) return nullptr;
            if(url[0] != '/') return nullptr;
            if(url[1] == '*') return filters["/"]->_filter;
            auto cur = filters["/"];
            filter* tmp = cur->_filter;
            size_t pre = 1;
            size_t pos = url.find('/',pre);
            std::string sub_str;
            while(pos != std::string::npos && pos < url.size()-1)
            {
                sub_str = url.substr(pre,pos-pre);
                if(cur->children.find(sub_str) == cur->children.end())
                {
                    return tmp;
                }
                cur = cur->children[sub_str];
                tmp = cur->_filter;
                pre = pos+1;
                pos = url.find('/',pre);
            }
            if(pos == url.size()-1)
            {
                sub_str = url.substr(pre,pos-pre-1);
            }
            else sub_str = url.substr(pre);
            if(cur->children.find(sub_str) == cur->children.end())
            {
                return tmp;
            }
            cur = cur->children[sub_str];
            tmp = cur->_filter;
            return tmp;
        }


    private:
        bool send_response_body_base()
        {
            signal(SIGPIPE,SIG_IGN);
            if(res_body.file_fd != -1)
            {
                while(write_cursor < write_total_bytes)
                {
                    auto offset = (off_t)write_cursor;
                    size_t send_count = sendfile(socket_fd,res_body.file_fd,&offset,write_total_bytes-write_cursor);
                    if(send_count == -1)
                    {
                        if(errno == EAGAIN)
                        {
                            return false;
                        }
                        ::close(res_body.file_fd);
                        res_body.file_fd = -1;
                        return false;
                    }
                    write_cursor += send_count;
                }
                ::close(res_body.file_fd);
                res_body.file_fd = -1;

            }
            else
            {
                send(res_body.body_text,res_body.body_text.size());
            }
            next(EPOLLIN);
            return true;
        }
        bool parse_header(const std::string& data)
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
                        std::string url = data.substr(p1 + 1,p2 - p1 -1);
                        size_t start = url.find('?');
                        if(start == std::string::npos)
                        {
                            req_header.url = url;
                        }
                        else
                        {
                            req_header.url = url.substr(0,start);
                            start += 1;
                            size_t end = start;
                            while(end != std::string::npos)
                            {
                                end = url.find_first_of("&=",start);
                                std::string key = url.substr(start,end-start);
                                start = end + 1;
                                end = url.find_first_of('&',start);
                                std::string value = url.substr(start,end-start);
                                req_header.parameters.emplace(std::move(key),std::move(value));
                                start = end + 1;
                            }
                        }
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
            std::stringstream ss(data.substr(request_line_pos + 2));
            std::string line;
            std::string last_header;
            while(getline(ss,line))
            {
                if(line.empty()) break;
                line.erase(line.end()-1);
                if(line[0] == ' ' || line[0] == '\t')
                {
                    if(!last_header.empty())
                    {
                        req_header.request_headers[last_header].back() += line;
                    }
                }
                else
                {
                    auto pos = line.find(':');
                    if(pos != std::string::npos)
                    {
                        std::string name = line.substr(0,pos);
                        std::string value = line.substr(pos + 1);
                        value.erase(0,value.find_first_not_of(' '));
                        value.erase(value.find_last_not_of(' ') + 1);
                        std::vector<std::string> values;
                        if(name == "User-Agent")
                        {
                            values.emplace_back(value);
                            req_header.request_headers[name] = values;
                            continue;
                        }
                        std::stringstream value_ss(value);
                        std::string value_line;
                        while(getline(value_ss,value_line,','))
                        {
                                values.emplace_back(value_line);
                        }
                        req_header.request_headers[name] = values;
                        last_header = name;
                    }
                }
            }
            return true;
        }
        bool parse_body(const std::string& body)
        {
            req_body.clear();

            if(req_header.request_headers["Content-Type"][0].find("multipart/form-data") != std::string::npos)
            {
                size_t boundary_end = body.find("\r\n");
                req_body.boundary = body.substr(0,boundary_end);
                std::stringstream ss(body.substr(boundary_end + 2));
                std::string line;
                std::string last_header;
                while(getline(ss,line))
                {
                    if(line.empty()) break;
                    line.erase(line.end()-1);
                    if(line[0] == ' ' || line[0] == '\t')
                    {
                        if(!last_header.empty())
                        {
                            req_body.request_body_headers[last_header].back() += line;
                        }
                    }
                    else
                    {
                        auto pos = line.find(':');
                        if(pos != std::string::npos)
                        {
                            std::string name = line.substr(0,pos);
                            std::string value = line.substr(pos + 1);
                            value.erase(0,value.find_first_not_of(' '));
                            value.erase(value.find_last_not_of(' ') + 1);

                            size_t form_type_line = value.find_first_of(';');
                            std::vector<std::string> values{value.substr(0,form_type_line)};
                            size_t name_line = value.find("name=");
                            size_t filename_line = value.find("filename=");
                            if(name_line != std::string::npos)
                            {
                                size_t post_data_start = body.find("\r\n\r\n");
                                std::string post_data = body.substr(post_data_start+4,body.size()-req_body.boundary.size()-6-post_data_start-4);
                                req_body.files.emplace(std::pair<std::string,std::pair<std::string,std::string>>
                                                               {
                                                                       value.substr(name_line+6,value.find('\"',name_line+6)-name_line-6),
                                                                       std::pair<std::string,std::string>{
                                                                               value.substr(filename_line+10,value.find('\"',filename_line+10)-filename_line-10),
                                                                               post_data
                                                                       }
                                                               });
                            }
                            req_body.request_body_headers[name] = values;
                            last_header = name;
                        }
                    }
                }
                return true;
            }
            else if(req_header.request_headers["Content-Type"][0].find("application/x-www-form-urlencoded") != std::string::npos)
            {
                std::stringstream ss(body);
                std::string line;
                while(getline(ss,line,'&'))
                {
                    if(line.empty()) break;
                    size_t equal_pos = line.find('=');
                    std::string key = line.substr(0,equal_pos);
                    std::string value = line.substr(equal_pos+1);
                    req_body.form[key] = value;
                }
                return true;
            }
            return false;
        }
        inline void build_body_text()
        {
            std::ostringstream buffer;
            buffer << "<h1> " << std::to_string((int32_t)res_header.status) << " " << http_status_map.at(res_header.status) << "</h1>";
            res_body.body_text = buffer.str();
        }
        bool send_response_header()
        {
            res_header.header_text = res_header.to_string();
            while(!send(res_header.header_text,res_header.header_text.size()))
            {
                if(errno == EAGAIN) continue;
                ::close(res_body.file_fd);
                res_body.file_fd = -1;
                notify_close();
                return false;
            }
            return true;
        }
        bool send_response_body()
        {
            while(!send_response_body_base())
            {
                if(errno == EAGAIN) continue;
                notify_close();
                return false;
            }
            return true;
        }
        inline bool method_not_allow()
        {
            res_header.status = http_Status::Method_Not_Allowed;
            build_body_text();
            res_header.response_headers["Content-Length"] = std::to_string(res_body.body_text.size());
            if(!send_response_header()) { return false; }
            if(!send_response_body()) { return false; }
            http_1_0_close();
            return true;
        }
        bool load_file()
        {
            res_body.file_name = base_path + req_header.url;
            res_body.file_fd = open(res_body.file_name.c_str(),O_RDONLY);
            if(fstat(res_body.file_fd,&res_body.file_stat) < 0)
            {
                res_header.status = http_Status::Not_Found;
                build_body_text();
                res_header.response_headers["Content-Length"] = std::to_string(res_body.body_text.size());
                return false;
            }
            if(!(res_body.file_stat.st_mode & S_IROTH))
            {
                res_header.status = http_Status::Forbidden;
                build_body_text();
                res_header.response_headers["Content-Length"] = std::to_string(res_body.body_text.size());
                return false;
            }
            if(S_ISDIR(res_body.file_stat.st_mode))
            {
                res_header.status = http_Status::Bad_Request;
                build_body_text();
                res_header.response_headers["Content-Length"] = std::to_string(res_body.body_text.size());
                return false;
            }

            res_header.response_headers["Content-Length"] = std::to_string(res_body.file_stat.st_size);
            size_t dot = res_body.file_name.find_last_of('.');
            if(dot == std::string::npos) res_header.response_headers["Content-Type"] = http_content_type_map["plain"];
            std::string t = res_body.file_name.substr(dot+1);
            try{
                res_header.response_headers["Content-Type"] = http_content_type_map.at(t);
            }
            catch (...)
            {
                res_header.response_headers["Content-Type"] = "application/octet-stream";
            }
            res_header.status = http_Status::OK;
            return true;
        }
        inline void clear_in()
        {
            req_header.clear();
        }
        inline void clear_out()
        {
            res_header.clear();
            res_body.clear();
        }
        inline void http_1_0_close()
        {
            if(res_header.version == http_Version::HTTP_1_0)
            {
                notify_close();
            }
            else
            {
                next(EPOLLIN);
            }
        }

        bool send_status(http_Status status)
        {
            res_header.status = status;
            build_body_text();
            res_header.response_headers["Content-Length"] = std::to_string(res_body.body_text.size());
            if(!send_response_header()) return false;
            if(!send_response_body()) return false;
            http_1_0_close();
            return true;
        }

        virtual bool process_get()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_get(this);
            }
            catch(...)
            {
                load_file();
                if(!send_response_header()) { return false; }
                write_total_bytes = res_body.file_stat.st_size;
                write_cursor = 0;
                if(!send_response_body()) { return false; }
                http_1_0_close();
                return true;
            }
        }
        virtual bool process_post()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_post(this);
            }
            catch(...) 
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_put()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_put(this);
            }
            catch(...) 
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_patch()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_patch(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_delete()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_delete(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_trace()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_trace(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_head()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_head(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_options()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_options(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }
        virtual bool process_connect()
        {
            try
            {
                auto r = routers.at(req_header.url);
                if(!r->allow(req_header.method))
                {
                    throw std::exception();
                }
                return r->method_connect(this);
            }
            catch(...)
            {
                method_not_allow();
                return true;
            }
        }

        bool process_in() override
        {
            clear_in();

            std::string data;
            if(!recv_all(data))
            {
                notify_close();
                return false;
            }
            if(data.empty()) { next(EPOLLIN); return true; }
            size_t divide = data.find("\r\n\r\n");
            std::string header = data.substr(0,divide + 2);
            if(!parse_header(header))
            {
                notify_close();
                return false;
            }
            if(req_header.method == POST)
            {
                std::string body = data.substr(divide+4);
                size_t cur = 0;
                while((cur = strtoll(req_header.request_headers["Content-Length"][0].c_str(),nullptr,10)) > body.size())
                {
                    recv(body,cur - body.size());
                }
                parse_body(body);
            }
            next(EPOLLOUT);
            return true;
        }
        bool process_out() override
        {
            clear_out();
            res_header.version = req_header.version;
            filter* f = match(req_header.url);
            if(f)
            {
                if(!f->allow(this))
                {
                    return send_status(http_Status::Forbidden);
                }
            }
            switch(req_header.method)
            {
                case GET : {
                    return process_get();
                }
                case POST : {
                    return process_post();
                }
                case PUT : {
                    return process_put();
                }
                case PATCH : {
                    return process_patch();
                }
                case DELETE : {
                    return process_delete();
                }
                case TRACE : {
                    return process_trace();
                }
                case HEAD : {
                    return process_head();
                }
                case OPTIONS : {
                    return process_options();
                }
                case CONNECT : {
                    return process_connect();
                }
            }
            return false;
        }
    };
    using router = http_conn::router;
    using filter = http_conn::filter;
    using hzd::http_Methods;
    const std::string http_conn::base_path = configure::get_config().configs["resource_path"];
    std::unordered_map<std::string,router*> http_conn::routers;
    std::unordered_map<std::string,std::shared_ptr<filter::node>> http_conn::filters { {"/",std::make_shared<filter::node>()} };

    using http_multi = conv_multi<http_conn>;
    using http_single = conv_single<http_conn>;
}
using namespace hzd;

#endif