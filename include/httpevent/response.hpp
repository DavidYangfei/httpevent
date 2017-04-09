#ifndef RESPONSE_HPP
#define RESPONSE_HPP



#include "libpoco.hpp"
#include "libevent2.hpp"
#include "cookie.hpp"

namespace httpevent {

    class response {
    public:
        response() = delete;

        response(struct evhttp_request *req)
        : req(req)
        , headers(evhttp_request_get_output_headers(req))
        , res(evhttp_request_get_output_buffer(req))
        , sent(false) {
        }

        response(const response& other) :
        req(other.req)
        , headers(other.headers)
        , res(other.res)
        , sent(other.sent) {
        }

        response& operator=(const response& right) {
            if (this == &right) {
                return *this;
            } else {
                this->req = right.req;
                this->headers = right.headers;
                this->res = right.res;
                this->sent = right.sent;
            }

            return *this;
        }


        virtual ~response() = default;

        response& send_head(const std::string& field, const std::string& value) {
            evhttp_add_header(this->headers, field.c_str(), value.c_str());
            return *this;
        }

        response& send_body(const std::string& str) {
            evbuffer_add(this->res, str.c_str(), str.size());
            return *this;
        }

        response& send_cookie(const cookie& cookie) {
            return this->send_head("Set-Cookie", cookie.toString());
        }

        void error(int code, const std::string& text) {
            if (!this->sent) {
                evhttp_send_error(req, code, text.c_str());
                this->sent = true;
            }
        }

        void submit(int code = 200, const std::string& reason = "OK") {
            if (!this->sent) {
                evhttp_send_reply(this->req, code, reason.c_str(), this->res);
                this->sent = true;
            }
        }

        void redirect(const std::string& uri, int code = 301) {
            if (!this->sent) {
                this->send_head("Location", uri);
                evhttp_send_reply(this->req, code, code == 301 ? "Moved Permanently" : "Found", this->res);
                this->sent = true;
            }
        }

        std::string get_response_data() {
            std::string result;
            if (!sent) {
                std::size_t buf_size = evbuffer_get_length(res);
                if (buf_size) {
                    char buf_data[buf_size + 1];
                    ev_ssize_t n = evbuffer_remove(res, buf_data, buf_size + 1);
                    if (n >= 0) {
                        result.assign(buf_data, n);
                    }
                }
            }
            return result;
        }

        bool is_sent()const {
            return this->sent;
        }

    private:
        struct evhttp_request * req;
        struct evkeyvalq *headers;
        struct evbuffer *res;
        bool sent;
    };
}


#endif /* RESPONSE_HPP */

