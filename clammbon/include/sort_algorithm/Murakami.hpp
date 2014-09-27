#include <vector>
#include <data_type.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <unordered_map>
//人名(村上剛基)なのでキャメルケースです
class Murakami
{
private:
	typedef std::vector<std::vector<point_type>> return_type;
	typedef boost::multiprecision::cpp_int bigint;//
	question_raw_data const& data_;
	compared_type const& comp_;

	//ブロック(分割画像のかたまり),サイズは分割数(m,n)で固定,空白(分割画像の入らない部分)にはpoint_type(x = -1,y = -1)が入る
	typedef std::vector<std::vector<point_type>> block_type;//サイズを固定し,point_typeを初期化する
	enum direction{
		up,right,down,left
	};
	//作ったブロックをしまっておくlist,この要素が1になったら復元画像完成(ただしはみ出ないものとする)
	//std::vector<block_type> block_list; //std::unordered_mapで一緒に評価値を持たせてもいいかも////cppに移動
	//[point_type][UDRL][Rank]の順でアクセスする,中身は分割画像
	std::unordered_map <point_type, std::vector < std::vector<point_type>>> sorted_comparation;//これのラッパー関数があったほうがいい?
	void sort_compare();
	//eval_blockの返り値のための構造体
	struct block_combination{
		//ここでブロックへの参照を保持しないと結合後のブロックを消せない
		block_type block1;
		block_type block2;
		int shift_x;
		int shift_y;
		//あるブロックとの結合するとき(結合したあと)の評価値
		std::int_fast64_t score; //評価値は結構大きいので32bitだとオーバーフローするかも?
	};
	//ブロック対ブロックの評価,ブロックに持たせるべき?
	block_combination eval_block(const block_type&, const block_type&);
	//ピース対ピースの評価,仕様をよく理解しないと書くのが辛い
	std::int_fast64_t eval_piece(const point_type&, const point_type&,direction);
	std::int_fast64_t eval_comp_(const point_type&, const point_type&,direction);
	block_type combine_block(const block_combination&);
	void make_sorted_comparation();
	typedef point_type block_size_type;
public:
	Murakami(question_raw_data const& data, compared_type const& comp);
	virtual ~Murakami() = default;
	std::vector<std::vector<std::vector<point_type>>> operator() ();
};
