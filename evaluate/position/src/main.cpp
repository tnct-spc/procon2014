#include <iostream>
#include <fstream>

int main()
{
	int success = 0;
	int failure = 0;
	int correct = 0;
#if 0
	//�����s�[�X��T��
	for (int k = 0; k < outputnum; k++){
		for (int j = 0; j < sepy; j++){
			for (int i = 0; i < sepx; i++){
				if (ans[k][i][j] == sepy*j + i)correct++;
			}
		}
	}

	//���S��v��T��
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
	//�t�@�C����,�o�͂��ꂽ�𓚂̐��C���S��v�����𓚂̐��C�����łȂ��𓚂̐��C��v�����s�[�X�̐��C�S�̂̃s�[�X�̐�
#endif
	std::cout << "Output solusions done." << std::endl;

    return 0;
}

