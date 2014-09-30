#ifndef RESOLVER_ALGORITHM_2_HPP
#define RESOLVER_ALGORITHM_2_HPP

#include <boost/optional.hpp>
#include "data_type.hpp"

class algorithm_2
{
public:
	typedef answer_type return_type;

	algorithm_2();
	~algorithm_2();

	auto get()->boost::optional<return_type>;
	void reset(question_data const& data);

private:
	boost::optional<question_data> data_;
	std::vector<std::vector<point_type>> matrix;
	int width;
	int height;
	int cost_select;
	int cost_change;
	int *table;
};

#endif
