#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <conio.h>
#ifdef _WIN32
#include <Windows.h>
#pragma comment (lib, "ws2_32.lib")
#elif __APPLE__
//#include <unistd.h>
#elif __linux__
//#include <unistd.h>
#elif __unix__
//#include <unistd.h>
#endif


#ifdef _WIN32
#define SetCursor(coord) SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord)
#elif __APPLE__
#elif __linux__
#elif __unix__
#endif


void MakeNewTarget();
bool chekNear();
bool chekField();
bool chekMap();
void ResInfo();
void ResArmor();
void ByteToField(char *buff);

//клиент серверное
int clientSocket;

int HeroID = 1;
char HeroLabel = 65;
int armor = 0;
int health = 100;
int resources = 0; //ресурсы игрока
char map[20][40]; //карта
COORD oldPosH; //старая позиция заблокирована (используется в ограниченном месте)
COORD posH; //позиция героя
COORD targetH; //цель
COORD posRes; //позиция ресурса
COORD coord; //временные координаты для позиции курсора 
			 //чтобы их не создавать каждый раз

//получение карты и зфайла
void getMap() {
	posH.X = 4; posH.Y = 3;
	targetH = posH;
	std::ifstream f("map.txt");
	char t = 32;
	int i = 0, j = 0;
	while (!f.eof()) {
		if (i == 20)break;
		f >> t;
		if (t == -73)
			t = 250;
		if (t == -80)
			t = -8;
		map[i][j] = t;
		j++;
		if (j == 39) { j = 0; i++; }
	}
	f.close();
}
//вывод карты на экран
void printMap() {
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 40; ++j) 		
			printf("%c", map[i][j]); 
		printf("\n");
	}
}

//проверка кнопок 
//0 - ничего не делать
//1 - идти
//2 - добывать
//3 - строить
//4 - строить под себя
//5 - удар ножем 
int cheсkKey(char c) {
	int res = 0;
	oldPosH = posH;
	switch (c) {
		case 'w': posH.Y--; if (chekMap()) { targetH = posH; targetH.Y--; res = 1; } else posH = oldPosH; break;
		case 'a': posH.X--; if (chekMap()) { targetH = posH; targetH.X--; res = 1; } else posH = oldPosH; break;
		case 'd': posH.X++; if (chekMap()) { targetH = posH; targetH.X++; res = 1; } else posH = oldPosH; break;
		case 's': posH.Y++; if (chekMap()) { targetH = posH; targetH.Y++; res = 1; } else posH = oldPosH; break;
		case 'q': if ( chekNear()) res = 2; break;
		case 'e': if (chekField()) res = 3; break;
		case 'f': if (chekField()) res = 4; break;
		//case 'r': if (chekField()) res = 5; break;
		default: res = 0;
	}
	//проверка наступания на стену
	if (res == 1 && map[posH.Y][posH.X] != -6 ) {
		if( map[posH.Y][posH.X] != -3 && map[posH.Y][posH.X] != -8)
		{ targetH = posH; posH = oldPosH; res = 0;}
	}
	//забираем drop (ресурсы оружие шмот)
	switch (map[posH.Y][posH.X])
	{
	case -3: resources += 3; map[posH.Y][posH.X] = -6; ResInfo(); break;
	case -8: if (armor < 100) { armor += 15; if (armor > 100) armor = 100;
		map[posH.Y][posH.X] = -6; ResArmor(); } break; //test ввести в карту
	default:break;
	}
	
	return res;
}
//установка курсора на позицию  x y 
inline void setcur(int x, int y) {	
	coord.X = x; coord.Y = y;
	SetCursor(coord);
};

//краткая функция для передвижения героя
inline void moveHero() {
	setcur(posH.X, posH.Y);
	printf("%c", HeroLabel);
	setcur(oldPosH.X, oldPosH.Y);
	printf("%c", map[oldPosH.Y][oldPosH.X]);
	setcur(0, 21);
}

//првоерка на наличие рядом разрушаемого окружения
inline bool chekNear() {
	//сначала проверяем цель (куда хотели пойти но не смогли
	if (map[targetH.Y][targetH.X] != -6 && map[targetH.Y][targetH.X] != -3) { posRes.X = targetH.X; posRes.Y = targetH.Y; return true; }
	//потом окружение
	else if (map[posH.Y - 1][posH.X] != -6 && map[targetH.Y][targetH.X] != -3) { posRes.X = posH.X; posRes.Y = posH.Y - 1; return true; }
	else if (map[posH.Y][posH.X + 1] != -6 && map[targetH.Y][targetH.X] != -3) { posRes.X = posH.X + 1; posRes.Y = posH.Y; return true; }
	else if (map[posH.Y + 1][posH.X] != -6 && map[targetH.Y][targetH.X] != -3) { posRes.X = posH.X; posRes.Y = posH.Y + 1; return true; }
	else if (map[posH.Y][posH.X - 1] != -6 && map[targetH.Y][targetH.X] != -3) { posRes.X = posH.X - 1; posRes.Y = posH.Y; return true; }
	else return false;
}
//проверка пустого поля чтобы построить стену
inline bool chekField() {
	//проверяем цель на пустоту
	if (map[targetH.Y][targetH.X] == -6) {
		return true; 	
	}
	else return false;
}
//проверка выхода за карту
inline bool chekMap() {
	if (posH.X < 40 && posH.X >0 && posH.Y < 20 && posH.Y >0) return true;
	else return false;
}

int InitializeGame() {
	//инициализация игры и соединения с сервером
	//getMap();

	char buff[1024];
	if (WSAStartup(0x0202, (WSADATA *)&buff[0])) {
		printf("Error WSAStartup %d\n", WSAGetLastError());
		return 1;
	}

	//создание сокета на клиентской стороне	
	if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0))<0)
	{
		printf("Error Socket%d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	//информация о сервере
	struct sockaddr_in serv;
	serv.sin_family = AF_INET;
	serv.sin_port = htons(666);
	serv.sin_addr.s_addr = inet_addr("127.0.0.1");

	//проверка подключения
	if (connect(clientSocket, (struct sockaddr *) & serv, sizeof(serv)) < 0)
		return 5;

	char buf[20 * 40 + 200];
	recv(clientSocket, &buf[0], sizeof(buf), 0); //принимаем карту
	ByteToField(buf);
	
	int t = (int)((unsigned char)buf[800]) - 100;

	HeroID = (int)((unsigned char)buf[801 + (t - 1) * 6]) - 100;
	HeroLabel = (int)((unsigned char)buf[802 + (t - 1) * 6]);
	posH.X = (int)(buf[803 + (t - 1) * 6]) * 255
		+ (int)(buf[804 + (t - 1) * 6]);
	posH.Y = (int)(buf[805 + (t - 1) * 6]) * 255
		+ (int)(buf[806 + (t - 1) * 6]);

	targetH = posH;
	//задание позиции героя
	setcur(posH.X, posH.Y);
	printf("%c", HeroLabel);
	setcur(0, 21);
	ResInfo();
	ResArmor();


	return 0;
}
//рушение округи
inline void Destroy(int t) {
	setcur(3, 21);
	for (int i = 0; i < t; ++i) {
		printf("%s", ">>>");
		_sleep(100);
	}
	map[posRes.Y][posRes.X] = -3;//-6; //-3 ресурс
	setcur(posRes.X, posRes.Y);
	printf("%c", -3); //250
	setcur(3, 21);
	for (int i = 0; i < t*3; ++i) {
		printf("%c", ' ');
	}
	setcur(0, 21);
}
//создание стен 
inline void Build(int b) {
	if (resources >= 2) {
		resources -= 2;
		ResInfo();
		switch (b)
		{
		case 1:
			map[targetH.Y][targetH.X] = 219;
			setcur(targetH.X, targetH.Y);
			printf("%c", 219);
			setcur(0, 21);
			break;
		case 2:		
			map[posH.Y][posH.X] = 219;			
			setcur(posH.X, posH.Y);
			printf("%c", 219);
			setcur(targetH.X, targetH.Y);
			printf("%c", HeroLabel);
			MakeNewTarget();
			setcur(0, 21);
			break;
		default:
			break;
		}
	}
}
//for Build(2)
inline void MakeNewTarget() {
	if (posH.X == targetH.X) {
		if (posH.Y < targetH.Y) { posH = targetH; targetH.Y++; }
		else { posH = targetH; targetH.Y--; }
	}
	else {
		if (posH.X < targetH.X) { posH = targetH; targetH.X++; }
		else { posH = targetH; targetH.X--; }
	}
}
//сведения о ресурсах
inline void ResInfo() {
	setcur(57, 1); printf("%c", ' '); printf("%c", ' '); printf("%c", ' ');
	setcur(45, 1);
	printf("%s%d", "Resuorces: ", resources);
	setcur(0, 21);
}
//сведения о броне
inline void ResArmor() {
	setcur(53, 1); printf("%c", ' '); printf("%c", ' '); printf("%c", ' ');
	setcur(45, 2);
	printf("%s%d", "Armor: ", armor);
	setcur(0, 21);
}
//преобразование игрового поля в массив
inline char* fieldToChar(int wid, int heig) {
	char* buf = new char[wid * heig];
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 39; ++j) {
			if (map[i][j] == '\0')buf[i * 39 + j] = '0';
			else buf[i * 39 + j] = map[i][j];
		}
	}
	return buf;
}
//преобразование массива в игровое поле 
inline void ByteToField(char *buff) {
	char tmp[20][40];
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 39; ++j) {
			if (buff[i * 39 + j] == '0')tmp[i][j] = '\0';
			else tmp[i][j] = buff[i * 39 + j];
		}
	}
	for (int i = 0; i < 20; ++i) {
		for (int j = 0; j < 39; ++j) {
			if (map[i][j] != tmp[i][j]) {
				setcur(j, i);
				printf("%c", tmp[i][j]);
				map[i][j] = tmp[i][j];
			}
			else map[i][j] = tmp[i][j];
		}
	}
	int t = (int)((unsigned char)buff[800]) - 100;
	/*for (int i = 0; i < t; ++i) {
		if(buff[801 + (t - 1) * 6]!=HeroID)
	}*/
}
