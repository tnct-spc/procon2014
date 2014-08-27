// MS WARNINGS MACRO
#define _SCL_SECURE_NO_WARNINGS
#include <network.hpp>
#include <ppm_reader.hpp>

#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>

#define NETWORK_TEST 1

BOOST_AUTO_TEST_CASE(ppm_read)
{
    ppm_reader ppm;
    auto const raw = ppm.from_file("./question/pp_prob01.ppm");

    BOOST_CHECK(raw.selectable_num == 3);
    BOOST_CHECK(raw.cost == std::make_pair(1,1));
    BOOST_CHECK(raw.split_num == std::make_pair(3,3));

    BOOST_CHECK(raw.size == std::make_pair(1023,897));
    BOOST_CHECK(raw.pixels.cols == raw.size.first);
    BOOST_CHECK(raw.pixels.rows == raw.size.second);
    BOOST_CHECK(raw.max_brightness == 255);
}

#if NETWORK_TEST

BOOST_AUTO_TEST_CASE(download)
{
    ppm_reader ppm;
    network::client client{};
    auto const data = client.get_problem(00).get();
    auto const raw = ppm.from_data(data);

    BOOST_CHECK(raw.selectable_num == 4);
    BOOST_CHECK(raw.cost == std::make_pair(30,10));
    BOOST_CHECK(raw.split_num == std::make_pair(4,4));

    BOOST_CHECK(raw.size == std::make_pair(256,256));
    BOOST_CHECK(raw.pixels.cols == raw.size.first);
    BOOST_CHECK(raw.pixels.rows == raw.size.second);
    BOOST_CHECK(raw.max_brightness == 255);
}

BOOST_AUTO_TEST_CASE(submit1)
{
    network::client client{};

    answer_list answer =
    {
    };
    auto const response = client.submit(0, "1", answer).get();
    BOOST_CHECK(response == "ACCEPTED 14");
}

BOOST_AUTO_TEST_CASE(submit2)
{
    answer_list answer;
    network::client client{};

    answer.push_back({ answer_type::action_type::select, point_type{ 0, 0}, '\0' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'D' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'D' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'R' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'U' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'L' });    
    auto const response1 = client.submit(0, "1", answer).get();
    BOOST_CHECK(response1 == "ACCEPTED 16");

    answer.push_back({ answer_type::action_type::select, point_type{ 0, 1}, '\0' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'R' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'D' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'R' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'D' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'R' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'U' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'U' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'U' });
    answer.push_back({ answer_type::action_type::change, point_type{-1,-1},  'L' });
    auto const response2 = client.submit(0, "2", answer).get();
    BOOST_CHECK(response2 == "ACCEPTED 11");
}

#endif // NETWORK_TEST
