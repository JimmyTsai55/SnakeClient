#define NOMINMAX
#include <stdio.h>
#include <WinSock2.h>
#include <Windows.h>  
#include <ws2tcpip.h>
#include <SFML/Graphics.hpp>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
void sendScoreToServer(SOCKET sSocket, int score);
void sendisDeadToServer(SOCKET sSocket);
SOCKET sSocket;
int score = 0;
using namespace std;
int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("��ĳ�������OV2.2!\n");
		//9. �M�z��ĳ�����T��
		WSACleanup();
		return -1;
	}
	printf("��ĳ������V2.2!\n");
	//2. �Ы�SOCKET================================================================
	sSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SOCKET_ERROR == sSocket) {
		printf("�Ы�SOCKET����: %d\n", GetLastError());
		return -2;
	}
	printf("�Ы�SOCKET���\\n");
	//3. �T�wSERVER����ĳ�a�}================================================================
	SOCKADDR_IN addr = { 0 };
	addr.sin_family = AF_INET;							//�PSOCKET��ƲĤ@�ӰѼƫO���@��
	//addr.sin_addr.S_un.S_addr = inet_pton("192.168.1.108"); //���aIP�a�}
	int status = inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
	addr.sin_port = htons(9527);					//�ݤf�]�w  �b10000���k
	//4. �s��Server================================================================
	int r = connect(sSocket, (sockaddr*)&addr, sizeof addr);
	if (r == -1) {
		printf("�s��Server����: %d\n", GetLastError());
		return -2;
	}
	printf("�s��Server���\\n");
	//10101

	CreateThread(NULL, NULL,
		(LPTHREAD_START_ROUTINE)sendScoreToServer, NULL, NULL, NULL); //�I�s�h�u�{�禡�å�LP���૬
	
	using V2i = sf::Vector2i;
	using V2f = sf::Vector2f;
	using Tex = sf::Texture;

	V2i fieldsize(20, 15);   

	Tex blockTexture;                                          
	if (!blockTexture.loadFromFile("block.png")) {
		cout << "block.png is not found" << endl;
		return EXIT_FAILURE;
	}
	sf::Font font;
	if (!font.loadFromFile("msjh.ttc")) {
		return EXIT_FAILURE;
	}
	sf::Text text("", font, 20);
	text.setFillColor(sf::Color::White);
	text.setPosition(10, 10);
	vector<V2i> snake = { V2i(3, 4) }; 
	V2i food(rand() % fieldsize.x, rand() % fieldsize.y);  

	sf::Sprite block(blockTexture);

	V2f blocksize(block.getGlobalBounds().width, block.getGlobalBounds().height);   

	sf::VideoMode mode(
		unsigned int(fieldsize.x * blocksize.x),
		unsigned int(fieldsize.y * blocksize.y));

	sf::RenderWindow w(sf::VideoMode(mode), L"�g�Y�D");

	enum class Direction { UP, DOWN, LEFT, RIGHT };    
	Direction direction = Direction::RIGHT;

	sf::Clock clock;

	bool isDead = false;
	
	while (w.isOpen()) {
		//USER���ʪ������P�B�z
		sf::Event evt;
		if (w.pollEvent(evt)) {
			if (evt.type == sf::Event::Closed) {
				w.close();
			}
			if (evt.type == sf::Event::KeyPressed) {
				if (evt.key.code == sf::Keyboard::Up)    {direction = Direction::UP;}
				if (evt.key.code == sf::Keyboard::Down)  {direction = Direction::DOWN;}
				if (evt.key.code == sf::Keyboard::Left)  {direction = Direction::LEFT;}
				if (evt.key.code == sf::Keyboard::Right) {direction = Direction::RIGHT;}
			}
		}
		if (clock.getElapsedTime().asSeconds() >= 0.1f) {    
			sf::Vector2i head = snake[0];
			if (direction == Direction::UP)    head.y--;
			if (direction == Direction::DOWN)  head.y++;
			if (direction == Direction::LEFT)  head.x--;
			if (direction == Direction::RIGHT) head.x++;
			if (head.x < 0 || head.x >= fieldsize.x ||    
				head.y < 0 || head.y >= fieldsize.y) {
				isDead = true;
			}
			else {
				for (int i = 1; i < snake.size(); i++) {   
					if (head == snake[i]) {
						isDead = true;
					}
				}
			}

			if (!isDead) {
				if (food == head) {				      
					score++;
					cout << "Eaten, score: " << score << endl;
					food = sf::Vector2i(rand() % fieldsize.x, rand() % fieldsize.y);
					sendScoreToServer(sSocket, score);
				} else {
					snake.pop_back();
				}
				snake.insert(snake.begin(), head);    
				text.setString("Score: " + to_string(score));
			}
			clock.restart();
		}
		if (isDead) {							
			w.clear(sf::Color::Red);
			//sendisDeadToServer(sSocket);
		}
		else {
			w.clear();
		}
		V2f FoodPos(food.x * blocksize.x, food.y * blocksize.y);
		block.setPosition(FoodPos);
		w.draw(block);
		for (const V2i& body : snake) {
			V2f bodyPos(body.x * blocksize.x, body.y * blocksize.y);
			block.setPosition(bodyPos);
			w.draw(block);
		}
		w.draw(text);
		w.display();
	}
	while (1);
	return 0;
}
void sendScoreToServer(SOCKET sSocket, int score) {
	//5. �q�T
	char buff[256] = { 0 };
		memset(buff, 0, 256); // �M�Ű}�C
		snprintf(buff, sizeof(buff), "Score: %d", score);  //�qcmd���f���� �õo��Server
		send(sSocket, buff, (strlen(buff)), NULL);  //�o�e��Server
}
void sendisDeadToServer(SOCKET sSocket) {
	//5. �q�T
	char buff[256] = { 0 };
	memset(buff, 0, 256); // �M�Ű}�C
	snprintf(buff, sizeof(buff), "Game Over");  //�qcmd���f���� �õo��Server
	send(sSocket, buff, (strlen(buff)), NULL);  //�o�e��Server
}