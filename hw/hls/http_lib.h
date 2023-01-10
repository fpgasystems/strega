/// @author Fabio Maschi (Department of Computer Science, ETH Zurich)
/// @copyright This software is copyrighted under the BSD 3-Clause License.

#pragma once
#include <ap_int.h>

namespace http {

//
// Parameters
//

#define HTTP_LISTEN_PORT 80

#define HTTP_DATA_WIDTH 512
#define HTTP_SESSION_WIDTH 16

//
// Enumeration
//

enum class HttpMethod {
  err,
  CONNECT,
  DELETE,
  GET,
  HEAD,
  OPTIONS,
  POST,
  PUT
};

enum class HttpStatus {
  CONTINUE_100,
  SWITCHING_PROTOCOLS_101,
  OK_200,
  CREATED_201,
  ACCEPTED_202,
  NONAUTHORITATIVE_INFORMATION_203,
  NO_CONTENT_204,
  RESET_CONTENT_205,
  PARTIAL_CONTENT_206,
  MULTIPLE_CHOICES_300,
  MOVED_PERMANENTLY_301,
  FOUND_302,
  SEE_OTHER_303,
  NOT_MODIFIED_304,
  USE_PROXY_305,
  TEMPORARY_REDIRECT_306,
  BAD_REQUEST_400,
  UNAUTHORISED_401,
  FORBIDDEN_403,
  NOT_FOUND_404,
  METHOD_NOT_ALLOWED_405,
  NOT_ACCEPTABLE_406,
  PROXY_AUTHENTICATION_REQUIRED_407,
  REQUEST_TIMEOUT_408,
  CONFLICT_409,
  GONE_410,
  LENGTH_REQUIRED_411,
  PRECONDITION_FAILED_412,
  REQUEST_ENTITY_TOO_LARGE_413,
  REQUEST_URI_TOO_LONG_414,
  UNSUPPORTED_MEDIA_TYPE_415,
  REQUEST_RANGE_NOT_SATISFIABLE_416,
  EXPECATION_FAILED_417,
  INTERNAL_SERVER_ERROR_500,
  NOT_IMPLEMENTED_501,
  BAD_GATEWAY_502,
  SERVICE_UNAVAILABLE_503,
  GATEWAY_TIMEOUT_504,
  HTTP_VERSION_NOT_SUPPORTED_505
};

//
// internal
//

struct http_meta
{
  HttpMethod method;
  ap_uint<HTTP_SESSION_WIDTH> sessionID;
  http_meta() {}
};

// Stream Payload Types
struct http_headline_ispt {
  ap_uint<HTTP_DATA_WIDTH> line;
  ap_uint<HTTP_SESSION_WIDTH> sessionID;
};

struct http_headline_ospt {
  HttpMethod method;
  ap_uint<HTTP_SESSION_WIDTH> sessionID;
  // ap_uint<HTTP_DATA_WIDTH> endpoint;
};

struct http_status_code_ospt {
  ap_uint<HTTP_DATA_WIDTH> data;
  ap_uint<HTTP_DATA_WIDTH/8> keep;
};

//
// request
//

struct http_request_spt
{
  http_meta meta;
  HttpMethod method;
  // ap_uint<HTTP_DATA_WIDTH> endpoint;
  http_request_spt() {}
};

//
// response
//

struct http_response_spt
{
  http_meta meta;
  HttpStatus status_code;
  ap_uint<16> body_size;
  ap_uint<16> headers_size;
  http_response_spt() {}
};

} // namespace http