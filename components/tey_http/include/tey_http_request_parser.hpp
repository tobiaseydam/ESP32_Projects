#ifndef TEY_HTTP_REQUEST_PARSER_HPP
    #define TEY_HTTP_REQUEST_PARSER_HPP

#include "tey_http.hpp"
#include <map>

typedef void (*field_callback_t)(std::string field, std::string value);
typedef void (*upload_callback_t)(std::string field, std::string filename, std::string value);

class http_request_parser{
    private:
        static constexpr char *TAG = (char*)"http_request_parser";
    protected:
        httpd_req_t *req;
        field_callback_t field_found = NULL;
        upload_callback_t upload_field_found = NULL;
        std::string get_header(std::string block, std::string header);
        std::string get_block_value(std::string block);
        std::string get_header_field(std::string header, std::string field);
        esp_err_t parse_block(std::string block);
    public:
        http_request_parser(httpd_req_t *request);
        void set_field_found_callback(field_callback_t value) {field_found = value; };
        void set_upload_found_callback(upload_callback_t value) {upload_field_found = value; };
        esp_err_t parse();
};


#endif