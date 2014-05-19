#include <iostream>
#include <fstream>

int main()
{
	int success = 0;
	int failure = 0;
	int correct = 0;
#if 0
	//正解ピースを探す
	for (int k = 0; k < outputnum; k++){
		for (int j = 0; j < sepy; j++){
			for (int i = 0; i < sepx; i++){
				if (ans[k][i][j] == sepy*j + i)correct++;
			}
		}
	}

	//完全一致を探す
	for (int k = 0; k < outputnum; k++){
		for (int j = 0; j < sepy; j++){
			for (int i = 0; i < sepx; i++){
				if (ans[k][i][j] != sepy*j + i)goto OUT;
			}
		}
		success++;
		OUT:;
	}

	failure = outputnum - success;
	std::ofstream ofs("solusions.csv", std::ios::out | std::ios::app | std::ios::ate);
	ofs << usefile << "," << outputnum << "," << success << "," << failure << "," << correct << "," << outputnum*sepx*sepy << "," << std::endl;
	//ファイル名,出力された解答の数，完全一致した解答の数，そうでない解答の数，一致したピースの数，全体のピースの数
#endif
	std::cout << "Output solusions done." << std::endl;

    return 0;
}

