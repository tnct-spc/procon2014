#include <future>
#include <memory>
#include <unordered_map>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/network.hpp>
#include "network.hpp"

namespace network
{
    // RFC2396 の非予約文字のみエンコード．
    // .NET 4.0以前の EscapeDataString 相当のエンコードを行う．
    template<typename InputIterator, typename OutputIterator>
    OutputIterator client::urlencode(InputIterator first, InputIterator last, OutputIterator out) const
    {
        while(first != last)
        {
            if(
                (0x30<=*first && *first<=0x39) ||
                (0x41<=*first && *first<=0x5a) ||
                (0x61<=*first && *first<=0x7a) ||
                *first==0x21 ||
                *first==0x27 ||
                *first==0x28 ||
                *first==0x29 ||
                *first==0x2a ||
                *first==0x2d ||
                *first==0x2e ||
                *first==0x5f ||
                *first==0x7e
                )
            {
                *out++ = *first++;
            }
            else
            {
                std::string ss((boost::format("%%%02X") % (*first++ & 0xff)).str());
                out = std::copy(ss.begin(), ss.end(), out);
            }
        }
        return out;
    }

    std::string client::urlencode(std::string const& src) const
    {
        std::string ret;
        urlencode(src.cbegin(), src.cend(), std::back_inserter(ret));
        return ret;
    }

    std::string client::form_urlencode(std::unordered_map<std::string, std::string> const& header) const
    {
        std::string ret;

        for(auto const pair : header)
        {
            ret.append(urlencode(pair.first));
            ret.push_back('=');
            ret.append(urlencode(pair.second));
            ret.push_back('&');
        }
        ret.pop_back();

        return ret;
    }

    std::string client::http_request(std::string const url, std::string const request_body)
    {
        client_type::request request(url);
        request << boost::network::header("Connection", "close")
                << boost::network::header("Content-Type", "application/x-www-form-urlencoded")
                << boost::network::header("Content-Length", std::to_string(request_body.size()))
                << boost::network::header("User-Agent", "Is the order a clammbon?")
                << boost::network::body(request_body);

        client_type::response response = http_client_.post(request);
        std::string response_body = body(response);
        return response_body;
    }

    client::client(
        std::string const& server_host,
        std::string const& problem_path,
        std::string const& submit_path
        ) : server_host_(server_host),
            problem_path_(problem_path),
            submit_path_(submit_path)
    {
    }

    std::future<std::string> client::get_problem(int const problem_id)
    {
        auto const url = (boost::format("http://%s%sprob%02d.ppm") % server_host_ % problem_path_ % problem_id).str();

        return std::async(
            std::launch::async,
            boost::bind(&client::http_request, this, url, "")
            );
    }

    std::future<std::string> client::submit(int const problem_id, std::string const& player_id, std::string const& answer)
    {
        std::unordered_map<std::string, std::string> content_list;
        content_list["playerid"] = player_id;
        content_list["problemid"] = std::to_string(problem_id);
        content_list["answer"] = answer;

        auto const url  = (boost::format("http://%s%s") % server_host_ % submit_path_).str();
        auto const body = form_urlencode(content_list);

        return std::async(
            std::launch::async,
            boost::bind(&client::http_request, this, url, body)
            );
    }
} // namespace network

