﻿////////////////////////////////////////////
/////A* Algorithm (最優先探索アルゴリズム)///////
////////////////////////////////////////////
//#define debug//★

#ifdef _DEBUG
#include <boost/timer/timer.hpp>
#endif

#include "algorithm_2.hpp"
#include <iostream>

//コンストラクタ
heap::heap(){
}
//デストラクタ
heap::~heap(){
}

void heap::setup(const int *in_y, const int *in_x){
	y = *in_y;
	x = *in_x;
	yx = y*x;
	//配列のリサイズ☆おおよそ
	cost.resize(1000000);
	history.resize(1000000);
	history_limit.resize(1000000);
	table.resize(1000000, std::vector<int>(yx));
	heaptable.resize(1000000);
	LIST_OC.resize(1000000);
}
void heap::pop(int *in_cost, std::vector<int> &in_table, std::vector<int> &in_history, int *in_history_limit){

	int i;
	int me;
	int top = pos;

	if (size > 1000000 * sizemaxcount - 1){
		sizemaxcount++;
		cost.resize(1000000 * sizemaxcount);
		history.resize(1000000 * sizemaxcount);
		history_limit.resize(1000000 * sizemaxcount);
		table.resize(1000000 * sizemaxcount, std::vector<int>(yx));
		heaptable.resize(1000000 * sizemaxcount);
		LIST_OC.resize(1000000 * sizemaxcount);
#ifdef _DEBUG
		std::cout << ">< heap.cpp vector pass1,000,000" << std::endl;
#endif
	}
	//重複確認2
	bool ok = true;
	int sizepos;
	auto match_pos = NODE_.find(NODE{in_table,0});//TEの第二引数は関係ない
	if (match_pos != NODE_.end()){ //テーブルが一致した
		sizepos = match_pos->pos;
		ok = false;
		if (*in_cost < cost[sizepos]){
			//このノードがiのテーブルよりコストが低い
			//ラベルつけてあとで消したほうがいいかも？←その必要はなさそう
			if (LIST_OC[sizepos] == true){
				//このノードと同じテーブルがOpenリストに含まれている
				//◆書き換え
				cost[sizepos] = *in_cost;
				history_limit[sizepos] = *in_history_limit;
				history[sizepos].resize(history_limit[sizepos]);
				for (int m = 0; m < history_limit[sizepos]; m++){
					history[sizepos][m] = in_history[m];
				}
			}
			else{
				//このノードと同じテーブルがCloseリストに含まれている
				//◆書き換え
				LIST_OC[sizepos] = true;
				cost[sizepos] = *in_cost;
				history_limit[sizepos] = *in_history_limit;
				history[sizepos].resize(history_limit[sizepos]);
				for (int m = 0; m < history_limit[sizepos]; m++){
					history[sizepos][m] = in_history[m];
				}
			}
		}
	}
	if (ok == true){
		//コスト挿入
		cost[size] = *in_cost;

		//履歴挿入
		history[size].resize(*in_history_limit);
		for (i = 0; i < *in_history_limit; i++){
			history[size][i] = in_history[i];
		}
		history_limit[size] = *in_history_limit;

		//テーブル挿入
		for (i = 0; i < yx; i++){
			table[size][i] = in_table[i];
		}

		//NODEに挿入
		NODE_.insert(NODE(table[size], size));

		//OPEN_LIST挿入
		LIST_OC[size] = true;

		//ヒープ配列挿入
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
}

void heap::push(int *out_cost, std::vector<int> &out_table, std::vector<int> &out_history, int *out_history_limit){
	const int out = heaptable[0];
	int i = 0;
	int me = 0;
	int bottom = 0;

	pos--;

	//OPENからCLOSE_LISTに移動
	LIST_OC[out] = false;

	//コスト挿入
	*out_cost = cost[out];

	//履歴挿入
	i = 0;
	*out_history_limit = history_limit[out];
	while (i < *out_history_limit){
		out_history[i] = history[out][i];
		i++;
	}

	//テーブル挿入
	for (i = 0; i < yx; i++){
		out_table[i] = table[out][i];
	}

	//ヒープ配列挿入
	while (1){
		me = bottom;
		bottom = (bottom * 2);
		if (pos>bottom + 2){
			if (cost[heaptable[bottom + 1]] < cost[heaptable[bottom + 2]]){
				if (cost[heaptable[pos]] > cost[heaptable[bottom + 1]]){
					bottom += 1;
					heaptable[me] = heaptable[bottom];
				}
				else{
					break;
				}
			}
			else{
				if (cost[heaptable[pos]] > cost[heaptable[bottom + 2]]){
					bottom += 2;
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

	heaptable[me] = heaptable[pos];
	heaptable.erase(heaptable.end() - 1);
}

void heap::end(){
	std::vector<int>().swap(cost);
	std::vector<int>().swap(heaptable);
	std::vector<bool>().swap(LIST_OC);
	std::vector<int>().swap(cost);
	std::vector<std::vector<int>>().swap(table);
	std::vector<std::vector<int>>().swap(history);
	std::vector<int>().swap(history_limit);
}
//コンストラクタ
algorithm_2::algorithm_2()
{
}

//デストラクタ
algorithm_2::~algorithm_2()
{
}

//初期化
void algorithm_2::reset(question_data const& data)
{
	// データのクローン
	data_ = data.clone();

	// 幅と高さ
	size_y = data_->size.second;
	size_x = data_->size.first;
	size = size_y * size_x;

	// コストとレート
	cost_s = data_->cost_select;
	cost_c = data_->cost_change;

	// テーブル
	//tableによそから貰ってきたマトリクスを整理して挿入
	table.resize(size);
	for (int y = 0; size_y > y; y++){
		for (int x = 0; size_x > x; x++){
			table[y * size_x + x] = data_->block[y][x].y * size_x + data_->block[y][x].x;
		}
	}
	//初期化
	harray.setup(&size_y, &size_x);


	history.resize(10000);
	keiro.resize(1024);
	sub_history.resize(10000);
	root1.resize(1024);
	root2.resize(1024);
#ifdef _DEBUG_incost
	std::cout << "\ngoal=";
	std::cin >> goal;
	std::cout << "sentaku=";
	std::cin >> sentaku;
	std::cout << "coukan=";
	std::cin >> coukan;
#endif
}

auto algorithm_2::get() -> boost::optional<return_type>
{
	int i, count;
	std::cout << "algorythm_2 start" << std::endl;
	std::cout << "size_y=" << size_y << ",size_x=" << size_x << std::endl;
#ifdef _DEBUG
	for (int y = 0; size_y > y; y++){
		for (int x = 0; size_x > x; x++){
			std::cout << std::setw(3) << table[y * size_x + x] << ",";
		}
		std::cout << std::endl;
	}
	//パズルスタート
	std::cout << "start" << std::endl;
	boost::timer::cpu_timer timer; // 時間計測を開始
#endif

	while (1){
		//探索
		algorithm_2::prescanning();
		//取得
		harray.push(&cost, table, history, &history_limit);
		//完成判定
		count = 0;
		for (i = 0; i<size; i++){
			if (table[i] == i) count++;
		}
		if (count == size){
			std::cout << "algorithm_2 finish" << std::endl;
			for (int i = 0; i < history_limit; i++){
				switch (history[i]){
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
					std::cout << "," << history[i];
					break;
				}
			}
			std::cout << std::endl;
			int ANSWER_S = 0, ANSWER_C = 0;
			for (int i = 0; i < history_limit; i++){
				if (history[i] < 16){
					ANSWER_S += 1;
				}
				else{
					ANSWER_C += 1;
				}
			}
			ANSWER_S /= 2;
			std::cout << "選択コスト=" << cost_s << ",交換コスト=" << cost_c << std::endl;
			std::cout << "cost=" << ANSWER_S * cost_s + ANSWER_C * cost_c << " S: " << ANSWER_S << " C: " << ANSWER_C << std::endl;
#ifdef _DEBUG
			//完成☆後で消したりなんたり
			std::string result = timer.format();
			std::cout << "end" << std::endl;
			std::cout << "処理時間:" << result << std::endl;
			for (int i = 0; i < history_limit; i++){
				switch (history[i]){
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
					std::cout << "," << history[i];
					break;
				}
			}
			std::cout << std::endl;
			int _DEBUG_S = 0, _DEBUG_C = 0;
			for (int i = 0; i < history_limit; i++){
				if (history[i] < 16){
					_DEBUG_S += 1;
				}
				else{
					_DEBUG_C += 1;
				}
			}
			_DEBUG_S /= 2;
			std::cout << "選択コスト=" << cost_s << ",交換コスト=" << cost_c << std::endl;
			std::cout << "cost=" << _DEBUG_S * cost_s + _DEBUG_C * cost_c << " S: " << _DEBUG_S << " C: " << _DEBUG_C << std::endl;
#endif
			harray.end();
			//解をanswer_typeにして返す
			answer_type answerlist;
			point_type position;
			std::ostringstream stream;
			int pos = 0;
			for (int count = 0; pos < history_limit; count++){
				position.y = history[pos];
				pos++;
				position.x = history[pos];
				pos++;
				while (pos < history_limit && 16 <= history[pos]){
					switch (history[pos]){
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
				answerlist.list.push_back({ position, stream.str() });
				stream.str("");
			}
			return answerlist;
		}
	}
}

//走査準備(全てのマスを一つづつscaningで調べる)
void algorithm_2::prescanning(){
	for (int y = 0; y < size_y; y++){
		for (int x = 0; x < size_x; x++){
			keiro[0] = y;
			keiro[1] = x;
			keiro_count = 2;
			scanning(y, x, y, x, -1);
		}
	}
}

//走査
void algorithm_2::scanning(int y, int x, int y_before, int x_before, int URDL){
	int i, buff;
	bool feel, feeled;

	//最初だけ上下左右を通りあとは再帰的に来て保存する（ちょっと無駄だけど直す時間がなかった）

	if ((y == y_before && x == x_before)){
		//進む(上右下左の順)
		//上
		if (y > 0 && URDL != 2){
			if (table[y * size_x + x] / size_x < y){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[(y - 1) * size_x + x] / size_x > y - 1)
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
				//交換
				buff = table[y * size_x + x];
				table[y * size_x + x] = table[(y - 1) * size_x + x];
				table[(y - 1) * size_x + x] = buff;
				scanning(y - 1, x, y, x, 0);
			}
		}
		//右
		if (x < size_x - 1 && URDL != 3){
			if (table[y * size_x + x] % size_x > x){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[y * size_x + (x + 1)] % size_x < x + 1)
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
				//交換
				buff = table[y * size_x + x];
				table[y * size_x + x] = table[y * size_x + (x + 1)];
				table[y * size_x + (x + 1)] = buff;
				scanning(y, x + 1, y, x, 1);
			}
		}
		//下
		if (y < size_y - 1 && URDL != 0){
			if (table[y * size_x + x] / size_x > y){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[(y + 1) * size_x + x] / size_x < y + 1)
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
				//交換
				buff = table[y * size_x + x];
				table[y * size_x + x] = table[(y + 1) * size_x + x];
				table[(y + 1) * size_x + x] = buff;
				scanning(y + 1, x, y, x, 2);
			}
		}
		//左
		if (x > 0 && URDL != 1){
			if (table[y * size_x + x] % size_x < x){
				feel = true;
			}
			else{
				feel = false;
			}
			if (table[y * size_x + (x - 1)] % size_x > x - 1)
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
				//交換
				buff = table[y * size_x + x];
				table[y * size_x + x] = table[y * size_x + (x - 1)];
				table[y * size_x + (x - 1)] = buff;
				scanning(y, x - 1, y, x, 3);
			}
		}
	}
	if ((y != y_before || x != x_before)) //一番最初以外
	{
		if (history_limit > 10000 * sizemaxhistory - 2){
			sizemaxhistory++;
			history.resize(10000 * sizemaxhistory);
			sub_history.resize(10000 * sizemaxhistory);
#ifdef _DEBUG
			std::cout << ">< search.cpp history vector pass10,000" << std::endl;
#endif
		}
		//パターン配列(経路)に保存
		for (i = 0; i < keiro_count; i++){
			history[history_limit + i] = keiro[i];
		}
		history_limit += keiro_count;
		//■■■■shorting■■■■
		sub_history_limit = history_limit;
		for (i = 0; i < sub_history_limit; i++){
			sub_history[i] = history[i];
		}
		shorting();
		//■■■■■■■■■■■■
		//cost = 選択コスト+交換コスト+ゴールまでの距離;
		G = 0;
		S = 0;
		C = 0;
		//ゴールまでの距離
		for (i = 0; i < size; i++){
			G += abs(table[i] / size_x - i / size_x) + abs(table[i] % size_x - i % size_x);
		}
		//交換コストと選択コストも入れる
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

#ifdef test2
		//@@@重要：：ここやらなきゃ←やらなくてもいいかも
		//最大選択回数より大きいならやめる
		if (S / 15 < 3){
			harray.pop(&cost, table, subhistory, &subhistory_limit);
		}
#else
		//登録
		harray.pop(&cost, table, sub_history, &sub_history_limit);
#endif
		history_limit -= keiro_count;
	}
	//全部見終わったらtableを元通りにして返す
	buff = table[y * size_x + x];
	table[y * size_x + x] = table[y_before * size_x + x_before];
	table[y_before * size_x + x_before] = buff;

	//うち経路消すん
	keiro[keiro_count] = 0;
	if (keiro_count>2) keiro_count -= 1;

	y = y_before;
	x = x_before;
	//最初の階層に上がる
}
void algorithm_2::shorting(){
	//経路をまとめて短縮する
	int i;
	int x1_head, y1_head, root1_count, y1_tail, x1_tail;
	int x2_head, y2_head, root2_count, y2_tail, x2_tail;
	int start, count = 0, end;

	while (count + 5<sub_history_limit){

		root1_count = 0;
		root2_count = 0;
		start = count;
		y1_head = sub_history[count];
		count++;
		x1_head = sub_history[count];
		count++;
		while (1){
			if (!(count<sub_history_limit && sub_history[count] >= 16 && sub_history[count] <= 23)) break;
			if (root1_count > 1024 * sizemaxroot1 - 1){
				sizemaxroot1++;
				root1.resize(1024 * sizemaxroot1);
#ifdef _DEBUG
				std::cout << ">< search.cpp root1 vector pass1,024" << std::endl;
#endif
			}
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
			if (root2_count > 1024 * sizemaxroot2 - 1){
				sizemaxroot2++;
				root2.resize(1024 * sizemaxroot2);
#ifdef _DEBUG
				std::cout << ">< search.cpp root2 vector pass10,000" << std::endl;
#endif
			}
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
			//root2の各方向を反転
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
			//root1の各方向を反転
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
			//root1の各方向を反転
			reverse(&root1[0]);
			//root2の各方向を反転
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
