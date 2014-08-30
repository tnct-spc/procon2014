#include <vector>
#include <data_type.hpp>
//�l��(���㍄��)�Ȃ̂ŃL�������P�[�X�ł�
class Murakami
{
private:
	typedef std::vector<std::vector<point_type>> return_type;
	question_raw_data const& data_;
	compared_type const& comp_;

	//�u���b�N(�����摜�̂����܂�),�T�C�Y�͕�����(m,n)�ŌŒ�,��(�����摜�̓���Ȃ�����)�ɂ�point_type(x = -1,y = -1)������
	typedef std::vector<std::vector<point_type>> block_type;//�T�C�Y���Œ肵,point_type������������
	enum direction{
		up,right,down,left
	};
	//������u���b�N�����܂��Ă���list,���̗v�f��1�ɂȂ����畜���摜����(�������͂ݏo�Ȃ����̂Ƃ���)
	//std::vector<block_type> block_list; //std::map�ňꏏ�ɕ]���l���������Ă���������////cpp�Ɉړ�
	//�ꖇ��      �񖇖�  �ꖇ�ڂ��猩������ ����(10�ʂ܂ł�11�v�f,0�͎���)
	//[point_type][UDRL][Rank]�̏��ŃA�N�Z�X����,���g�͕����摜
	std::map <point_type, std::vector < std::vector<point_type>>> sorted_comparation(point_type target);//����̃��b�p�[�֐����������ق�������?
	//eval_block�̕Ԃ�l�̂��߂̍\����
	struct block_combination{
		//�����Ńu���b�N�ւ̎Q�Ƃ�ێ����Ȃ��ƌ�����̃u���b�N�������Ȃ�
		block_type block1;
		block_type block2;
		//����u���b�N�Ƃ̌�������Ƃ�(������������)�̕]���l
		std::int_fast64_t score; //�]���l�͌��\�傫���̂�32bit���ƃI�[�o�[�t���[���邩��?
	};
	//�u���b�N�΃u���b�N�̕]��,�u���b�N�Ɏ�������ׂ�?
	block_combination eval_block(block_type, block_type);
	//�s�[�X�΃s�[�X�̕]��,�d�l���悭�������Ȃ��Ə����̂��h��
	std::int_fast64_t eval_piece(point_type, point_type);
	block_type combine_block(block_type, block_type);
public:
	Murakami(question_raw_data const& data, compared_type const& comp);
	virtual ~Murakami() = default;
	std::vector<std::vector<std::vector<point_type>>> operator() ();
};