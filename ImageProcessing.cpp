/*
BMPファイルに対して, 以下の処理を行う
・グレースケール変換
・エッジ検出(Prewittオペレータ, Sobelオペレータ)
・拡大，縮小，回転, 移動(線形補間法)
*/


#include<stdio.h>
#include<stdlib.h>
#include<math.h>

//char型の最大値と最小値を定義する
const double MIN_CHAR = 0;
const double MAX_CHAR = 255;

typedef struct {
	unsigned char *red;
	unsigned char *blue;
	unsigned char *green;
}ImageData;

typedef struct {
	unsigned char	FileHeader[14];
	unsigned int	Size;
	int				Width, Height;
	unsigned char	InfoHeader[28];
	ImageData img;
}BMP;

//BMPファイル読み込み
void ReadBmp(const char FileName[], BMP* bmp);
//画像を書き込む
void WriteBmp(const char FileName[], BMP* bmp);
//BMP構造体ののメモリ開放
void Destruct_Bmp(BMP* bmp);
//BMP構造体の情報を表示
void Show_BmpInfo(BMP* bmp);
//グレースケール変換
void ConvertToGrey(BMP* bmp);
//prewittオペレータによるエッジ検出
void DetectEdge_ByPrewitt(BMP* bmp);
//Sobelオペレータによるエッジ検出
void DetectEdge_BySobel(BMP *bmp);
//畳み込み演算を行う
void Convolution_ByEdge(BMP* bmp, char kernel_vertical[], char kernel_horizontal[]);


int main() {
	BMP bmp, bmp_forGrey, bmp_forPrewitt, bmp_forSobel;
	ReadBmp("lenna.bmp", &bmp);
	ReadBmp("lenna.bmp", &bmp_forGrey);
	ReadBmp("lenna.bmp", &bmp_forPrewitt);
	ReadBmp("lenna.bmp", &bmp_forSobel);

	ConvertToGrey(&bmp_forGrey);
	DetectEdge_ByPrewitt(&bmp_forPrewitt);
	DetectEdge_BySobel(&bmp_forSobel);
	ConvertToGrey(&bmp_forPrewitt);
	ConvertToGrey(&bmp_forSobel);

	WriteBmp("img_out.bmp", &bmp);
	WriteBmp("lenna_out_Grey.bmp", &bmp_forGrey);
	WriteBmp("lenna_out_Prewitt.bmp", &bmp_forPrewitt);
	WriteBmp("lenna_out_Sobel.bmp", &bmp_forSobel);

	
	Destruct_Bmp(&bmp);
	Destruct_Bmp(&bmp_forGrey);
	Destruct_Bmp(&bmp_forPrewitt);
	/******************************************************/
	/*
	ReadBmp("Parrots.bmp", &bmp);
	ReadBmp("Parrots.bmp", &bmp_forGrey);
	ReadBmp("Parrots.bmp", &bmp_forPrewitt);
	ReadBmp("Parrots.bmp", &bmp_forSobel);

	ConvertToGrey(&bmp_forGrey);
	DetectEdge_ByPrewitt(&bmp_forPrewitt);
	DetectEdge_BySobel(&bmp_forSobel);
	ConvertToGrey(&bmp_forPrewitt);
	ConvertToGrey(&bmp_forSobel);

	WriteBmp("Parrots_out.bmp", &bmp);
	WriteBmp("Parrots_out_Grey.bmp", &bmp_forGrey);
	WriteBmp("Parrots_out_Prewitt.bmp", &bmp_forPrewitt);
	WriteBmp("Parrots_out_Sobel.bmp", &bmp_forSobel);
	*/
	
	Destruct_Bmp(&bmp);
	Destruct_Bmp(&bmp_forGrey);
	Destruct_Bmp(&bmp_forPrewitt);
	return 0;
}


//BMPファイル読み込み
void ReadBmp(const char FileName[], BMP* bmp)
{
	FILE* fp;

	//visual studio用
	//fopen_s(&fp, FileName, "rb");
	//vscode用
	fp = fopen(FileName, "rb");
	if (fp == NULL) {
		printf("Not Found : %s", FileName);
		exit(1);
	}

	//ヘッダー情報を読み込み
	fread(bmp->FileHeader, sizeof(unsigned char), 14, fp);
	fread(&bmp->Size,	   sizeof(int),   1, fp);
	fread(&bmp->Width,     sizeof(int),   1, fp);
	fread(&bmp->Height,    sizeof(int),   1, fp);
	fread(bmp->InfoHeader, sizeof(unsigned char), 28, fp);

		 
	//画像本体の読み込み
	bmp->img.red   = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
	bmp->img.blue  = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
bmp->img.green = (unsigned char*)malloc((bmp->Width) * (bmp->Height) * sizeof(unsigned char));
if (bmp->img.red == NULL) {
	printf("ﾒﾓﾘ確保に失敗\n");
	exit(1);
}
if (bmp->img.green == NULL) {
	printf("ﾒﾓﾘ確保に失敗\n");
	exit(1);
}
if (bmp->img.blue == NULL) {
	printf("ﾒﾓﾘ確保に失敗\n");
	exit(1);
}
//一列4byte単位の長さでないといけないので, padding処理を行う
int stride = (bmp->Width * 3 + 3) / 4 * 4;
unsigned char padding;

for (int h = 0; h < bmp->Height; h++) {
	for (int w = 0; w < bmp->Width; w++) {
		fread(&bmp->img.red[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
		fread(&bmp->img.green[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
		fread(&bmp->img.blue[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
	}
	for (int i = 0; i < stride - bmp->Width * 3; i++) {
		fread(&padding, sizeof(unsigned char), 1, fp);
	}
}

//読みこんだファイルを閉じる
fclose(fp);
}

//画像を書き込む
void WriteBmp(const char FileName[], BMP* bmp)
{
	FILE* fp;
	//visual studio用
	//fopen_s(&fp, FileName, "wb");
	//vscode用
	fp = fopen(FileName, "wb");
	if (fp == NULL) {
		printf("Not Found : %s\n", FileName);
		exit(1);
	}

	fwrite(bmp->FileHeader, sizeof(unsigned char), 14, fp);
	fwrite(&bmp->Size, sizeof(int), 1, fp);
	fwrite(&bmp->Width, sizeof(int), 1, fp);
	fwrite(&bmp->Height, sizeof(int), 1, fp);
	fwrite(bmp->InfoHeader, sizeof(unsigned char), 28, fp);

	//一列4byte単位でなければならないので、padding処理を行う
	int stride = (bmp->Width * 3 + 3) / 4 * 4;
	unsigned char padding = 00;

	//画像本体の書き込み
	for (int h = 0; h < bmp->Height; h++) {
		for (int w = 0; w < bmp->Width; w++) {
			fwrite(&bmp->img.red[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
			fwrite(&bmp->img.green[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
			fwrite(&bmp->img.blue[w + h * bmp->Width], sizeof(unsigned char), 1, fp);
		}
		for (int i = 0; i < stride - bmp->Width * 3; i++) {
			fwrite(&padding, sizeof(unsigned char), 1, fp);
		}

	}

	fclose(fp);
}

//BMP構造体のメモリを解放
void Destruct_Bmp(BMP* bmp)
{
	free(bmp->img.red);
	free(bmp->img.green);
	free(bmp->img.blue);
}

void Show_BmpInfo(BMP* bmp)
{
	printf("幅  =%d\n", bmp->Width);
	printf("高さ=%d\n", bmp->Height);
	printf("大きさ = %d\n", bmp->Height * bmp->Width);

}



//グレースケール変換
void ConvertToGrey(BMP* bmp)
{
	for (int h = 0; h < bmp->Height; h++) {
		for (int w = 0; w < bmp->Width; w++) {
			unsigned char Average = 00;
			//ITU-R Rec BT601方式で重み分け
			Average = (0.299 * bmp->img.red[w + h * bmp->Width]
				+ 0.587 * bmp->img.green[w + h * bmp->Width]
				+ 0.114 * bmp->img.blue[w + h * bmp->Width]);
			
			bmp->img.red  [w + h * bmp->Width] = Average;
			bmp->img.green[w + h * bmp->Width] = Average;
			bmp->img.blue [w + h * bmp->Width] = Average;			
		}
	}
}

//prewittオペレータによるエッジ検出
void DetectEdge_ByPrewitt(BMP* bmp)
{
	//縦方向のカーネル，横方向のカーネルを定義
	char Kernel_Vertical[9] = {  1,  1,  1,
								 0,  0,  0
								-1, -1, -1 };
	char Kernel_Horizontal[9] = { 1, 0,  -1,
								  1, 0,  -1,
								  1, 0,  -1 };

	//畳み込み演算を行い．エッジの強さの値を新たな画素値とする
	Convolution_ByEdge(bmp, Kernel_Vertical, Kernel_Horizontal);
}

void DetectEdge_BySobel(BMP* bmp)
{
	//縦方向のカーネル，横方向のカーネルを定義
	char Kernel_Vertical[9] = { -1, -2, -1,
								 0,  0,  0,
								 1,  2,  1 };
	char Kernel_Horizontal[9] = { -1, 0,  1,
								  -2, 0,  2,
							       1, 0,  1};

	//畳み込み演算を行い．エッジの強さの値を新たな画素値とする
	Convolution_ByEdge(bmp, Kernel_Vertical, Kernel_Horizontal);


}



//畳み込み演算(3*3のカーネルを用いる
void Convolution_ByEdge(BMP* bmp, char kernel_vertical[], char kernel_horizontal[])
{
	//横方向のエッジ，縦方向のエッジ，エッジの強さを一時的に保持する変数
	double temp_x_red  , temp_y_red  , temp_sum_red;
	double temp_x_green, temp_y_green, temp_sum_green;
	double temp_x_blue , temp_y_blue , temp_sum_blue;

	//上端，下端，左端，右端以外について，畳み込み演算を行う
	for (int h = 1; h < bmp->Height-1; h++) {
		for (int w = 1; w < bmp->Width - 1; w++) {
			//初期化
			temp_x_red   = temp_y_red   = temp_sum_red   = 0;
			temp_x_green = temp_y_green = temp_sum_green = 0;
			temp_x_blue  = temp_y_blue  = temp_sum_blue  = 0;

			//畳み込み演算
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					temp_x_red   += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.red  [w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_x_green += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.green[w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_x_blue  += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.blue [w - 1 + i + (h - 1 + j) * bmp->Width]);
					//temp_y_red   += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.red  [w - 1 + i + (h - 1 + j) * bmp->Width]);
					//temp_y_green += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.green[w - 1 + i + (h - 1 + j) * bmp->Width]);
					//temp_y_blue  += double(kernel_horizontal[j + 3 * i]) * double(bmp->img.blue [w - 1 + i + (h - 1 + j) * bmp->Width]);
				}
			}
			//エッジの強さを計算する
			temp_sum_red   = sqrt(pow(temp_x_red  , 2) + pow(temp_y_red  , 2));
			temp_sum_green = sqrt(pow(temp_x_green, 2) + pow(temp_y_green, 2));
			temp_sum_blue  = sqrt(pow(temp_x_blue , 2) + pow(temp_y_blue , 2));

			//エッジの強さの値を0から255に収める
			if (temp_sum_red < MIN_CHAR)	temp_sum_red = MIN_CHAR;
			else if (temp_sum_red > MAX_CHAR) temp_sum_red = MAX_CHAR;


			if (temp_sum_blue < MIN_CHAR)	temp_sum_blue = MIN_CHAR;
			else if (temp_sum_blue > MAX_CHAR) temp_sum_blue = MAX_CHAR;

			if (temp_sum_green < MIN_CHAR)	temp_sum_green = MIN_CHAR;
			else if (temp_sum_green > MAX_CHAR) temp_sum_green = MAX_CHAR;


			//エッジの強さの値を新たな画素値とする
			bmp->img.red  [w + h * bmp->Width] = (unsigned char) temp_sum_red;
			bmp->img.green[w + h * bmp->Width] = (unsigned char) temp_sum_green;
			bmp->img.blue [w + h * bmp->Width] = (unsigned char) temp_sum_blue;			
		}
	}
}



























