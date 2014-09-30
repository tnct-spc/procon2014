//////////////////////////
//最優先探索アルゴリズム//
//////////////////////////

//インクルードファイル
#include <atltime.h>//時間計測

#include <vector>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/coroutine/coroutine.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include "algorithm_2.hpp"

#define debug
//コンストラクタ
heap::heap(){
}
//デストラクタ
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

	//コスト挿入
	cost[size] = *in_cost;
	//履歴挿入
	i = 0;
	history[size].resize(*in_history_limit);
	while (i < *in_history_limit){
		history[size][i] = (in_history[i]);
		i++;
	}
	history_limit[size] = *in_history_limit;
	//テーブル挿入
	table[size].resize(y*x);
	for (i = 0; i < y*x; i++){
		table[size][i] = (in_table[i]);
	}
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

void heap::push(int *out_cost, int *out_table, std::vector<int> &out_history, int *out_history_limit){
	long long int out = heaptable[0];

	long long int i = 0;
	long long int me = 0;
	long long int bottom = 0;

	pos--;
	size--;
	//コスト挿入
	*out_cost = cost[heaptable[0]];
	//履歴挿入
	i = 0;
	*out_history_limit = history_limit[heaptable[0]];
	while (i < *out_history_limit){
		out_history[i] = history[heaptable[0]][i];
		i++;
	}
	//テーsブル挿入
	for (i = 0; i < table[heaptable[0]].size(); i++){
		out_table[i] = table[heaptable[0]][i];
	}
	//std::cout << "num=" << heaptable[0] << std::endl;
	//std::cout << "pos="<<pos<<" size="<<size << std::endl;
	//ヒープ配列挿入
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




//コンストラクタ
algorithm_2::algorithm_2()
{
}

//デストラクタ
algorithm_2::~algorithm_2()
{
	delete[] table;
	delete[] agotable;
}

auto algorithm_2::get() -> boost::optional<return_type>
{
	std::cout << "algorythm_2 start" << std::endl;
	// 画像
	matrix = data_->block;

	// 幅と高さ
	width = data_->size.first;
	height = data_->size.second;

	// コストとレート
	cost_select = data_->cost_select;
	cost_change = data_->cost_change;
	//初期化(後でコンストラクタにでも移す)
	int i;

	ysize = height;
	xsize = width;
	size = ysize*xsize;
	table = new int[size];//y*xサイズのテーブルを作る
	//tableによそから貰ってきたmatrixを整理して挿入
	for (int y = 0; height > y; y++){
		for (int x = 0; width > x; x++){
			table[y*width + x] = matrix[y][x].y*width + matrix[y][x].x;
		}
	}
	agotable = new int[size];
	harray.setup(ysize, xsize);
	for (i = 0; i < size; i++){
		agotable[i] = table[i];
	}
	history.resize(1000000);
	subhistory.resize(1000000);
	keiro.resize(1000000);
	history_limit = 0;
	subhistory_limit = 0;
	//debug表示
	/*
	std::cout << "width=" << width << ",height=" << height << std::endl;
	std::cout << "width=" << width << ",height=" << height << std::endl;
	for (int y = 0; height > y; y++){
		for (int x = 0; width > x; x++){
			std::cout << "(" << matrix[y][x].y << "," << matrix[y][x].x << ")";
		}
		std::cout<<std::endl;
	}
	for (int y = 0; height > y; y++){
		for (int x = 0; width > x; x++){
			std::cout << table[y*width + x] << ",";
		}
		std::cout << std::endl;
	}
	*/


	CFileTime cTimeStart, cTimeEnd;
	CFileTimeSpan cTimeSpan;
	cTimeStart = CFileTime::GetCurrentTime();
	std::vector<int> g_history(0);
	int g_cost = 0;
	int g_history_limit = 0;

	//パズルスタート
	std::cout << "start" << std::endl;
	g_history_limit = puzzle(g_history, g_cost);
	std::cout << "end" << std::endl;
	cTimeEnd = CFileTime::GetCurrentTime();
	cTimeSpan = cTimeEnd - cTimeStart;
	std::cout << "\n処理時間:" << cTimeSpan.GetTimeSpan() / 10000 << "[ms]" << std::endl;
	for (i = 0; i < g_history_limit; i++){
		switch (g_history[i]){
			case 16:
			case 20:
				std::cout << ",上";
				break;
			case 17:
			case 21:
				std::cout << ",右";
				break;
			case 18:
			case 22:
				std::cout << ",下";
				break;
			case 19:
			case 23:
				std::cout << ",左";
				break;
			default:
				std::cout << "," << g_history[i];
				break;
		}
	}
	int g_S = 0, g_C = 0;
	for (i = 0; i < g_history_limit; i++){
		if (g_history[i] < 16){
			g_S += 1;
		}
		else{
			g_C += 1;
		}
	}
	g_S /= 2;
	std::cout << "\ncost=" << g_S + g_C << " S: " << g_S / 1 << " C: " << g_C / 1 << std::endl;


	//stop
	int stdebug;
	std::cin >> stdebug;
	return 0;
	//return pimpl_->get();
}

void algorithm_2::reset(question_data const& data)
{
	data_ = data.clone();
	//pimpl_->reset(data);
}



//探索
int algorithm_2::puzzle(std::vector<int> &out_history, int &out_cost){
	int i, count, buff;


	while (1){
		//search
		debug_c++;
		//std::cout << "searchstart" << debug_c << std::endl;
		algorithm_2::prescanning();
		//std::cout << "searchfinish" << std::endl;
		//push
		//★★★試験的な実験★★★
		//連続で来たおんなじのは飛ばす
		while (1){
			harray.push(&cost, table, history, &history_limit);
			buff = 0;
			for (i = 0; i < size; i++){
				if (agotable[i] == table[i])buff++;
			}
			for (i = 0; i < size; i++){
				agotable[i] = table[i];
			}
			if (buff != size){
				//前回と違うマス位置
				break;
			}
		}
		//★★★★ここまで★★★★
		/*
		std::cout << "cost: " << cost << std::endl;
		std::cout << "table: ";
		for (i = 0; i < size; i++){
		std::cout << "," << table[i];
		}
		std::cout << std::endl << "history: ";
		for (i = 0; i < history_limit; i++){
		std::cout << "," << history[i];
		}
		std::cout << std::endl;
		std::cin >> i;
		*/
		//■完成判定■
		count = 0;
		for (i = 0; i < size; i++){
			if (table[i] == i) count++;
		}
		if (count == size){
			out_cost = cost;
			out_history.resize(history_limit);
			for (int i = 0; i < history_limit; i++){
				out_history[i] = history[i];
			}
			return history_limit;
		}
	}
}

//走査準備(全てのマスから調べる)
void algorithm_2::prescanning(){
	for (int y = 0; y < ysize; y++){
		for (int x = 0; x < xsize; x++){
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
	bool bbuff, bbuff2;
	//コスト
	int G, S, C;
	/*
	for (i = 0; i<keiro_count; i++){
	std::cout << keiro[i] << ",";
	}
	std::cout << "x=" << x << " y=" << y;
	std::cout << std::endl;
	for (int j = 0; j<ysize; j++){
	for (int k = 0; k<xsize; k++){
	if (j == y&&k == x){
	std::cout << "■■";
	}
	else{
	printf("%4d", table[ysize*j + k]);
	}
	}
	std::cout << std::endl;
	}
	std::cout << std::endl;
	std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl; std::cout << std::endl;
	//std::cin >> i;
	_sleep(30);
	*/
	if (keiro_count > 1000000 * size_million_keiro){
#ifdef debug
		std::cout << "><search.cpp keiro vector pass1,000,000" << std::endl;
#endif
		size_million_keiro++;
		history.resize(1000000 * size_million_keiro);
		subhistory.resize(1000000 * size_million_keiro);
	}
#ifdef nosaiki
	if ((y == y_before && x == x_before)){
#endif
		//進む(上右下左の順)
		//上
		if (y > 0 && URDL != 2){
			if (table[y * ysize + x] / ysize < y)			{ bbuff = true; }
			else{ bbuff = false; }
			if (table[(y - 1) * ysize + x] / ysize > y - 1) { bbuff2 = true; }
			else{ bbuff2 = false; }
			if (bbuff == true/* || bbuff2==true*/){
				//if (/*bbuff == true || */bbuff2==true){
				if (bbuff == true && bbuff2 == true){
					keiro[keiro_count] = 16;
				}
				else {
					keiro[keiro_count] = 20;
				}
				keiro_count += 1;
				//交換
				buff = table[y * ysize + x];
				table[y * ysize + x] = table[(y - 1) * ysize + x];
				table[(y - 1) * ysize + x] = buff;
				scanning(y - 1, x, y, x, 0);
			}
		}
		//右
		if (x<xsize - 1 && URDL != 3){
			if (table[y * ysize + x] % xsize > x)			{ bbuff = true; }
			else{ bbuff = false; }
			if (table[y * ysize + (x + 1)] % xsize < x + 1) { bbuff2 = true; }
			else{ bbuff2 = false; }
			if (bbuff == true/* || bbuff2 == true*/){
				//if (/*bbuff == true || */bbuff2 == true){
				if (bbuff == true && bbuff2 == true){
					keiro[keiro_count] = 17;
				}
				else{
					keiro[keiro_count] = 21;
				}
				keiro_count += 1;
				//交換
				buff = table[y * ysize + x];
				table[y * ysize + x] = table[y * ysize + (x + 1)];
				table[y * ysize + (x + 1)] = buff;
				scanning(y, x + 1, y, x, 1);
			}
		}
		//下
		if (y<ysize - 1 && URDL != 0){
			if (table[y * ysize + x] / ysize > y)			{ bbuff = true; }
			else{ bbuff = false; }
			if (table[(y + 1) * ysize + x] / ysize < y + 1) { bbuff2 = true; }
			else{ bbuff2 = false; }
			if (bbuff == true/* || bbuff2 == true*/){
				//if (/*bbuff == true || */bbuff2 == true){
				if (bbuff == true && bbuff2 == true){
					keiro[keiro_count] = 18;
				}
				else{
					keiro[keiro_count] = 22;
				}
				keiro_count += 1;
				//交換
				buff = table[y * ysize + x];
				table[y * ysize + x] = table[(y + 1) * ysize + x];
				table[(y + 1) * ysize + x] = buff;
				scanning(y + 1, x, y, x, 2);
			}
		}
		//左
		if (x > 0 && URDL != 1){
			if (table[y * ysize + x] % xsize < x)			{ bbuff = true; }
			else{ bbuff = false; }
			if (table[y * ysize + (x - 1)] % xsize > x - 1) { bbuff2 = true; }
			else{ bbuff2 = false; }
			if (bbuff == true/* || bbuff2 == true*/){
				//if (/*bbuff == true || */bbuff2 == true){
				if (bbuff == true && bbuff2 == true){
					keiro[keiro_count] = 19;
				}
				else{
					keiro[keiro_count] = 23;
				}
				keiro_count += 1;
				//交換
				buff = table[y * ysize + x];
				table[y * ysize + x] = table[y * ysize + (x - 1)];
				table[y * ysize + (x - 1)] = buff;
				scanning(y, x - 1, y, x, 3);
			}
		}
#ifdef nosaiki
	}
#endif
	if ((y != y_before || x != x_before)){//一番最初以外/* && ◆◆上位互換が存在しない(未実装)◆◆*/
		if (history_limit > 1000000 * size_million_history){
#ifdef debug
			std::cout << "><search.cpp history vector pass1,000" << std::endl;
#endif
			size_million_history++;
			history.resize(1000000 * size_million_history);
			subhistory.resize(1000000 * size_million_history);
		}

		//パターン配列(経路)に保存
		for (i = 0; i < keiro_count; i++){
			history[history_limit + i] = keiro[i];
		}
		history_limit += keiro_count;
		//■■■■shorting■■■■
		subhistory_limit = history_limit;
		for (i = 0; i < subhistory_limit; i++){
			subhistory[i] = history[i];
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
		//■■■■■■■■■■■■
		//cost = 選択コスト+交換コスト+ゴールまでの距離;
		G = 0;
		S = 0;
		C = 0;
		//取り敢えずゴールまでの距離を入れておく
		for (i = 0; i < size; i++){
			G += abs(table[i] / ysize - i / ysize) + abs(table[i] % xsize - i % xsize);
		}
		//交換コストと選択コストも入れる
		for (i = 0; i < subhistory_limit; i++){
			if (subhistory[i] < 16){
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

		//1,1,1		ノーマル
		//10,5,3	選択が短いのを作れた
		//10 8 4
		//10 14 6
		harray.pop(&cost, table, subhistory, &subhistory_limit);
		/*
		//★最大選択回数より大きいならやめる
		if (S / 15 < 3){
		harray.pop(&cost, table, subhistory, &subhistory_limit);
		}
		*/
		history_limit -= keiro_count;
	}
	//全部見終わったら、dataを元通りにして返す(?)
	buff = table[y * ysize + x];
	table[y * ysize + x] = table[y_before * ysize + x_before];
	table[y_before * ysize + x_before] = buff;

	//うち経路消すん
	keiro[keiro_count] = 0;
	if (keiro_count>2) keiro_count -= 1;

	y = y_before;
	x = x_before;
	//up
}
void algorithm_2::shorting(){
	int i;
	//経路をまとめて短縮する
	int x1_head, y1_head, root1_count, y1_tail, x1_tail; std::vector<int> root1;
	int x2_head, y2_head, root2_count, y2_tail, x2_tail; std::vector<int> root2;
	int start, count = 0, end;
	root1.resize(256);//このくらいでいいかな
	root2.resize(256);
	/*
	std::cout << "keiro " << std::endl;

	for (int g = 0; g < 64; g++){
	std::cout << search_pattern_keiro[kaisou][count][g] << ",";
	}

	std::cout << "@" << std::endl;
	*/
	while (count + 3<subhistory_limit){
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
		y1_head = subhistory[count];
		count++;
		x1_head = subhistory[count];
		count++;
		while (1){
			if (!(count<subhistory_limit && subhistory[count] >= 16 && subhistory[count] <= 23)) break;//(-1または0からsize)以外
			root1[root1_count] = subhistory[count];
			root1_count++;
			count++;
		}
		if (!(count<subhistory_limit)){
			break;
		}
		end = count;
		y2_head = subhistory[end];
		end++;
		x2_head = subhistory[end];
		end++;
		while (1){
			if (!(end < subhistory_limit && subhistory[end] >= 16 && subhistory[end] <= 23)) break;
			root2[root2_count] = subhistory[end];
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
				subhistory[count] = root2[i];
				count++;
			}
			for (i = 0; i + end < subhistory_limit; i++){
				subhistory[count + i] = subhistory[end + i];
			}
			subhistory_limit = count + i;
			count = 0;
		}
		//Rule2
		else if (y1_tail == y2_tail && x1_tail == x2_tail && root2_count == 1){
			//std::cout << "rulu2" << std::endl;
			//root2の各方向を反転
			reverse(&root2[0]);
			subhistory[count] = root2[0];
			count++;
			for (i = 0; i + end < subhistory_limit; i++){
				subhistory[count + i] = subhistory[end + i];
			}
			subhistory_limit = count + i;
			count = 0;
		}
		//Rule3
		else if (y1_head == y2_head && x1_head == x2_head && root1_count == 1){
			//std::cout << "rulu3" << std::endl;
			//root1の各方向を反転
			reverse(&root1[0]);
			subhistory[start] = y1_tail;
			start++;
			subhistory[start] = x1_tail;
			start++;
			subhistory[start] = root1[0];
			start++;
			for (i = 0; i < root2_count; i++){
				subhistory[start] = root2[i];
				start++;
			}
			for (i = 0; i + end < subhistory_limit; i++){
				subhistory[start + i] = subhistory[end + i];
			}
			subhistory_limit = start + i;
			count = 0;
		}
		//Rule4
		else if (y1_head == y2_tail && x1_head == x2_tail && root1_count == 1 && root2_count == 1){
			//std::cout << "rulu4" << std::endl;
			//root1の各方向を反転
			reverse(&root1[0]);
			//root2の各方向を反転
			reverse(&root2[0]);
			subhistory[start] = y1_tail;
			subhistory[start + 1] = x1_tail;
			subhistory[start + 2] = root1[0];
			subhistory[start + 3] = root2[0];
			for (i = 0; i + end < subhistory_limit; i++){
				subhistory[start + 4 + i] = subhistory[end + i];
			}
			subhistory_limit = start + 4 + i;
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
