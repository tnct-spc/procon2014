#ifndef RESOLVER_NETWORK_HPP
#define RESOLVER_NETWORK_HPP

#include <future>
#include <unordered_map>
#include <boost/network/protocol/http/client.hpp>
#include "data_type.hpp"

namespace network
{
    class client
    {
    private:
        typedef boost::network::http::basic_client<boost::network::http::tags::http_async_8bit_udp_resolve, 1, 0> client_type;

        std::string const server_host_;
        std::string const problem_path_;
        std::string const submit_path_;
        client_type http_client_;

        //
        // x-www-form-urlencoded用
        // 主な違いは" "を"+"にエンコードすること
        //
        template<typename InputIterator, typename OutputIterator>
        OutputIterator urlencode(InputIterator first, InputIterator last, OutputIterator out) const;
        std::string urlencode(std::string const& src) const;

        // answer_listをシリアライズ(直列化)して，送信用データにする
        std::string serialize_answer(answer_list const& answer);

        // AAA=BBB&CCC=DDD
        std::string form_urlencode(std::unordered_map<std::string, std::string> const& header) const;

        // 汎用的にリクエスト作って，bodyが返されるの待つ
        std::string http_request(std::string const url, std::string const request_body = "");

    public:
        client(
            std::string const& server_host = "127.0.0.1",
            std::string const& problem_path = "/problem/",
            std::string const& submit_path = "/SubmitAnswer"
            );
        virtual ~client() = default;

        std::future<std::string> get_problem(int const problem_id);
        std::future<std::string> submit(int const problem_id, std::string const& player_id, std::string const& answer);
        std::future<std::string> submit(int const problem_id, std::string const& player_id, answer_list const& answer);
    };
} // namespace network

#endif

