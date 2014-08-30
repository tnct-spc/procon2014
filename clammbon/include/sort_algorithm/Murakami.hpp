#include <vector>
#include <data_type.hpp>
//人名(村上剛基)なのでキャメルケースです
class Murakami
{
private:
	typedef std::vector<std::vector<point_type>> return_type;
	question_raw_data const& data_;
	compared_type const& comp_;

	//ブロック(分割画像のかたまり),サイズは分割数(m,n)で固定,空白(分割画像の入らない部分)にはpoint_type(x = -1,y = -1)が入る
	typedef std::vector<std::vector<point_type>> block_type;//サイズを固定し,point_typeを初期化する
	enum direction{
		up,right,down,left
	};
	//作ったブロックをしまっておくlist,この要素が1になったら復元画像完成(ただしはみ出ないものとする)
	//std::vector<block_type> block_list; //std::mapで一緒に評価値を持たせてもいいかも////cppに移動
	//一枚目      二枚目  一枚目から見た向き 順位(10位までの11要素,0は自分)
	//[point_type][UDRL][Rank]の順でアクセスする,中身は分割画像
	std::map <point_type, std::vector < std::vector<point_type>>> sorted_comparation(point_type target);//これのラッパー関数があったほうがいい?
	//eval_blockの返り値のための構造体
	struct block_combination{
		//ここでブロックへの参照を保持しないと結合後のブロックを消せない
		block_type block1;
		block_type block2;
		//あるブロックとの結合するとき(結合したあと)の評価値
		std::int_fast64_t score; //評価値は結構大きいので32bitだとオーバーフローするかも?
	};
	//ブロック対ブロックの評価,ブロックに持たせるべき?
	block_combination eval_block(block_type, block_type);
	//ピース対ピースの評価,仕様をよく理解しないと書くのが辛い
	std::int_fast64_t eval_piece(point_type, point_type);
	block_type combine_block(block_type, block_type);
public:
	Murakami(question_raw_data const& data, compared_type const& comp);
	virtual ~Murakami() = default;
	std::vector<std::vector<std::vector<point_type>>> operator() ();
};