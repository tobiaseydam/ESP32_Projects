#ifndef TEY_HTML_TEMPLATE_PROCESSOR_HPP
    #define TEY_HTML_TEMPLATE_PROCESSOR_HPP

#include <string>

typedef std::string (*process_var_callback_t)(std::string var, void *ctx);

class template_processor{
    private:
        static constexpr char *TAG = (char*)"template_processor";
    protected:
        std::string input;
        std::string output;
        int pos = 0;
        void process_next_tag(std::string tag);

        void *process_ctx = NULL;
        process_var_callback_t var_cb = NULL;
    public:
        template_processor(std::string templ);
        void process();
        std::string get_output() { return output; };

        void set_process_var_callback(process_var_callback_t value)
            { var_cb = value; };
        
        void set_process_ctx(void *value){ process_ctx = value; };
};

#endif