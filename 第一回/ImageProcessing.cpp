/*
BMP�t�@�C���ɑ΂���, �ȉ��̏������s��
�E�O���[�X�P�[���ϊ�
�E�G�b�W���o(Prewitt�I�y���[�^, Sobel�I�y���[�^)
�E�g��C�k���C��], �ړ�(���`��Ԗ@)
*/


#include<stdio.h>
#include<stdlib.h>
#include<math.h>

//char�^�̍ő�l�ƍŏ��l���`����
const double MIN_CHAR = 0;
const double MAX_CHAR = 255;

//�~�������`����
const double PI = 3.141592;

//2�l���̎��̂������l
const unsigned char THRESHOLD = 150;

//�摜�̉�f�l��ۑ�
typedef struct {
	unsigned char *red;
	unsigned char *blue;
	unsigned char *green;
}ImageData;

//BMP�̃w�b�_���Ɖ�f�l��ێ�����
typedef struct {
	unsigned char	FileHeader[14];
	unsigned int	Size;
	int				Width, Height;
	unsigned char	InfoHeader[28];
	ImageData img;
}BMP;

//BMP�t�@�C���ǂݍ���
void ReadBmp(const char FileName[], BMP* bmp);
//�摜����������
void WriteBmp(const char FileName[], BMP* bmp);
//BMP�\���̂̂̃������J��
void Destruct_Bmp(BMP* bmp);
//BMP�\���̂̏���\��
void Show_BmpInfo(BMP* bmp);
//�O���[�X�P�[���ϊ�
void ConvertToGrey(BMP* bmp);
//�ړ����σt�B���^
void Average_Image(BMP *bmp);
//prewitt�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_ByPrewitt(BMP* bmp);
//Sobel�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_BySobel(BMP *bmp);
//��ݍ��݉��Z���s��
void Convolve_ByKernel(BMP* bmp, double Kernel_Vertical[], double Kernel_Horizontal[]);
//2�l������
void Binarize_Image(BMP *bmp);
//�A�t�B���ϊ����s��
void Convert_ByAffine(BMP *bmp, double Matrix[], int x_movement, int y_movement);
//�摜�̉�]
void Rotate_Image(BMP *bmp, double theta);
//�摜�̊g��
void Scale_Image(BMP *bmp, double x_scale, double y_scale);
//�摜�̕��s�ړ�
void Move_Image_Parallelly(BMP *bmp, double x_movement, double y_movement);

int main() {
	BMP bmp_forGrey, bmp_forPrewitt, bmp_forSobel, bmp_forRotation, bmp_forScale, bmp_forParallel;

	/*-----------------------------�O���[�X�P�[���ϊ�-------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forGrey);
	ConvertToGrey(&bmp_forGrey);
	WriteBmp("lenna_out_Grey.bmp", &bmp_forGrey);
	Destruct_Bmp(&bmp_forGrey);

	/*-----------------------------Prewitt�I�y���[�^�ɂ��G�b�W���o------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forPrewitt);
	//Average_Image(&bmp_forPrewitt);
	ConvertToGrey(&bmp_forPrewitt);
	DetectEdge_ByPrewitt(&bmp_forPrewitt);
	Binarize_Image(&bmp_forPrewitt);
	WriteBmp("lenna_out_Prewitt.bmp", &bmp_forPrewitt);
	Destruct_Bmp(&bmp_forPrewitt);
	
	/*-----------------------------Sobel�I�y���[�^�ɂ��G�b�W���o------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forSobel);
	ConvertToGrey(&bmp_forSobel);
	DetectEdge_BySobel(&bmp_forSobel);
	Binarize_Image(&bmp_forSobel);
	WriteBmp("lenna_out_Sobel.bmp", &bmp_forSobel);
	Destruct_Bmp(&bmp_forSobel);

	/*----------------------------�摜�̉�]--------------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forRotation);
	Rotate_Image(&bmp_forRotation, 45);
	WriteBmp("lenna_out_Rotate.bmp", &bmp_forRotation);
	Destruct_Bmp(&bmp_forRotation);

	/*-----------------------------�摜�̊g��------------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forScale);
	Scale_Image(&bmp_forScale, 5, 3);
	WriteBmp("lenna_out_Scale.bmp", &bmp_forScale);
	Destruct_Bmp(&bmp_forScale);

	/*----------------------------�摜�̕��s�ړ�-----------------------------------------------------*/
	ReadBmp("lenna.bmp", &bmp_forParallel);
	Move_Image_Parallelly(&bmp_forParallel, 100, 50);
	WriteBmp("lenna_out_Parallel.bmp", &bmp_forParallel);
	Destruct_Bmp(&bmp_forParallel);	

	return 0;
}


//BMP�t�@�C���ǂݍ���
void ReadBmp(const char FileName[], BMP* bmp)
{
	FILE* fp;

	//visual studio�p
	//fopen_s(&fp, FileName, "rb");
	//vscode�p
	fp = fopen(FileName, "rb");
	if (fp == NULL) {
		printf("Not Found : %s", FileName);
		exit(1);
	}

	//�w�b�_�[����ǂݍ���
	fread(bmp->FileHeader, sizeof(unsigned char), 14, fp);
	fread(&bmp->Size,	   sizeof(int),   1, fp);
	fread(&bmp->Width,     sizeof(int),   1, fp);
	fread(&bmp->Height,    sizeof(int),   1, fp);
	fread(bmp->InfoHeader, sizeof(unsigned char), 28, fp);

	//�J�����t�@�C����BMP�t�@�C�����m�F����
	if(bmp->FileHeader[0] != 'B' || bmp->FileHeader[1] != 'M'){
		printf("This is not BMP File\n");
		exit(1);
	}
	//�摜�{�̂̓ǂݍ���
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
	//���4byte�P�ʂ̒����łȂ��Ƃ����Ȃ��̂�, padding�������s��
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

	//�ǂ݂��񂾃t�@�C�������
	fclose(fp);
}

//�摜����������
void WriteBmp(const char FileName[], BMP* bmp)
{
	FILE* fp;
	//visual studio�p
	//fopen_s(&fp, FileName, "wb");
	//vscode�p
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

	//���4byte�P�ʂłȂ���΂Ȃ�Ȃ��̂ŁApadding�������s��
	int stride = (bmp->Width * 3 + 3) / 4 * 4;
	unsigned char padding = 00;

	//�摜�{�̂̏�������
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

//BMP�\���̂̃����������
void Destruct_Bmp(BMP* bmp)
{
	free(bmp->img.red);
	free(bmp->img.green);
	free(bmp->img.blue);
}
//BMP�\���̂̏���\��(�f�o�b�N�p)
void Show_BmpInfo(BMP* bmp)
{
	printf("width  =%d\n", bmp->Width);
	printf("height =%d\n", bmp->Height);
	printf("size   = %d\n", bmp->Height * bmp->Width);

}
//�O���[�X�P�[���ϊ�
void ConvertToGrey(BMP* bmp)
{
	for (int h = 0; h < bmp->Height; h++) {
		for (int w = 0; w < bmp->Width; w++) {
			unsigned char Average = 0;
			//ITU-R Rec BT601�����ŏd�ݕ���
			Average = (0.299 * bmp->img.red[w + h * bmp->Width]
				+ 0.587 * bmp->img.green[w + h * bmp->Width]
				+ 0.114 * bmp->img.blue[w + h * bmp->Width]);
			
			bmp->img.red  [w + h * bmp->Width] = Average;
			bmp->img.green[w + h * bmp->Width] = Average;
			bmp->img.blue [w + h * bmp->Width] = Average;			
		}
	}
}

//�ړ����σt�B���^
void Average_Image(BMP *bmp)
{
	double Kernel_Average[9] = { 1.0/9, 1.0/9, 1.0/9, 
								 1.0/9, 1.0/9, 1.0/9,
								 1.0/9, 1.0/9, 1.0/9};
	Convolve_ByKernel(bmp, Kernel_Average, Kernel_Average);
							
}
//prewitt�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_ByPrewitt(BMP* bmp)
{
	//�c�����̃J�[�l���C�������̃J�[�l�����`
	double Kernel_Vertical[9] = {  -1, -1, -1,
								    0,  0,  0,
								    1,  1,  1 };
	double Kernel_Horizontal[9] = { -1, 0,  1,
								    -1, 0,  1,
								    -1, 0,  1 };

	//��ݍ��݉��Z���s���D�G�b�W�̋����̒l��V���ȉ�f�l�Ƃ���
	Convolve_ByKernel(bmp, Kernel_Vertical, Kernel_Horizontal);
}

//Sobel�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_BySobel(BMP* bmp)
{
	//�c�����̃J�[�l���C�������̃J�[�l�����`
	double Kernel_Vertical[9] = { -1, -2, -1,
								   0,  0,  0,
								   1,  2,  1 };
	double Kernel_Horizontal[9] = { -1, 0,  1,
								    -2, 0,  2,
							        1, 0,  1};

	//��ݍ��݉��Z���s���D�G�b�W�̋����̒l��V���ȉ�f�l�Ƃ���
	Convolve_ByKernel(bmp, Kernel_Vertical, Kernel_Horizontal);
}

//��ݍ��݉��Z(3*3�̃J�[�l����p����
void Convolve_ByKernel(BMP* bmp, double Kernel_Vertical[], double Kernel_Horizontal[])
{
	//�������̃G�b�W�C�c�����̃G�b�W�C�G�b�W�̋������ꎞ�I�ɕێ�����ϐ�
	double temp_x_red  , temp_y_red  , temp_sum_red;
	double temp_x_green, temp_y_green, temp_sum_green;
	double temp_x_blue , temp_y_blue , temp_sum_blue;

	//��[�C���[�C���[�C�E�[�ȊO�ɂ��āC��ݍ��݉��Z���s��
	for (int h = 1; h < bmp->Height-1; h++) {
		for (int w = 1; w < bmp->Width - 1; w++) {
			//������
			temp_x_red   = temp_y_red   = temp_sum_red   = 255;
			temp_x_green = temp_y_green = temp_sum_green = 255;
			temp_x_blue  = temp_y_blue  = temp_sum_blue  = 255;

			//��ݍ��݉��Z
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
			//�G�b�W�̋������v�Z����
			temp_sum_red   = sqrt(pow(temp_x_red  , 2) + pow(temp_y_red  , 2));
			temp_sum_green = sqrt(pow(temp_x_green, 2) + pow(temp_y_green, 2));
			temp_sum_blue  = sqrt(pow(temp_x_blue , 2) + pow(temp_y_blue , 2));

			//�G�b�W�̋����̒l��0����255�Ɏ��߂�
			if (temp_sum_red < MIN_CHAR)	temp_sum_red = MIN_CHAR;
			else if (temp_sum_red > MAX_CHAR) temp_sum_red = MAX_CHAR;


			if (temp_sum_blue < MIN_CHAR)	temp_sum_blue = MIN_CHAR;
			else if (temp_sum_blue > MAX_CHAR) temp_sum_blue = MAX_CHAR;

			if (temp_sum_green < MIN_CHAR)	temp_sum_green = MIN_CHAR;
			else if (temp_sum_green > MAX_CHAR) temp_sum_green = MAX_CHAR;


			//�G�b�W�̋����̒l��V���ȉ�f�l�Ƃ���
			bmp->img.red  [w + h * bmp->Width] = (unsigned char) temp_sum_red;
			bmp->img.green[w + h * bmp->Width] = (unsigned char) temp_sum_green;
			bmp->img.blue [w + h * bmp->Width] = (unsigned char) temp_sum_blue;			
		}
	}
}

//2�l������
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

/*Affine�ϊ����s��
bmp        : BMP�\����
Matrix     : 2*2�̍��W�ϊ��s��
x_movement : �������̕��s�ړ���
y_movement : �c�����̕��s�ړ���
*/
void Convert_ByAffine(BMP *bmp, double Matrix[], int x_movement, int y_movement)
{
	int Affined_x, Affined_y;	//�ϊ���̍��W
	double temp_x, temp_y;		//�ϊ��O�̍��W�������ňꎞ�I�ɕێ�
	int Orignal_x, Original_y;	//�ϊ��O�̍��W
	ImageData temp_img;	//��f�l���ꎞ�I�ɕێ�

	//���I�z��̊m�ۂƏ�����
	temp_img.red   = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	temp_img.green = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	temp_img.blue  = (unsigned char*)malloc(sizeof(char) * bmp->Width * bmp->Height);
	for(int i = 0; i < bmp->Width * bmp->Height; i++){
		temp_img.red[i]   = 0;
		temp_img.green[i] = 0;
		temp_img.green[i] = 0;
	}

	double Inversed_Matrix[4];
	//Matrix�̃G���[�`�F�b�N

	//�t�s����v�Z����
	Inversed_Matrix[0] = Matrix[3];
	Inversed_Matrix[1] = -1.0 * Matrix[1];
	Inversed_Matrix[2] = -1.0 * Matrix[2];
	Inversed_Matrix[3] = Matrix[0];
	for(int i=0; i < 4; i++){
		Inversed_Matrix[i] = Inversed_Matrix[i] / (Matrix[0] * Matrix[3] - Matrix[1] * Matrix[2]);
	}


	for(Affined_y = 0; Affined_y < bmp->Height; Affined_y++){
		for(Affined_x = 0; Affined_x < bmp->Width; Affined_x++){
			//�ϊ��s���p���āC�ϊ���̍��W�ɑΉ�����ϊ��O�̍��W�����߂�
			//�摜�̒��������_�Ƃ��č�p�����Ă��邱�Ƃɒ���
			temp_x = Inversed_Matrix[0] * (Affined_x - bmp->Width/2.0 - x_movement) + Inversed_Matrix[1] * (Affined_y - bmp->Height/2.0 - y_movement);
			temp_y = Inversed_Matrix[2] * (Affined_x - bmp->Width/2.0 - x_movement) + Inversed_Matrix[3] * (Affined_y - bmp->Height/2.0- y_movement);
			//���W�n�����̍��������_�̂��̂ɖ߂�
			temp_x += bmp->Width/2.0;
			temp_y += bmp->Height/2.0;
			//�ϊ��O�̍��W�����̔z��͈͓̔��ɂȂ��ꍇ�́C0��������
			if((temp_x < 1 || temp_x > bmp->Width-2) || (temp_y < 1 || temp_y > bmp->Height-2)){
				temp_img.red  [Affined_x + Affined_y * bmp->Width] = 0;
				temp_img.green[Affined_x + Affined_y * bmp->Width] = 0;
				temp_img.blue [Affined_x + Affined_y * bmp->Width] = 0;
			}
			else {
				//�ϊ���̍��W�ɑΉ�����_����{�I�ɑ��݂��Ȃ�(�����l�ƂȂ�Ȃ�)�̂�, �ߖT4�_�ɂ����`�ߎ����s��
				//http://ipr20.cs.ehime-u.ac.jp/column/gazo_syori/chapter3.html�Q��
				double a = temp_x - (int)temp_x;
				double b = temp_y - (int)temp_y;

				//�ߖT4�_�̍��W���v�Z
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

/*�摜�̉�]
bmp   : BMP�\����
theta : ��]�̊p�x[��]
*/
void Rotate_Image(BMP *bmp, double theta)
{
	double rad = theta * PI / 180.0;

	//��]�s����`
	double Matrix_forRotation [] = {cos(rad), -1.0 * sin(rad), sin(rad), cos(rad)};

	//��]�s���p����Affine�ϊ����s��
	Convert_ByAffine(bmp, Matrix_forRotation, 0, 0);
}

/*�摜�̊g��
bmp : BMP�\����
x_scale : �������ɉ��{�g�傷�邩�w�肷��ϐ�
y_scale : �c�����ɉ��{�g�傷�邩�w�肷��ϐ�
*/
void Scale_Image(BMP *bmp, double x_scale, double y_scale)
{
	//�g��s����`
	double Matrix_forScale[] = {x_scale, 0, 0, y_scale};

	//�g��s���p����Affine�ϊ����s��
	Convert_ByAffine(bmp, Matrix_forScale, 0, 0);
}

/*�摜�𕽍s�ړ�������
bmp        : BMP�\����
x_movement : x�����Ɉړ������
y_movement : y�����Ɉړ������
*/
void Move_Image_Parallelly(BMP *bmp, double x_movement, double y_movement)
{
	//Affine�ϊ�����ۂ̕ϊ��s��͒P�ʍs��(�摜���̂��͕̂ω������Ȃ�)
	double UnitMatrix[] = {1.0, 0, 0, 1.0};

	//Affine�ϊ�
	Convert_ByAffine(bmp, UnitMatrix, x_movement, y_movement);
}






















