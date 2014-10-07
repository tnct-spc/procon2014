//////////////////////////
//�ŗD��T���A���S���Y��//
//////////////////////////

#define debug
//#define test1
//#define nosaiki
//#define debugdebug

//�C���N���[�h�t�@�C��
#ifdef debug
#endif
#include "algorithm_2.hpp"
#include <cmath>
#include <boost/timer/timer.hpp>

//�R���X�g���N�^
heap::heap(){
}
//�f�X�g���N�^
heap::~heap(){
}


void heap::setup(int in_y, int in_x){
	y = in_y;
	x = in_x;
	cost.resize(1000000);
	history.resize(1000000);
	history_limit.resize(1000000);
	table.resize(1000000);
	heaptable.resize(1000000);
}
void heap::pop(int *in_cost, int *in_table, std::vector<int> &in_history, int *in_history_limit){

	long long int i = 0;
	long long int me;
	long long int top = pos;

	if (size > 1000000 * size_million - 1){
#ifdef debug
		std::cout << "><heap.cpp vector pass1,000,000" << std::endl;
#endif
		size_million++;
		cost.resize(1000000 * size_million);
		history.resize(1000000 * size_million);
		history_limit.resize(1000000 * size_million);
		table.resize(1000000 * size_million);
		heaptable.resize(1000000 * size_million);
	}

	//�R�X�g�}��
	cost[size] = *in_cost;
	//����}��
	i = 0;
	history[size].resize(*in_history_limit);
	while (i < *in_history_limit){
		history[size][i] = (in_history[i]);
		i++;
	}
	history_limit[size] = *in_history_limit;
	//�e�[�u���}��
	table[size].resize(y*x);
	for (i = 0; i < y*x; i++){
		table[size][i] = (in_table[i]);
	}
	//�q�[�v�z��}��
	while (1){
		me = top;
		top = (top - 1) / 2;
		if (me == top){
			break;
		}
		else if (cost[heaptable[top]] > *in_cost){
			heaptable[me] = heaptable[top];
		}
		else{
			break;
		}
	}
	heaptable[me] = size;

	pos++;
	size++;

}

void heap::push(int *out_cost, int *out_table, std::vector<int> &out_history, int *out_history_limit){
	long long int out = heaptable[0];

	long long int i = 0;
	long long int me = 0;
	long long int bottom = 0;

	pos--;
	size--;
	//�R�X�g�}��
	*out_cost = cost[heaptable[0]];
	//����}��
	i = 0;
	*out_history_limit = history_limit[heaptable[0]];
	while (i < *out_history_limit){
		out_history[i] = history[heaptable[0]][i];
		i++;
	}
	//�e�[s�u���}��
	for (i = 0; i < table[heaptable[0]].size(); i++){
		out_table[i] = table[heaptable[0]][i];
	}
	//std::cout << "num=" << heaptable[0] << std::endl;
	//std::cout << "pos="<<pos<<" size="<<size << std::endl;
	//�q�[�v�z��}��
	while (1){
		me = bottom;
		bottom = (bottom * 2);
		if (pos>bottom + 2){
			if (cost[heaptable[bottom + 1]] < cost[heaptable[bottom + 2]]){
				if (cost[heaptable[pos]] > cost[heaptable[bottom + 1]]){
					bottom += 1;
					//std::cout << "" << me << " << " << bottom << std::endl;
					heaptable[me] = heaptable[bottom];
				}
				else{
					break;
				}
			}
			else{
				if (cost[heaptable[pos]] > cost[heaptable[bottom + 2]]){
					bottom += 2;
					//std::cout << "" << me << " << " << bottom << std::endl;
					heaptable[me] = heaptable[bottom];
				}
				else{
					break;
				}
			}
		}
		else if (pos>bottom + 1){
			if (cost[heaptable[pos]] > cost[heaptable[bottom + 1]]){
				bottom += 1;
				//std::cout << "" << me << " << " << bottom << std::endl;
				heaptable[me] = heaptable[bottom];
			}
			else{
				break;
			}
		}
		else{
			break;
		}
	}
	//std::cout << "me=" << me << "pos=" << pos << std::endl;
	heaptable[me] = heaptable[pos];
	heaptable.erase(heaptable.end() - 1);

	size++;
}









//��������A���S���Y���̃R�[�h







//�R���X�g���N�^
algorithm_2::algorithm_2()
{
}

//�f�X�g���N�^
algorithm_2::~algorithm_2()
{
	delete[] table;
	delete[] sub_table;
}

//������
void algorithm_2::reset(question_data const& data)
{
	// �f�[�^�̃N���[��
	data_ = data.clone();

	// ���ƍ���
	y_size = data_->size.second;
	x_size = data_->size.first;
	size = y_size*x_size;

	// �R�X�g�ƃ��[�g
	cost_select = data_->cost_select;
	cost_change = data_->cost_change;

	// �e�[�u��
	//table�ɂ悻�������Ă����}�g���N�X�𐮗����đ}��
	table = new int[size];//y*x�T�C�Y�̃e�[�u�������
	sub_table = new int[size];//y*x�T�C�Y�̃e�[�u�������
	for (int y = 0; y_size > y; y++){
		for (int x = 0; x_size > x; x++){
			table[y * x_size + x] = data_->block[y][x].y * x_size + data_->block[y][x].x;
			sub_table[y * x_size + x] = table[y * x_size + x];
		}
	}

	//�z��̂�T�C�Y
	history.resize(1000000);
	sub_history.resize(1000000);
	keiro.resize(1000000);

	//�ϐ��̏�����
	history_limit = 0;
	sub_history_limit = 0;

	//heap�̏�����
	harray.setup(y_size, x_size);

}





auto algorithm_2::get() -> boost::optional<answer_type>
{
#ifdef debug
	std::cout << "algorythm_2 start" << std::endl;
	boost::timer::cpu_timer timer; // ���Ԍv�����J�n

	std::cout << "x_size=" << x_size << ",y_size=" << y_size << std::endl;
	for (int y = 0; y_size > y; y++){
		for (int x = 0; x_size > x; x++){
			std::cout << table[y * x_size + x] << ",";
		}
		std::cout << std::endl;
	}
#endif
	std::vector<int> get_history(0);
	int get_cost = 0;

#ifdef debug
	//�p�Y���X�^�[�g
	std::cout << "start" << std::endl;
#endif
	puzzle(get_history, get_cost);	//���s
#ifdef debug
	std::cout << "end" << std::endl;
	std::string result = timer.format();
	std::cout << "\n��������:" << result << std::endl;
	for (int i = 0; i < get_history.size(); i++){
		switch (get_history[i]){
			case 16:
			case 20:
				std::cout << ",U";
				break;
			case 17:
			case 21:
				std::cout << ",R";
				break;
			case 18:
			case 22:
				std::cout << ",D";
				break;
			case 19:
			case 23:
				std::cout << ",L";
				break;
			default:
				std::cout << "," << get_history[i];
				break;
		}
	}
	int debug_S = 0, debug_C = 0;
	for (int i = 0; i < get_history.size(); i++){
		if (get_history[i] < 16){
			debug_S += 1;
		}
		else{
			debug_C += 1;
		}
	}
	debug_S /= 2;
	std::cout << "\ncost=" << debug_S * cost_select + debug_C * cost_change << " S: " << debug_S << " C: " << debug_C << std::endl;
#endif
	//����answer_type�ɂ��ĕԂ�
	answer_type answerlist;
	point_type position;
	std::ostringstream stream;
	answerlist.list.resize(1024);//����
	int pos = 0;
	for (int count=0; pos < get_history.size();count++){
		position.y = get_history[pos];
		pos++;
		position.x = get_history[pos];
		pos++;
		while(pos < get_history.size() && 16 <= get_history[pos]){
			switch (get_history[pos]){
			case 16:
			case 20:
				stream << "U";
				break;
			case 17:
			case 21:
				stream << "R";
				break;
			case 18:
			case 22:
				stream << "D";
				break;
			case 19:
			case 23:
				stream << "L";
				break;
			}
			pos++;
		}
		answerlist.list[count] = { position, stream.str() };
		stream.str("");
	}
	return answerlist;
}













//�T��
int algorithm_2::puzzle(std::vector<int> &out_history, int &out_cost){
	int count, i;


	while (1){
		algorithm_2::prescanning(); //�T��
#ifdef test1
		//�����������I�Ȏ���������
		//�A���ŗ�������Ȃ��͔̂�΂�
		while (1){
			harray.push(&cost, table, history, &history_limit);
			int buff = 0;
			for (i = 0; i < size; i++){
				if (sub_table[i] == table[i])buff++;
			}
			for (i = 0; i < size; i++){
				sub_table[i] = table[i];
			}
			if (buff != size){
				//�O��ƈႤ�}�X�ʒu
				break;
			}
		}
		//�������������܂Ł�������
#else
		harray.push(&cost, table, history, &history_limit);
#endif
		//debug
		/*
		for (int i = 0; i < 3; i++){
			for (int j = 0; j < 3; j++){
				std::cout << table[i * 3 + j] << ",";
			}
			std::cout<<std::endl;
		}
		for (int i = 0; i < history_limit; i++){
			std::cout << history[i] << ",";
		}
		std::cout << std::endl;
		std::cout << "count=" << count<<std::endl;
		int f;
		std::cin >> f;
		*/
		//��������
		count=0;
		for (i=0;i<size;i++){
			if (table[i] == i) count++;
		}
		if (count == size){
			//����
			out_cost = cost;
			out_history.resize(history_limit);
			for (i = 0; i < history_limit; i++){
				out_history[i] = history[i];
			}
			return 0;
		}
	}
}

//��������(�S�Ẵ}�X����Â�scaning�Œ��ׂ�)
void algorithm_2::prescanning(){
	for (int y = 0; y < y_size; y++){
		for (int x = 0; x < x_size; x++){
			keiro[0] = y;
			keiro[1] = x;
			keiro_count = 2;
			scanning(y, x, y, x, -1);
		}
	}
}

//����
void algorithm_2::scanning(int y, int x, int y_before, int x_before, int URDL){
	int i, j, buff;
	bool feel, feeled;
	//�R�X�g
	int G, S, C;
#ifdef debugdebug
	for (i = 0; i < keiro_count; i++){
		std::cout << keiro[i] << ",";
	}
	std::cout << "x=" << x << " y=" << y << std::endl;
	for (int y_ = 0; y_ < y_size; y_++){
		for (int x_ = 0; x_ < x_size; x_++){
			if (y_ == y && x_ == x){
				std::cout << "����";
			}
			else{
				printf("%4d", table[y_ * x_size + x_]);
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl;
	std::cin >> i;
	//_sleep(30);
#endif
	if (keiro_count > 1000000 * size_million_keiro){
		size_million_keiro++;
		history.resize(1000000 * size_million_keiro);
		sub_history.resize(1000000 * size_million_keiro);
#ifdef debug
		std::cout << "><search.cpp keiro vector pass1,000,000" << std::endl;
#endif
	}
#ifdef nosaiki //����ōċA�����̒T�����o����
	if ((y == y_before && x == x_before)){
#endif
		//�i��(��E�����̏�)
		//��
		if (y > 0 && URDL != 2){
			if (table[y * x_size + x] / x_size < y){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[(y - 1) * x_size + x] / x_size > y - 1)
			{
				feeled = true;
			}
			else{
				feeled = false;
			}
			if (feel == true/* || feeled==true*/){
				if (feel == true && feeled == true){
					keiro[keiro_count] = 16;
				}
				else {
					keiro[keiro_count] = 20;
				}
				keiro_count += 1;
				//����
				buff = table[y * x_size + x];
				table[y * x_size + x] = table[(y - 1) * x_size + x];
				table[(y - 1) * x_size + x] = buff;
				scanning(y - 1, x, y, x, 0);
			}
		}
		//�E
		if (x < x_size - 1 && URDL != 3){
			if (table[y * x_size + x] % x_size > x){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[y * x_size + (x + 1)] % x_size < x + 1)
			{
				feeled = true;
			}
			else{
				feeled = false;
			}
			if (feel == true/* || feeled==true*/){
				if (feel == true && feeled == true){
					keiro[keiro_count] = 17;
				}
				else {
					keiro[keiro_count] = 21;
				}
				keiro_count += 1;
				//����
				buff = table[y * x_size + x];
				table[y * x_size + x] = table[y * x_size + (x + 1)];
				table[y * x_size + (x + 1)] = buff;
				scanning(y, x + 1, y, x, 1);
			}
		}
		//��
		if (y < y_size - 1 && URDL != 0){
			if (table[y * x_size + x] / x_size > y){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[(y + 1) * x_size + x] / x_size < y + 1)
			{
				feeled = true;
			}
			else{
				feeled = false;
			}
			if (feel == true/* || bbuff2==true*/){
				if (feel == true && feeled == true){
					keiro[keiro_count] = 18;
				}
				else {
					keiro[keiro_count] = 22;
				}
				keiro_count += 1;
				//����
				buff = table[y * x_size + x];
				table[y * x_size + x] = table[(y + 1) * x_size + x];
				table[(y + 1) * x_size + x] = buff;
				scanning(y + 1, x, y, x, 2);
			}
		}
		//��
		if (x > 0 && URDL != 1){
			if (table[y * x_size + x] % x_size < x){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[y * x_size + (x - 1)] % x_size > x - 1)
			{
				feeled = true;
			}
			else{
				feeled = false;
			}
			if (feel == true/* || bbuff2==true*/){
				if (feel == true && feeled == true){
					keiro[keiro_count] = 19;
				}
				else {
					keiro[keiro_count] = 23;
				}
				keiro_count += 1;
				//����
				buff = table[y * x_size + x];
				table[y * x_size + x] = table[y * x_size + (x - 1)];
				table[y * x_size + (x - 1)] = buff;
				scanning(y, x - 1, y, x, 3);
			}
		}
#ifdef nosaiki
	}
#endif
	if ((y != y_before || x != x_before)){//��ԍŏ��ȊO/* && ������ʌ݊������݂��Ȃ�(������)����*/
		if (history_limit > 1000000 * size_million_history){
			size_million_history++;
			history.resize(1000000 * size_million_history);
			sub_history.resize(1000000 * size_million_history);
#ifdef debug
			std::cout << "><search.cpp history vector pass1,000" << std::endl;
#endif
		}

		//�p�^�[���z��(�o�H)�ɕۑ�
		for (i = 0; i < keiro_count; i++){
			history[history_limit + i] = keiro[i];
		}
		history_limit += keiro_count;
		//��������shorting��������
		sub_history_limit = history_limit;
		for (i = 0; i < sub_history_limit; i++){
			sub_history[i] = history[i];
		}
		/*
		std::cout << "strt" << std::endl;
		std::cout << " limit=" << subhistory_limit << std::endl;
		for (i = 0; i < subhistory_limit; i++){
		std::cout << subhistory[i] << ",";
		}
		std::cout << std::endl;
		*/
		shorting();
		/*
		std::cout << "\nend" << std::endl;
		std::cout << " limit=" << subhistory_limit << std::endl;
		for (i = 0; i < subhistory_limit; i++){
		std::cout << subhistory[i] << ",";
		}
		std::cout << std::endl;
		std::cin >> i;
		*/
		//������������������������
		//cost = �I���R�X�g+�����R�X�g+�S�[���܂ł̋���;
		G = 0;
		S = 0;
		C = 0;
		//��芸�����S�[���܂ł̋��������Ă���
		for (i = 0; i < size; i++){
			G += abs(table[i] / x_size - i / x_size) + abs(table[i] % x_size - i % x_size);
		}
		//�����R�X�g�ƑI���R�X�g�������
		for (i = 0; i < sub_history_limit; i++){
			if (sub_history[i] < 16){
				S++;
			}
			else{
				C++;
			}
		}
		G *= goal;
		S *= sentaku;
		C *= coukan;
		cost = G + S + C;

		//1,1,1		�m�[�}��
		//10,5,3	�I�����Z���̂���ꂽ
		//10 8 4
		//10 14 6
		harray.pop(&cost, table, sub_history, &sub_history_limit);
		/*
		//���ő�I���񐔂��傫���Ȃ��߂�
		if (S / 15 < 3){
		harray.pop(&cost, table, subhistory, &subhistory_limit);
		}
		*/
		history_limit -= keiro_count;
	}
	//�S�����I�������Adata�����ʂ�ɂ��ĕԂ�(?)
	buff = table[y * x_size + x];
	table[y * x_size + x] = table[y_before * x_size + x_before];
	table[y_before * x_size + x_before] = buff;

	//�����o�H������
	keiro[keiro_count] = 0;
	if (keiro_count>2) keiro_count -= 1;

	y = y_before;
	x = x_before;
	//up
}
void algorithm_2::shorting(){
	int i;
	//�o�H���܂Ƃ߂ĒZ�k����
	int x1_head, y1_head, root1_count, y1_tail, x1_tail; std::vector<int> root1;
	int x2_head, y2_head, root2_count, y2_tail, x2_tail; std::vector<int> root2;
	int start, count = 0, end;
	root1.resize(256);//���̂��炢�ł�������
	root2.resize(256);
	/*
	std::cout << "keiro " << std::endl;

	for (int g = 0; g < 64; g++){
	std::cout << search_pattern_keiro[kaisou][count][g] << ",";
	}

	std::cout << "@" << std::endl;
	*/
	while (count + 3<sub_history_limit){
		/*
		std::cout << "c=" << count << " l=" << subhistory_limit << std::endl;
		for (i = 0; i < subhistory_limit; i++){
		std::cout << subhistory[i] << ",";
		}
		std::cin >> i;
		*/
		root1_count = 0;
		root2_count = 0;
		start = count;
		y1_head = sub_history[count];
		count++;
		x1_head = sub_history[count];
		count++;
		while (1){
			if (!(count<sub_history_limit && sub_history[count] >= 16 && sub_history[count] <= 23)) break;//(-1�܂���0����size)�ȊO
			root1[root1_count] = sub_history[count];
			root1_count++;
			count++;
		}
		if (!(count<sub_history_limit)){
			break;
		}
		end = count;
		y2_head = sub_history[end];
		end++;
		x2_head = sub_history[end];
		end++;
		while (1){
			if (!(end < sub_history_limit && sub_history[end] >= 16 && sub_history[end] <= 23)) break;
			root2[root2_count] = sub_history[end];
			root2_count++;
			end++;
		}
		//get tail
		y1_tail = y1_head;
		x1_tail = x1_head;
		for (i = 0; i < root1_count; i++){
			switch (root1[i]){
			case 16:
				y1_tail -= 1;
				break;
			case 17:
				x1_tail += 1;
				break;
			case 18:
				y1_tail += 1;
				break;
			case 19:
				x1_tail -= 1;
				break;
			case 20:
				y1_tail -= 1;
				break;
			case 21:
				x1_tail += 1;
				break;
			case 22:
				y1_tail += 1;
				break;
			case 23:
				x1_tail -= 1;
				break;
			}
		}
		y2_tail = y2_head;
		x2_tail = x2_head;
		for (i = 0; i < root2_count; i++){
			switch (root2[i]){
			case 16:
				y2_tail -= 1;
				break;
			case 17:
				x2_tail += 1;
				break;
			case 18:
				y2_tail += 1;
				break;
			case 19:
				x2_tail -= 1;
				break;
			case 20:
				y2_tail -= 1;
				break;
			case 21:
				x2_tail += 1;
				break;
			case 22:
				y2_tail += 1;
				break;
			case 23:
				x2_tail -= 1;
				break;
			}
		}
		//Rule1
		if (y1_tail == y2_head && x1_tail == x2_head){
			//std::cout << "rulu1" << std::endl;
			for (i = 0; i < root2_count; i++){
				sub_history[count] = root2[i];
				count++;
			}
			for (i = 0; i + end < sub_history_limit; i++){
				sub_history[count + i] = sub_history[end + i];
			}
			sub_history_limit = count + i;
			count = 0;
		}
		//Rule2
		else if (y1_tail == y2_tail && x1_tail == x2_tail && root2_count == 1){
			//std::cout << "rulu2" << std::endl;
			//root2�̊e�����𔽓]
			reverse(&root2[0]);
			sub_history[count] = root2[0];
			count++;
			for (i = 0; i + end < sub_history_limit; i++){
				sub_history[count + i] = sub_history[end + i];
			}
			sub_history_limit = count + i;
			count = 0;
		}
		//Rule3
		else if (y1_head == y2_head && x1_head == x2_head && root1_count == 1){
			//std::cout << "rulu3" << std::endl;
			//root1�̊e�����𔽓]
			reverse(&root1[0]);
			sub_history[start] = y1_tail;
			start++;
			sub_history[start] = x1_tail;
			start++;
			sub_history[start] = root1[0];
			start++;
			for (i = 0; i < root2_count; i++){
				sub_history[start] = root2[i];
				start++;
			}
			for (i = 0; i + end < sub_history_limit; i++){
				sub_history[start + i] = sub_history[end + i];
			}
			sub_history_limit = start + i;
			count = 0;
		}
		//Rule4
		else if (y1_head == y2_tail && x1_head == x2_tail && root1_count == 1 && root2_count == 1){
			//std::cout << "rulu4" << std::endl;
			//root1�̊e�����𔽓]
			reverse(&root1[0]);
			//root2�̊e�����𔽓]
			reverse(&root2[0]);
			sub_history[start] = y1_tail;
			sub_history[start + 1] = x1_tail;
			sub_history[start + 2] = root1[0];
			sub_history[start + 3] = root2[0];
			for (i = 0; i + end < sub_history_limit; i++){
				sub_history[start + 4 + i] = sub_history[end + i];
			}
			sub_history_limit = start + 4 + i;
			count = 0;
		}
	}
}
void algorithm_2::reverse(int *x){
	switch (*x){
	case 16:
		*x = 18;
		break;
	case 17:
		*x = 19;
		break;
	case 18:
		*x = 16;
		break;
	case 19:
		*x = 17;
		break;
	case 20:
		*x = 22;
		break;
	case 21:
		*x = 23;
		break;
	case 22:
		*x = 20;
		break;
	case 23:
		*x = 21;
		break;
	}
	return;
}
