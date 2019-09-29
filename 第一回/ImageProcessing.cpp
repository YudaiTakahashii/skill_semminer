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

//円周率を定義する
const double PI = 3.141592;

//2値化の時のしきい値
const unsigned char THRESHOLD = 150;

//画像の画素値を保存
typedef struct {
	unsigned char *red;
	unsigned char *blue;
	unsigned char *green;
}ImageData;

//BMPのヘッダ情報と画素値を保持する
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
//移動平均フィルタ
void Average_Image(BMP *bmp);
//prewittオペレータによるエッジ検出
void DetectEdge_ByPrewitt(BMP* bmp);
//Sobelオペレータによるエッジ検出
void DetectEdge_BySobel(BMP *bmp);
//畳み込み演算を行う
void Convolve_ByKernel(BMP* bmp, double Kernel_Vertical[], double Kernel_Horizontal[]);
//2値化処理
void Binarize_Image(BMP *bmp);
//アフィン変換を行う
void Convert_ByAffine(BMP *bmp, double Matrix[], int x_movement, int y_movement);
//画像の回転
void Rotate_Image(BMP *bmp, double theta);
//画像の拡大
void Scale_Image(BMP *bmp, double x_scale, double y_scale);
//画像の平行移動
void Move_Image_Parallelly(BMP *bmp, double x_movement, double y_movement);

int main() {
	BMP bmp_forGrey, bmp_forPrewitt, bmp_forSobel, bmp_forRotation, bmp_forScale, bmp_forParallel;

	/*-----------------------------グレースケール変換-------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forGrey);
	ConvertToGrey(&bmp_forGrey);
	WriteBmp("lenna_out_Grey.bmp", &bmp_forGrey);
	Destruct_Bmp(&bmp_forGrey);

	/*-----------------------------Prewittオペレータによるエッジ検出------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forPrewitt);
	//Average_Image(&bmp_forPrewitt);
	ConvertToGrey(&bmp_forPrewitt);
	DetectEdge_ByPrewitt(&bmp_forPrewitt);
	Binarize_Image(&bmp_forPrewitt);
	WriteBmp("lenna_out_Prewitt.bmp", &bmp_forPrewitt);
	Destruct_Bmp(&bmp_forPrewitt);
	
	/*-----------------------------Sobelオペレータによるエッジ検出------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forSobel);
	ConvertToGrey(&bmp_forSobel);
	DetectEdge_BySobel(&bmp_forSobel);
	Binarize_Image(&bmp_forSobel);
	WriteBmp("lenna_out_Sobel.bmp", &bmp_forSobel);
	Destruct_Bmp(&bmp_forSobel);

	/*----------------------------画像の回転--------------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forRotation);
	Rotate_Image(&bmp_forRotation, 45);
	WriteBmp("lenna_out_Rotate.bmp", &bmp_forRotation);
	Destruct_Bmp(&bmp_forRotation);

	/*-----------------------------画像の拡大------------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forScale);
	Scale_Image(&bmp_forScale, 5, 3);
	WriteBmp("lenna_out_Scale.bmp", &bmp_forScale);
	Destruct_Bmp(&bmp_forScale);

	/*----------------------------画像の平行移動-----------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forParallel);
	Move_Image_Parallelly(&bmp_forParallel, 100, 50);
	WriteBmp("lenna_out_Parallel.bmp", &bmp_forParallel);
	Destruct_Bmp(&bmp_forParallel);	

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

	//開いたファイルがBMPファイルか確認する
	if(bmp->FileHeader[0] != 'B' || bmp->FileHeader[1] != 'M'){
		printf("This is not BMP File\n");
		exit(1);
	}
	//画像本体の読み込み
	bmp->img.red   = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
	bmp->img.blue  = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
	bmp->img.green = (unsigned char*)malloc((bmp->Width) * (bmp->Height) * sizeof(unsigned char));
	if (bmp->img.red == NULL) {
		printf("Failed : malloc\n");
		exit(1);
	}
	if (bmp->img.green == NULL) {
		printf("Failed : malloc\n");
		exit(1);
	}
	if (bmp->img.blue == NULL) {
		printf("Failed : malloc\n");
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
//BMP構造体の情報を表示(デバック用)
void Show_BmpInfo(BMP* bmp)
{
	printf("width  =%d\n", bmp->Width);
	printf("height =%d\n", bmp->Height);
	printf("size   = %d\n", bmp->Height * bmp->Width);

}
//グレースケール変換
void ConvertToGrey(BMP* bmp)
{
	for (int h = 0; h < bmp->Height; h++) {
		for (int w = 0; w < bmp->Width; w++) {
			unsigned char Average = 0;
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

//移動平均フィルタ
void Average_Image(BMP *bmp)
{
	double Kernel_Average[9] = { 1.0/9, 1.0/9, 1.0/9, 
								 1.0/9, 1.0/9, 1.0/9,
								 1.0/9, 1.0/9, 1.0/9};
	Convolve_ByKernel(bmp, Kernel_Average, Kernel_Average);
							
}
//prewittオペレータによるエッジ検出
void DetectEdge_ByPrewitt(BMP* bmp)
{
	//縦方向のカーネル，横方向のカーネルを定義
	double Kernel_Vertical[9] = {  -1, -1, -1,
								    0,  0,  0,
								    1,  1,  1 };
	double Kernel_Horizontal[9] = { -1, 0,  1,
								    -1, 0,  1,
								    -1, 0,  1 };

	//畳み込み演算を行い．エッジの強さの値を新たな画素値とする
	Convolve_ByKernel(bmp, Kernel_Vertical, Kernel_Horizontal);
}

//Sobelオペレータによるエッジ検出
void DetectEdge_BySobel(BMP* bmp)
{
	//縦方向のカーネル，横方向のカーネルを定義
	double Kernel_Vertical[9] = { -1, -2, -1,
								   0,  0,  0,
								   1,  2,  1 };
	double Kernel_Horizontal[9] = { -1, 0,  1,
								    -2, 0,  2,
							        1, 0,  1};

	//畳み込み演算を行い．エッジの強さの値を新たな画素値とする
	Convolve_ByKernel(bmp, Kernel_Vertical, Kernel_Horizontal);
}

//畳み込み演算(3*3のカーネルを用いる
void Convolve_ByKernel(BMP* bmp, double Kernel_Vertical[], double Kernel_Horizontal[])
{
	//横方向のエッジ，縦方向のエッジ，エッジの強さを一時的に保持する変数
	double temp_x_red  , temp_y_red  , temp_sum_red;
	double temp_x_green, temp_y_green, temp_sum_green;
	double temp_x_blue , temp_y_blue , temp_sum_blue;

	//上端，下端，左端，右端以外について，畳み込み演算を行う
	for (int h = 1; h < bmp->Height-1; h++) {
		for (int w = 1; w < bmp->Width - 1; w++) {
			//初期化
			temp_x_red   = temp_y_red   = temp_sum_red   = 255;
			temp_x_green = temp_y_green = temp_sum_green = 255;
			temp_x_blue  = temp_y_blue  = temp_sum_blue  = 255;

			//畳み込み演算
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					temp_x_red   += Kernel_Horizontal[j + 3 * i] * double(bmp->img.red  [w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_x_green += Kernel_Horizontal[j + 3 * i] * double(bmp->img.green[w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_x_blue  += Kernel_Horizontal[j + 3 * i] * double(bmp->img.blue [w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_y_red   += Kernel_Vertical[j + 3 * i] * double(bmp->img.red  [w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_y_green += Kernel_Vertical[j + 3 * i] * double(bmp->img.green[w - 1 + i + (h - 1 + j) * bmp->Width]);
					temp_y_blue  += Kernel_Vertical[j + 3 * i] * double(bmp->img.blue [w - 1 + i + (h - 1 + j) * bmp->Width]);
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

//2値化処理
void Binarize_Image(BMP *bmp)
{
	for(int i = 0; i< bmp->Width * bmp->Height; i++){
		if(bmp->img.red[i] < THRESHOLD)		bmp->img.red[i] = 255;
		else	bmp->img.red[i] = 0;
		if(bmp->img.green[i] < THRESHOLD)		bmp->img.green[i] = 255;
		else	bmp->img.green[i] = 0;
		if(bmp->img.blue[i] < THRESHOLD)		bmp->img.blue[i] = 255;
		else	bmp->img.blue[i] = 0;
	}
}

/*Affine変換を行う
bmp        : BMP構造体
Matrix     : 2*2の座標変換行列
x_movement : 横方向の平行移動量
y_movement : 縦方向の平行移動量
*/
void Convert_ByAffine(BMP *bmp, double Matrix[], int x_movement, int y_movement)
{
	int Affined_x, Affined_y;	//変換後の座標
	double temp_x, temp_y;		//変換前の座標を実数で一時的に保持
	int Orignal_x, Original_y;	//変換前の座標
	ImageData temp_img;	//画素値を一時的に保持

	//動的配列の確保と初期化
	temp_img.red   = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	temp_img.green = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	temp_img.blue  = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	for(int i = 0; i < bmp->Width * bmp->Height; i++){
		temp_img.red[i]   = 0;
		temp_img.green[i] = 0;
		temp_img.green[i] = 0;
	}

	double Inversed_Matrix[4];
	//Matrixのエラーチェック

	//逆行列を計算する
	Inversed_Matrix[0] = Matrix[3];
	Inversed_Matrix[1] = -1.0 * Matrix[1];
	Inversed_Matrix[2] = -1.0 * Matrix[2];
	Inversed_Matrix[3] = Matrix[0];
	for(int i=0; i < 4; i++){
		Inversed_Matrix[i] = Inversed_Matrix[i] / (Matrix[0] * Matrix[3] - Matrix[1] * Matrix[2]);
	}


	for(Affined_y = 0; Affined_y < bmp->Height; Affined_y++){
		for(Affined_x = 0; Affined_x < bmp->Width; Affined_x++){
			//変換行列を用いて，変換後の座標に対応する変換前の座標を求める
			//画像の中央を原点として作用させていることに注意
			temp_x = Inversed_Matrix[0] * (Affined_x - bmp->Width/2.0 - x_movement) + Inversed_Matrix[1] * (Affined_y - bmp->Height/2.0 - y_movement);
			temp_y = Inversed_Matrix[2] * (Affined_x - bmp->Width/2.0 - x_movement) + Inversed_Matrix[3] * (Affined_y - bmp->Height/2.0- y_movement);
			//座標系を元の左下が原点のものに戻す
			temp_x += bmp->Width/2.0;
			temp_y += bmp->Height/2.0;
			//変換前の座標が元の配列の範囲内にない場合は，0を代入する
			if((temp_x < 1 || temp_x > bmp->Width-2) || (temp_y < 1 || temp_y > bmp->Height-2)){
				temp_img.red  [Affined_x + Affined_y * bmp->Width] = 0;
				temp_img.green[Affined_x + Affined_y * bmp->Width] = 0;
				temp_img.blue [Affined_x + Affined_y * bmp->Width] = 0;
			}
			else {
				//変換後の座標に対応する点が基本的に存在しない(整数値とならない)ので, 近傍4点による線形近似を行う
				//http://ipr20.cs.ehime-u.ac.jp/column/gazo_syori/chapter3.html参照
				double a = temp_x - (int)temp_x;
				double b = temp_y - (int)temp_y;

				//近傍4点の座標を計算
				int x0 = (int)temp_x;
				int x1 = (int)(temp_x + 1);
				int y0 = (int)temp_y;
				int y1 = (int)(temp_y + 1);

				temp_img.red[Affined_x + Affined_y * bmp->Width] = bmp->img.red[x0 + y0 * bmp->Width] * (1.0-a) * b
															 	 + bmp->img.red[x0 + y1 * bmp->Width] * (1.0-a) * (1.0-b)
															 	 + bmp->img.red[x1 + y0 * bmp->Width] *      a  * b
															 	 + bmp->img.red[x1 + y1 * bmp->Width] *      a  * (1-b);

				temp_img.green[Affined_x + Affined_y * bmp->Width] = bmp->img.green[x0 + y0 * bmp->Width] * (1.0-a) * b
															   	   + bmp->img.green[x0 + y1 * bmp->Width] * (1.0-a) * (1.0-b)
															   	   + bmp->img.green[x1 + y0 * bmp->Width] *      a  * b
															   	   + bmp->img.green[x1 + y1 * bmp->Width] *      a  * (1-b);
			
				temp_img.blue[Affined_x + Affined_y * bmp->Width] = bmp->img.blue[x0 + y0 * bmp->Width] * (1.0-a) * b
															  	  + bmp->img.blue[x0 + y1 * bmp->Width] * (1.0-a) * (1.0-b)
															  	  + bmp->img.blue[x1 + y0 * bmp->Width] *      a  * b
															  	  + bmp->img.blue[x1 + y1 * bmp->Width] *      a  * (1-b);	
			}
		}
	}
	for(int i=0; i < bmp->Width * bmp->Height; i++){
		bmp->img.red[i]   = temp_img.red[i];
		bmp->img.green[i] = temp_img.green[i];
		bmp->img.blue[i]  = temp_img.blue[i];
	}

	free(temp_img.red);
	free(temp_img.green);
	free(temp_img.blue)	;
}

/*画像の回転
bmp   : BMP構造体
theta : 回転の角度[°]
*/
void Rotate_Image(BMP *bmp, double theta)
{
	double rad = theta * PI / 180.0;

	//回転行列を定義
	double Matrix_forRotation [] = {cos(rad), -1.0 * sin(rad), sin(rad), cos(rad)};

	//回転行列を用いてAffine変換を行う
	Convert_ByAffine(bmp, Matrix_forRotation, 0, 0);
}

/*画像の拡大
bmp : BMP構造体
x_scale : 横方向に何倍拡大するか指定する変数
y_scale : 縦方向に何倍拡大するか指定する変数
*/
void Scale_Image(BMP *bmp, double x_scale, double y_scale)
{
	//拡大行列を定義
	double Matrix_forScale[] = {x_scale, 0, 0, y_scale};

	//拡大行列を用いてAffine変換を行う
	Convert_ByAffine(bmp, Matrix_forScale, 0, 0);
}

/*画像を平行移動させる
bmp        : BMP構造体
x_movement : x方向に移動する量
y_movement : y方向に移動する量
*/
void Move_Image_Parallelly(BMP *bmp, double x_movement, double y_movement)
{
	//Affine変換する際の変換行列は単位行列(画像そのものは変化させない)
	double UnitMatrix[] = {1.0, 0, 0, 1.0};

	//Affine変換
	Convert_ByAffine(bmp, UnitMatrix, x_movement, y_movement);
}






















