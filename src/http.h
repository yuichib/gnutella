//////////  Response  /////////////////
// Method
#define GET  "GET"
#define HEAD "HEAD"
// HTTP=Version
#define HTTP_1_0 "HTTP/1.0"
#define HTTP_1_1 "HTTP/1.1"
#define SP " "
#define CRLF "\r\n"

// Default Status-Line  // sample
#define DEFAULT_STATUS_LINE "HTTP/1.0 200 OK"
#undef  DEFAULT_STATUS_LINE

// Respon-Phrase
#define OK                 "200 OK"
#define CRIATED            "201 Created"
#define ACCEPTED           "202 Accepted"
#define PROVISIONAL        "203 Provisional Information"
#define NO_RESPONSE        "204 No Response"
#define MOVED_PERMANENTLY  "301 Moved Permanently"
#define MOVED_TEMPORARILY  "302 Moved Temporarily"
#define METHOD             "303 Method"
#define NOT_MODIFIED       "304 Not Modified"

#define BAD_REQUEST        "400 Bad Request"
#define UNAUTHORIZED       "401 Unauthorized"
#define PAYMENT_REQUIRED   "402 Payment Required"
#define FORBIDDEN          "403 Forbidden"
#define NOT_FOUND          "404 Not Found"
#define METHOD_NOT_ALLOWED "405 Method Not Allowed"
#define NONE_ACCEPTABLE    "406 None Acceptable"
#define PROXY              "407 Proxy Authentication Required"
#define REQUEST_TIMEOUT    "408 Request Timeout"

#define INTERNAL_SERVER_ERROR "500 Internal Server Error"
#define NOT_IMPLEMENTED       "501 Not Implemented"
#define BAD_GATEWAY           "502 Bad Gateway"
#define SERVICE_UNAVAILABLE   "503 Service Unavailable"
#define GATEWAY_TIMEOUT       "504 Gateway Timeout"
///////////////////////////////////////////////////////////


/*//   Sample Response for Server Message
"HTTP/1.0 200 OK\n"                                     // Status-Line
"Connection: ...\n"                                     // General-Header
"Date:Tue,7 APR 1994 13:34:31 GMT\n"
"Forwarded:by http://info.cern.ch8000/for ptsun00.cern.ch\n"
"Mandatory: ...\n"
"Message-ID:<9411151630.4256@info.cern.ch>\n"
"MIME-Version:1.0\n"
"Server:CERN/3.0 libwww/2.17\n"                         // Response Header Fields
"WWW-Authenticate: ...\n"                               // 401
"Proxy-Authenticate: ...\n"                             // 407
"Retry-After:Wed,14 Dec 1994 18:22:54 GMT\n" //503
CRLF
"<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Frameset//EN">\n"    // Objects
"<HTML>\n"
"<HEAD>\n"
"<META HTTP-EQUIV="Content-Type" CONTENT="text/html;CHARSET=x-sjis">\n"
"<Link rel="SHORTCUT ICON" href="./mainpage/img/ico.ico" />\n"
"</HEAD>\n"
"<TITLE>Server Test Page</TITLE>\n"
"</HEAD>\n"
"<BODY>\n"
"<P>test page<br>\n"
"<a href="http://www.google.com/">google</a>\n"
"</P>\n"
"</BODY>\n"
"</HTML>\n"
//*/
