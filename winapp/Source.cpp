#include "Header.h"

using namespace std;

int main() {

	if (InitializeGame() == 0) {
		//1) ������ ��������� �����
		//2) ��� ������ ���� �������������� ����� �� ������ � �������� �����
				
		char buf[20 * 40 + 200]; //����� � ������
		char c = ' ';
		while (true) {
			recv(clientSocket, &buf[0], sizeof(buf), 0); //��������� �����
			ByteToField(buf);
			if (_kbhit()) {
				c = getch();
				if (c != '\0') {

					switch (che�kKey(c))
					{
					case 1: moveHero(); break;
					case 2: Destroy(5);	break;
					case 3: Build(1); break;
					case 4: Build(2); break;
					default: break;
					}
				}
			}
			for (int i = 0; i < 20; ++i) {
				for (int j = 0; j < 39; ++j) {
					if (map[i][j] == '\0')buf[i * 39 + j] = '0';
					else buf[i * 39 + j] = map[i][j];
				}
			}
			buf[800] = 100 + HeroID; //�����
			buf[801] = HeroLabel; //�����
			buf[802] = 0; //������� ���� ������ 255
			buf[803] = posH.X; //������� ���� ������ 255
			buf[804] = 0; //������� ���� ������ 255
			buf[805] = posH.Y; //������� ���� ������ 255

			send(clientSocket, buf, sizeof(buf), 0);
		}
	}
	else return 0;
	system("pause");
	return 1;
}