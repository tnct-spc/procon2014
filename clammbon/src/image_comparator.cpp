#include "image_comparator.hpp"


//2つのピクセル間の距離を2乗した値を返却．result = |r| + |g| + |b|
int image_comparator::pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
{
    int s = 0;
    for(int i=0; i<3; ++i) s += static_cast<int>(std::abs(lhs[i] - rhs[i]));
    return s;
}
//2つのピクセル列を比較した値を返す
int image_comparator::pixel_line_comparison(image_type const& lhs, image_type const& rhs) const
{
    auto lhs_it = lhs.begin(), lhs_end = lhs.end();
    auto rhs_it = rhs.begin(), rhs_end = rhs.end();

    int s = 0;
	//#pragma omp parallel for
    for(; lhs_it != lhs_end && rhs_it != rhs_end; ++lhs_it, ++rhs_it)
    {
        s += pixel_comparison(*lhs_it, *rhs_it);
    }
    return s;
}

//一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
uint_fast64_t image_comparator::rl_comparison(image_type const& lhs, image_type const& rhs) const
{
    return pixel_line_comparison(lhs.col(lhs.cols - 1), rhs.col(0));
}

//一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
uint_fast64_t image_comparator::lr_comparison(image_type const& lhs, image_type const& rhs) const
{
    return pixel_line_comparison(lhs.col(0), rhs.col(rhs.cols - 1));
}

//一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
uint_fast64_t image_comparator::ud_comparison(image_type const& lhs, image_type const& rhs) const
{
    return pixel_line_comparison(lhs.row(0), rhs.row(rhs.rows - 1));
}

//一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
uint_fast64_t image_comparator::du_comparison(image_type const& lhs, image_type const& rhs) const
{
    return pixel_line_comparison(lhs.row(rhs.rows - 1), rhs.row(0));
}

compared_type image_comparator::image_comp(question_raw_data const& data_,split_image_type const& image) const
{
    //返却用変数
    //"最大距離が収納されたtuple"の4次元配列になる
    compared_type comp(
        image.size(),
        std::vector<std::vector<std::vector<direction_type<uint_fast64_t>>>>(
            image[0].size(),
            std::vector<std::vector<direction_type<uint_fast64_t>>>(
                image.size(),
                std::vector<direction_type<uint_fast64_t>>(
                    image[0].size(),
                    {
                        std::numeric_limits<uint_fast64_t>::max(),
                        std::numeric_limits<uint_fast64_t>::max(),
                        std::numeric_limits<uint_fast64_t>::max(),
                        std::numeric_limits<uint_fast64_t>::max()
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
				/*
				std::cout << i * 3 + j << "," << k * 3 + l << " " << ud_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
				std::cout << i * 3 + j << "," << k * 3 + l << " " << rl_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
				std::cout << i * 3 + j << "," << k * 3 + l << " " << du_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
				std::cout << i * 3 + j << "," << k * 3 + l << " " << lr_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
				*/
			}
        }
    }

	return comp;
}

//2つのピクセル間の距離を2乗した値を返却．result = sqrt(r^2 + g^2 + b^2)
int image_comparator_dx::pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
{
	int s = 0;
	for (int i = 0; i<3; ++i) s += static_cast<int>(std::pow((lhs[i] - rhs[i]),2));
	return std::sqrt(s);
}
