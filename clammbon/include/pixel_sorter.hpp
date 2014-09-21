#ifndef RESOLVER_PIXEL_SORTER_HPP
#define RESOLVER_PIXEL_SORTER_HPP

#include <iostream>
#include "data_type.hpp"
#include "splitter.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

template<class Algorithm>
class pixel_sorter
{
public:
    // 座標(j,i)と座標(l,k)の比較結果としては，compared_type[i][j][k][l]にtuple<上,右,下,左>で入る
    // compared_typeより，一番距離が近い断片画像をadjacent_typeに整理する
    typedef std::vector<std::vector<std::vector<std::vector<direction_type<uint_fast64_t>>>>> compared_type;
    typedef std::vector<std::vector<direction_type<point_type>>> adjacent_type;
    typedef std::vector<std::vector<point_type>> return_type;

    pixel_sorter<Algorithm>() : split_(){}
    virtual ~pixel_sorter<Algorithm>() = default;

    return_type operator() (question_raw_data const& raw) const
    {
        auto const& proposed = proposed_answer(raw);

        // TODO: ここで返却されたデータのうち，適しているものを選択してreturn

		if (proposed.size() == 0) return std::vector < std::vector<point_type> > {};
		else return proposed[0];
    }

    std::vector<return_type> proposed_answer(question_raw_data const& raw) const
    {
        //
        // Sub Algorithm
        // 正しい位置に並べた時に左上から，1~nまでの番号をふり，それが今どこにあるのかという情報をreturn
        //

        split_image_type const& splited_image = split_.split_image(raw);
        compared_type const& comp = this->image_comp(raw,splited_image);

        Algorithm algo(raw, comp);
        auto const& proposed = algo();

        return proposed;
    }

private:

    //2つのピクセル間の距離を2乗した値を返却．result = r^2 + g^2 + b^2
    int pixel_comparison(pixel_type const& lhs, pixel_type const& rhs) const
    {
        int s = 0;
        for(int i=0; i<3; ++i) s += static_cast<int>(std::abs(lhs[i] - rhs[i]));
        return s;
    }

    //2つのピクセル列を比較した値を返す
    int pixel_line_comparison(image_type const& lhs, image_type const& rhs) const
    {
        auto lhs_it = lhs.begin(), lhs_end = lhs.end();
        auto rhs_it = rhs.begin(), rhs_end = rhs.end();

        int s = 0;
        for(; lhs_it != lhs_end && rhs_it != rhs_end; ++lhs_it, ++rhs_it)
        {
            s += pixel_comparison(*lhs_it, *rhs_it);
        }
        return s;
    }

    //一枚目の右端と二枚目の左端を見る関数, 一致が多いほど低いを返す
    uint_fast64_t rl_comparison(image_type const& lhs, image_type const& rhs) const
    {
        return pixel_line_comparison(lhs.col(lhs.cols - 1), rhs.col(0));
    }

    //一枚目の左端と二枚目の右端を見る関数, 一致が多いほど低いを返す
    uint_fast64_t lr_comparison(image_type const& lhs, image_type const& rhs) const
    {
        return pixel_line_comparison(lhs.col(0), rhs.col(rhs.cols - 1));
    }

    //一枚目の上端と二枚目の下端を見る関数, 一致が多いほど低いを返す
    uint_fast64_t ud_comparison(image_type const& lhs, image_type const& rhs) const
    {
        return pixel_line_comparison(lhs.row(0), rhs.row(rhs.rows - 1));
    }

    //一枚目の下端と二枚目の上端を見る関数, 一致が多いほど低いを返す
    uint_fast64_t du_comparison(image_type const& lhs, image_type const& rhs) const
    {
        return pixel_line_comparison(lhs.row(rhs.rows - 1), rhs.row(0));
    }

    compared_type image_comp(question_raw_data const& data_,split_image_type const& image) const
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
		

		//################失礼します##########################//

		int const sepx = image[0].size();
		int const sepy = image.size();
		int const one_picy = (data_.size.second / data_.split_num.second);// picy/sepy
		int const one_picx = (data_.size.first / data_.split_num.first);// picx/sepx

		cv::Mat prob_img = cv::imread("prob01.ppm", 1);

		std::vector<std::vector<cv::Mat>>mat_vec(sepy, std::vector<cv::Mat>(sepx));
		std::vector<cv::Mat> mat_vec1;
		for (int i = 0; i < sepy; i++){
			for (int j = 0; j < sepx; j++){
				cv::Rect roi(j*one_picx, i*one_picy, one_picx, one_picy);//x, yからpicx / sepx, picy / sepyでROIを設定
				mat_vec1.push_back(prob_img(roi).clone());
				mat_vec[i][j] = prob_img(roi).clone();
				//mat_vec[i][j] = data_.pixels(roi).clone();// 深いコピー
			}
		}
		
		splitter sp;
		split_image_type splited = sp.split_image(data_);
				
        for(int i=0; i<image.size(); ++i) for(int j=0; j<image[0].size(); ++j)
        {
            for(int k=0; k<image.size(); ++k) for(int l=0; l<image[0].size(); ++l)
            {
                if(k > i || (k == i && l > j)) // (j,i)より(l,k)の方が，探索順として後半に或るための条件
                {
                    //順序を変えて逆の逆の組み合わせの相対評価は同じであることを使用して探索量を半分にする
                    //例えば，Aから見たBは上なら，Bから見たAは下．
					/*
                    comp[i][j][k][l].up    = comp[k][l][i][j].down  = ud_comparison(image[i][j], image[k][l]);
                    comp[i][j][k][l].right = comp[k][l][i][j].left  = rl_comparison(image[i][j], image[k][l]);
                    comp[i][j][k][l].down  = comp[k][l][i][j].up    = du_comparison(image[i][j], image[k][l]);
                    comp[i][j][k][l].left  = comp[k][l][i][j].right = lr_comparison(image[i][j], image[k][l]);
					*/

					comp[i][j][k][l].up    = comp[k][l][i][j].down  = ud_comparison(splited[i][j], splited[k][l]);
					comp[i][j][k][l].right = comp[k][l][i][j].left  = rl_comparison(splited[i][j], splited[k][l]);
					comp[i][j][k][l].down  = comp[k][l][i][j].up    = du_comparison(splited[i][j], splited[k][l]);
					comp[i][j][k][l].left  = comp[k][l][i][j].right = lr_comparison(splited[i][j], splited[k][l]);
					/*
					std::cout << i * 3 + j << "," << k * 3 + l << " " << ud_comparison(splited[i][j], splited[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << rl_comparison(splited[i][j], splited[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << du_comparison(splited[i][j], splited[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << lr_comparison(splited[i][j], splited[k][l]) << std::endl;
					*/
					std::cout << i * 3 + j << "," << k * 3 + l << " " << ud_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << rl_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << du_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;
					std::cout << i * 3 + j << "," << k * 3 + l << " " << lr_comparison(mat_vec[i][j], mat_vec[k][l]) << std::endl;

				}
            }
        }
		/*
		for (int i = 0; i < sepx*sepy; ++i){
			for (int j = 0; j < sepx*sepy; ++j){
				std::cout << i << "," << j << " " << ud_comparison(mat_vec1[i], mat_vec1[j]) << std::endl;
				std::cout << i << "," << j << " " << rl_comparison(mat_vec1[i], mat_vec1[j]) << std::endl;
				std::cout << i << "," << j << " " << du_comparison(mat_vec1[i], mat_vec1[j]) << std::endl;
				std::cout << i << "," << j << " " << lr_comparison(mat_vec1[i], mat_vec1[j]) << std::endl;

			}
		}
		*/

		cv::Mat file1 = cv::imread("a.png", 1);
		cv::Mat file2 = cv::imread("b.png", 1);
		
		std::cout << "file1 , file2 " << ud_comparison(file1, file2) << std::endl;
		std::cout << "file1 , file2 " << rl_comparison(file1, file2) << std::endl;
		std::cout << "file1 , file2 " << du_comparison(file1, file2) << std::endl;
		std::cout << "file1 , file2 " << lr_comparison(file1, file2) << std::endl;
       
		return comp;
    }

    splitter const split_;
};

#endif
