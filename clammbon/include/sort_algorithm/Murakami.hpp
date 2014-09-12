#include <vector>
#include <data_type.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <map>
//�l��(���㍄��)�Ȃ̂ŃL�������P�[�X�ł�
class Murakami
{
private:
	typedef std::vector<std::vector<point_type>> return_type;
	typedef boost::multiprecision::cpp_int bigint;//
	question_raw_data const& data_;
	compared_type const& comp_;

	//�u���b�N(�����摜�̂����܂�),�T�C�Y�͕�����(m,n)�ŌŒ�,��(�����摜�̓���Ȃ�����)�ɂ�point_type(x = -1,y = -1)������
	typedef std::vector<std::vector<point_type>> block_type;//�T�C�Y���Œ肵,point_type������������
	enum direction{
		up,right,down,left
	};
	//������u���b�N�����܂��Ă���list,���̗v�f��1�ɂȂ����畜���摜����(�������͂ݏo�Ȃ����̂Ƃ���)
	//std::vector<block_type> block_list; //std::map�ňꏏ�ɕ]���l���������Ă���������////cpp�Ɉړ�
	//[point_type][UDRL][Rank]�̏��ŃA�N�Z�X����,���g�͕����摜
	std::map <point_type, std::vector < std::vector<point_type>>> sorted_comparation;//����̃��b�p�[�֐����������ق�������?
	void sort_compare();
	//eval_block�̕Ԃ�l�̂��߂̍\����
	struct block_combination{
		//�����Ńu���b�N�ւ̎Q�Ƃ�ێ����Ȃ��ƌ�����̃u���b�N�������Ȃ�
		block_type block1;
		block_type block2;
		int shift_x;
		int shift_y;
		//����u���b�N�Ƃ̌�������Ƃ�(������������)�̕]���l
		std::int_fast64_t score; //�]���l�͌��\�傫���̂�32bit���ƃI�[�o�[�t���[���邩��?
	};
	//�u���b�N�΃u���b�N�̕]��,�u���b�N�Ɏ�������ׂ�?
	block_combination eval_block(block_type, block_type);
	//�s�[�X�΃s�[�X�̕]��,�d�l���悭�������Ȃ��Ə����̂��h��
	std::int_fast64_t eval_piece(point_type, point_type,direction);
	std::int_fast64_t eval_comp_(point_type, point_type,direction);
	block_type combine_block(block_combination);
	void make_sorted_comparation();
public:
	Murakami(question_raw_data const& data, compared_type const& comp);
	virtual ~Murakami() = default;
	std::vector<std::vector<std::vector<point_type>>> operator() ();
};