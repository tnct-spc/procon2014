#ifdef DEBUG
#include <iostream>
#endif

#include <algorithm>
#include <limits>
#include "pixel_sorter.hpp"

question_data pixel_sorter::operator() (question_raw_data const& raw, split_image_type const& splited_image) const
{
    question_data formed = {
        raw.split_num,
        raw.selectable_num,
        raw.cost.first,
        raw.cost.second,
        std::vector<std::vector<int>>(raw.split_num.second, std::vector<int>(raw.split_num.first, std::numeric_limits<int>::max()) )
    };

    auto& block = formed.block;

    //
    // Sub Algorithm
    // 正しい位置に並べた時に左上から，1~nまでの番号をふり，それが今どこにあるのかという情報をblockに格納
    //

    auto const& comp = this->image_comp(splited_image);


    // Sub Algorithm End

    return formed;
}

//2つのピクセル間の距離を2乗した値を返却．result = r^2 + g^2 + b^2
int pixel_sorter::pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
{
    int s = 0;
    s += static_cast<int>(std::pow(std::abs(std::get<0>(lhs) - std::get<0>(rhs)), 2)); //r
    s += static_cast<int>(std::pow(std::abs(std::get<1>(lhs) - std::get<1>(rhs)), 2)); //g
    s += static_cast<int>(std::pow(std::abs(std::get<2>(lhs) - std::get<2>(rhs)), 2)); //b
    return s;
}

//一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::rl_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].back(), rhs[i].front());
    return s;
}

//一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::lr_comparison(image_type const& lhs, image_type const& rhs) const
{
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs[i].front(), rhs[i].back());
    return s;
}

//一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::ud_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_top    = lhs.front();
    auto const& rhs_bottom = rhs.back();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_top[i], rhs_bottom[i]);
    return s;
}

//一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
uint64_t pixel_sorter::du_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto const& lhs_bottom = lhs.back();
    auto const& rhs_top    = rhs.front();
        
    uint64_t s = 0;
    for(int i=0; i<lhs.size(); ++i)
        s += pixel_comparison(lhs_bottom[i], rhs_top[i]);
    return s;
}

point_type pixel_sorter::ur_choose(compared_type const& comp, point_type const ul, point_type const dl, point_type const dr) const
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dr.y][dr.x][i][j].up + comp[ul.y][ul.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type pixel_sorter::ul_choose(compared_type const& comp, point_type const ur, point_type const dl, point_type const dr) const
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[dl.y][dl.x][i][j].up + comp[ur.y][ur.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type pixel_sorter::dr_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dl) const
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ur.y][ur.x][i][j].down + comp[dl.y][dl.x][i][j].right;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

point_type pixel_sorter::dl_choose(compared_type const& comp, point_type const ul, point_type const ur, point_type const dr) const
{
    point_type possition;
    uint64_t sum = std::numeric_limits<uint64_t>::max();

    for(int i=0; i<comp.size(); ++i) for(int j=0; j<comp[0].size(); ++j)
    {
        uint64_t const tmp = comp[ul.y][ul.x][i][j].down + comp[dr.y][dr.x][i][j].left;
        if(0 < tmp && tmp < sum)
        {
            sum = tmp;
            possition = {j, i};
        }
    }

    return possition;
}

compared_type pixel_sorter::image_comp(split_image_type const& image) const
{
    //返却用変数
    //"最大距離が収納されたtuple"の4次元配列になる
    compared_type comp(
        image.size(),
        std::vector<std::vector<std::vector<direction_type<uint64_t>>>>(
            image[0].size(),
            std::vector<std::vector<direction_type<uint64_t>>>(
                image.size(),
                std::vector<direction_type<uint64_t>>(
                    image[0].size(),
                    {
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max(),
                        std::numeric_limits<uint64_t>::max()
                    })
                )
            )
        );

    for(int i=0; i<image.size(); ++i) for(int j=0; j<image[0].size(); ++j)
    {
        for(int k=0; k<image.size(); ++k) for(int l=0; l<image[0].size(); ++l)
        {
            if(k > i || (k == i && l > j)) // (j,i)より(l,k)の方が，探索順として後半に或るための条件
            {
                //順序を変えて逆の逆の組み合わせの相対評価は同じであることを使用して探索量を半分にする
                //例えば，Aから見たBは上なら，Bから見たAは下．
                comp[i][j][k][l].up    = comp[k][l][i][j].down  = ud_comparison(image[i][j], image[k][l]);
                comp[i][j][k][l].right = comp[k][l][i][j].left  = rl_comparison(image[i][j], image[k][l]);
                comp[i][j][k][l].down  = comp[k][l][i][j].up    = du_comparison(image[i][j], image[k][l]);
                comp[i][j][k][l].left  = comp[k][l][i][j].right = lr_comparison(image[i][j], image[k][l]);
            }
        }
    }
    return comp;
}

adjacent_type select_minimum(compared_type const& compared_data)
{
    auto const height = compared_data.size();
    auto const width = compared_data.at(0).size();

    adjacent_type adjacent_data(
        height,
        std::vector<direction_type<point_type>>(width)
        );

    for(int i=0; i<height; ++i) for(int j=0; j<width; ++j)
    {
        uint64_t norm_rl, norm_lr, norm_ud, norm_du;
        norm_rl = norm_lr = norm_ud = norm_du = std::numeric_limits<uint64_t>::max();

        for(int k=0; k<height; ++k) for(int l=0; l<width; ++l)
        {
            if(i==k && j==l) continue;

            if(compared_data[i][j][k][l].right < norm_rl)
            {
                norm_rl = compared_data[i][j][k][l].right;
                adjacent_data[i][j].right = point_type{l, k};
            }
            if(compared_data[i][j][k][l].left < norm_lr)
            {
                norm_lr = compared_data[i][j][k][l].left;
                adjacent_data[i][j].left = point_type{l, k};
            }
            if(compared_data[i][j][k][l].up < norm_ud)
            {
                norm_ud = compared_data[i][j][k][l].up;
                adjacent_data[i][j].up = point_type{l, k};
            }
            if(compared_data[i][j][k][l].down < norm_du)
            {
                norm_du = compared_data[i][j][k][l].down;
                adjacent_data[i][j].down = point_type{l, k};
            }
        }
    }

    return adjacent_data;
}

int pixel_sorter::yrange2(std::vector<std::vector<int>>& out, adjacent_type const& adjacent_data, compared_type const& compared_data) const
{
    auto const height = out.size();
    auto const width = out.at(0).size();

    auto const exists = [height, width](point_type const& p)
    {
        return 0 <= p.x && p.x < width && 0 <= p.y && p.y < height;
    };

    std::vector<std::vector<point_type>> sorted_matrix(
        height*2-1,
        std::vector<point_type>(
            width*2-1,
            {std::numeric_limits<int>::max(), std::numeric_limits<int>::max()}
            )
        );

    std::vector<std::vector<std::vector<int>>> answer;

    //すべてのピースから並べ始めるためのループ
    for(int c_y=0; c_y<out.size(); ++c_y) for(int c_x=0; c_x<out.at(0).size(); ++c_x)
    {
        sorted_matrix[height-1][width-1] = point_type{c_x, c_y};

        //上に見ていく
        for(int i=0; i<height-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1-i][width-1];
            sorted_matrix[height-2-i][width-1] = adjacent_data[adjacent.y][adjacent.x].up;
            if(sorted_matrix[height-2-i][width-1].y < 0 || sorted_matrix[height-2-i][width-1].y >= height) break;
        }
        //下に見ていく
        for(int i=0; i<height-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1+i][width-1];
            sorted_matrix[height+i][width-1] = adjacent_data[adjacent.y][adjacent.x].down;
            if(sorted_matrix[height+i][width-1].y < 0 || sorted_matrix[height+i][width-1].y >= height) break;
        }
        //右に見ていく
        for(int i=0; i<width-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1][width-1+i];
            sorted_matrix[height-1][width+i] = adjacent_data[adjacent.y][adjacent.x].right;
            if(sorted_matrix[height-1][width+i].x < 0 || sorted_matrix[height-1][width+i].x >= width) break;
        }
        //左に見ていく
        for(int i=0; i<width-1; ++i)
        {
            auto const& adjacent = sorted_matrix[height-1][width-1-i];
            sorted_matrix[height-1][width-2-i] = adjacent_data[adjacent.y][adjacent.x].left;
            if(sorted_matrix[height-1][width-2-i].x < 0 || sorted_matrix[height-1][width-2-i].x >= width) break;
        }

        //中心を除き上に向かってループ，右に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height - i - 1][width + j - 1];
            auto const& upper  = sorted_matrix[height - i - 2][width + j - 1];
            auto const& right  = sorted_matrix[height - i - 1][width + j    ];

            if(exists(center) && exists(upper) && exists(right))
                sorted_matrix[height - i - 2][width + j] = ur_choose(compared_data, upper, center, right);
            else
                break;
        }
        //中心を除き上に向かってループ，左に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height - i - 1][width - j - 1];
            auto const& upper  = sorted_matrix[height - i - 2][width - j - 1];
            auto const& left   = sorted_matrix[height - i - 1][width - j - 2];

            if(exists(center) && exists(upper) && exists(left))
                sorted_matrix[height - i - 2][width - j - 2] = ul_choose(compared_data, upper,left, center);
            else
                break;
        }
        //中心を除き下に向かってループ，右に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height + i - 1][width + j - 1];
            auto const& lower  = sorted_matrix[height + i    ][width + j - 1];
            auto const& right  = sorted_matrix[height + i - 1][width + j    ];
            
            if(exists(center) && exists(lower) && exists(right))
                sorted_matrix[height + i][width + j] = dr_choose(compared_data, center, right, lower);
            else
                break;
        }
        //中心を除き下に向かってループ，左に見ていく
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
            auto const& center = sorted_matrix[height + i - 1][width - j - 1];
            auto const& lower  = sorted_matrix[height + i    ][width - j - 1];
            auto const& left   = sorted_matrix[height + i + j][width - j - 2];
            
            if(exists(center) && exists(lower) && exists(left))
                sorted_matrix[height + i][width + j] = dl_choose(compared_data, left, center, lower);
            else
                break;
        }

        std::vector<std::vector<int>> one_answer;
        
        for(int i=0; i<height-1; ++i) for(int j=0; j<width-1; ++j)
        {
        }

        answer.push_back(std::move(one_answer));
    }

#ifdef DEBUG
    std::cout << "There are " << answer.size() << " solutions" << std::endl;
#endif

    return 0;

}

///*ans表示関数*/
//void RestoreImage::ShowAns()
//{
//	for (int k = 0; k < outputnum; k++){
//		std::cout << "**********ShowAns**********" << std::endl;
//		for (int i = 0; i < sepy; i++){
//			for (int j = 0; j < sepx; j++){
//				std::cout << " " << std::setw(3) << ans[k][i][j];
//			}
//			std::cout << std::endl;
//		}
//		std::cout << std::setw(3) << k + 1 << "個目の解" << "****************" << std::endl;
//	}
//}
//
///*指定した範囲の配列の和を返す関数*/
//int RestoreImage::ArrySum(int i, int j)
//{
//	int s = 0;
//	for (int y = 0; y < sepy; y++){
//		for (int x = 0; x < sepx; x++){
//			if (sorted_matrix_y[i + x][j + y] > 1000)sorted_matrix_y[i + x][j + y] = 777;
//			s += sorted_matrix_y[i + x][j + y];
//		}
//	}
//	return s;
//}
