#include "Header.h"

using namespace std;

int main() {

	if (InitializeGame() == 0) {
		//1) сервер присылает карту
		//2) при каждом шаге переотправляем карту на сервер и получаем новую
				
		char buf[20 * 40 + 200]; //буфер с картой
		char c = ' ';
		while (true) {
			recv(clientSocket, &buf[0], sizeof(buf), 0); //принимаем карту
			ByteToField(buf);
			if (_kbhit()) {
				c = getch();
				if (c != '\0') {

					switch (cheсkKey(c))
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
			buf[800] = 100 + HeroID; //номер
			buf[801] = HeroLabel; //буква
			buf[802] = 0; //позиция если больше 255
			buf[803] = posH.X; //позиция если меньше 255
			buf[804] = 0; //позиция если больше 255
			buf[805] = posH.Y; //позиция если меньше 255

			send(clientSocket, buf, sizeof(buf), 0);
		}
	}
	else return 0;
	system("pause");
	return 1;
}