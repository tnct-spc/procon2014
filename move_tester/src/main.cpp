#include <iostream>
#include <string>
#include <boost/format.hpp>
#include "slide_algorithm/algorithm.hpp"
#include "data_type.hpp"

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "使用方法: " << argv[0] << " <ヨコ> <タテ> <選択可能回数> <選択コスト> <交換コスト>" << std::endl;
		std::cout << "標準入力から行列を与えてください" << std::endl;
		return -1;
	}
	int const width = std::stoi(argv[1]);
	int const height = std::stoi(argv[2]);
	int const selectable = std::stoi(argv[3]);
	int const cost_select = std::stoi(argv[4]);
	int const cost_change = std::stoi(argv[5]);

	std::vector<std::vector<point_type>> matrix;
	int point_int;
	std::string point_str;
	for (int y = 0; y < height; ++y) {
		std::vector<point_type> row;
		for (int x = 0; x < width; ++x) {
			std::cin >> point_str;
			point_int = std::stoi(point_str, nullptr, 16);
			row.push_back({point_int / 16, point_int % 16});
		}
		matrix.push_back(std::move(row));
	}

	auto question = question_data(0, "", {width, height}, selectable, cost_select, cost_change, matrix);
	algorithm algo;
	algo.reset(question);
	algo.get();

	return 0;
}

// vim: set ts=4 sw=4:
