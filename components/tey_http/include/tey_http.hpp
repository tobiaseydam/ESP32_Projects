#ifndef TEY_HTTP_HPP
    #define TEY_HTTP_HPP

#include <string>
#include "esp_http_server.h"

#ifndef MIN
    #define MIN(x, y)  ((x) < (y) ? (x) : (y))
#endif

class http_config{
    private:
        static constexpr char *TAG = (char*)"http_config";
    protected:
        httpd_handle_t* server_handle = NULL;
    public:
        http_config();
        httpd_handle_t* get_server_handle() { return server_handle; };
};

typedef esp_err_t (*uri_handler_t)(httpd_req_t *r);

class http_uri_handler{
    private:
        static constexpr char *TAG = (char*)"http_uri_handler";
        std::string uri;
        httpd_method_t method;
        uri_handler_t handler;
        void *user_ctx;
    public:
        void set_uri(std::string value){ uri = value; };
        std::string get_uri(){ return uri; };

        void set_method(httpd_method_t value){ method = value; };
        httpd_method_t get_method(){ return method; };

        void set_handler(uri_handler_t value){ handler = value; };
        uri_handler_t get_handler(){ return handler; };

        void set_user_ctx(void* value){ user_ctx = value; };
        void* get_user_ctx(){ return user_ctx; };
};

class http_server{
    private:
        static constexpr char *TAG = (char*)"http_server";
    protected:
        http_config* cfg;
    public:
        http_server(http_config* config);
        void start();
        void register_uri_handler(http_uri_handler* handler);
};

#endif