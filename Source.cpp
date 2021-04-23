#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
using namespace std;

#include <Windows.h>

#define _USE_MATH_DEFINES
#include <math.h>

int nScreenWidth = 120;
int nScreenHeight = 40;

float fPlayerX = 8.0f;
float fPlayerY = 8.0f;
float fPlayerA = 0.0f;

int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = M_PI / 4.0;
float fDepth = 16.0f;

int main() {
	//create screen buffer
	wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
	HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);
	DWORD dwBytesWritten = 0;

	wstring map;

	map += L"################";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#---#####------#";
	map += L"#-------#------#";
	map += L"#-------#------#";
	map += L"#-------#------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"#--------------#";
	map += L"################";

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (1) {
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1= tp2;
		float deltaTime = elapsedTime.count();

		//�÷��̾� ����
		if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
			fPlayerA -= 1 * deltaTime;
		if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
			fPlayerA += 1 * deltaTime;
		if (GetAsyncKeyState((unsigned short)'W') & 0x8000) {
			fPlayerX += sinf(fPlayerA) * 5.0f* deltaTime;
			fPlayerY += cosf(fPlayerA) * 5.0f * deltaTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX -= sinf(fPlayerA) * 5.0f * deltaTime;
				fPlayerY -= cosf(fPlayerA) * 5.0f * deltaTime;
			}
		}
		if (GetAsyncKeyState((unsigned short)'S') & 0x8000) {
			fPlayerX -= sinf(fPlayerA) * 5.0f * deltaTime;
			fPlayerY -= cosf(fPlayerA) * 5.0f * deltaTime;
			if (map[(int)fPlayerY * nMapWidth + (int)fPlayerX] == '#') {
				fPlayerX += sinf(fPlayerA) * 5.0f * deltaTime;
				fPlayerY += cosf(fPlayerA) * 5.0f * deltaTime;
			}
		}


		for (int x = 0; x < nScreenWidth; x++) {
			//�� ��ո���, �߻�� ray�� ���� ������ǥ�� ������ �����
			float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

			float fDistanceToWall = 0;
			bool bHitWall = false;
			bool bBoundary = false;

			float fEyeX = sinf(fRayAngle); //�� Ray�� ��������
			float fEyeY = cosf(fRayAngle);

			//ray�� ���̸� .1f�� �÷����鼭 ���� �¾Ҵ��� Ȯ��
			while (!bHitWall && fDistanceToWall < fDepth) {
				fDistanceToWall += .1f;

				int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
				int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

				//ray�� ��踦 ������� Ȯ��
				if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) {
					bHitWall = true;
					fDistanceToWall = fDepth;
				}
				else {
					//���� ���� [���� ray�� x��ǥ,y��ǥ]�ε����� ��('#')�̶��
					if (map.c_str()[nTestY * nMapWidth + nTestX] == '#') {
						bHitWall = true;
						 
						vector<pair<float,float>> p;//�Ÿ�, ����

						for (int tx = 0; tx < 2; tx++) {
							for (int ty = 0; ty < 2; ty++) {
								float vy = (float)nTestY + ty - fPlayerY;
								float vx = (float)nTestX + tx - fPlayerX;
								float d = sqrt(vx * vx + vy * vy);
								float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
								p.push_back(make_pair(d, dot));
							}
						}
						sort(p.begin(), p.end(), [](const pair<float, float>& left, const pair<float, float>& right) {return left.first < right.first; });

						float fBound = .01;
						if (acos(p.at(0).second) < fBound) bBoundary = true;
						if (acos(p.at(1).second) < fBound) bBoundary = true;						
					}
				}
			}

			//�ٴڰ� ��հ��� �÷��̾��� �Ÿ��� ����Ѵ�
			int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
			int nFloor = nScreenHeight - nCeiling;

			short nShade = ' ';

			if (fDistanceToWall <= fDepth / 4.0f) nShade = 0x2588;//fully shaded
			else if (fDistanceToWall < fDepth / 2.0f) nShade = 0x2593;
			else if (fDistanceToWall <= fDepth / 2.0f) nShade = 0x2592;
			else if (fDistanceToWall < fDepth) nShade = 0x2591;
			else nShade = ' ';//�ʹ� ��

			if (bBoundary) nShade = 'I';

			for (int y = 0; y < nScreenHeight; y++) {
				if (y <= nCeiling)
					screen[y * nScreenWidth + x] = ' ';
				else if (y > nCeiling && y <= nFloor)
					screen[y * nScreenWidth + x] = nShade;
				else
				{
					float b  = 1 - (((float)y - nScreenHeight / 2) / ((float)nScreenHeight / 2));
					if (b < .25) nShade = '#';
					else if (b < .5) nShade = 'x';
					else if (b < .75) nShade = '-';
					else if (b < .9) nShade = '.';
					else nShade = ' ';
					screen[y * nScreenWidth + x] = nShade;
				}
			}
		}				
		screen[nScreenWidth * nScreenHeight - 1] = '\0';
		WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
	}
	return 0;
}