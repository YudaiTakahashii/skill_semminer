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
//prewitt�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_ByPrewitt(BMP* bmp);
//Sobel�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_BySobel(BMP *bmp);
//��ݍ��݉��Z���s��
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

		 
	//�摜�{�̂̓ǂݍ���
	bmp->img.red   = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
	bmp->img.blue  = (unsigned char*)malloc( (bmp->Width) * (bmp->Height) * sizeof(unsigned char));
bmp->img.green = (unsigned char*)malloc((bmp->Width) * (bmp->Height) * sizeof(unsigned char));
if (bmp->img.red == NULL) {
	printf("��؊m�ۂɎ��s\n");
	exit(1);
}
if (bmp->img.green == NULL) {
	printf("��؊m�ۂɎ��s\n");
	exit(1);
}
if (bmp->img.blue == NULL) {
	printf("��؊m�ۂɎ��s\n");
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

void Show_BmpInfo(BMP* bmp)
{
	printf("��  =%d\n", bmp->Width);
	printf("����=%d\n", bmp->Height);
	printf("�傫�� = %d\n", bmp->Height * bmp->Width);

}



//�O���[�X�P�[���ϊ�
void ConvertToGrey(BMP* bmp)
{
	for (int h = 0; h < bmp->Height; h++) {
		for (int w = 0; w < bmp->Width; w++) {
			unsigned char Average = 00;
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

//prewitt�I�y���[�^�ɂ��G�b�W���o
void DetectEdge_ByPrewitt(BMP* bmp)
{
	//�c�����̃J�[�l���C�������̃J�[�l�����`
	char Kernel_Vertical[9] = {  1,  1,  1,
								 0,  0,  0
								-1, -1, -1 };
	char Kernel_Horizontal[9] = { 1, 0,  -1,
								  1, 0,  -1,
								  1, 0,  -1 };

	//��ݍ��݉��Z���s���D�G�b�W�̋����̒l��V���ȉ�f�l�Ƃ���
	Convolution_ByEdge(bmp, Kernel_Vertical, Kernel_Horizontal);
}

void DetectEdge_BySobel(BMP* bmp)
{
	//�c�����̃J�[�l���C�������̃J�[�l�����`
	char Kernel_Vertical[9] = { -1, -2, -1,
								 0,  0,  0,
								 1,  2,  1 };
	char Kernel_Horizontal[9] = { -1, 0,  1,
								  -2, 0,  2,
							       1, 0,  1};

	//��ݍ��݉��Z���s���D�G�b�W�̋����̒l��V���ȉ�f�l�Ƃ���
	Convolution_ByEdge(bmp, Kernel_Vertical, Kernel_Horizontal);


}



//��ݍ��݉��Z(3*3�̃J�[�l����p����
void Convolution_ByEdge(BMP* bmp, char kernel_vertical[], char kernel_horizontal[])
{
	//�������̃G�b�W�C�c�����̃G�b�W�C�G�b�W�̋������ꎞ�I�ɕێ�����ϐ�
	double temp_x_red  , temp_y_red  , temp_sum_red;
	double temp_x_green, temp_y_green, temp_sum_green;
	double temp_x_blue , temp_y_blue , temp_sum_blue;

	//��[�C���[�C���[�C�E�[�ȊO�ɂ��āC��ݍ��݉��Z���s��
	for (int h = 1; h < bmp->Height-1; h++) {
		for (int w = 1; w < bmp->Width - 1; w++) {
			//������
			temp_x_red   = temp_y_red   = temp_sum_red   = 0;
			temp_x_green = temp_y_green = temp_sum_green = 0;
			temp_x_blue  = temp_y_blue  = temp_sum_blue  = 0;

			//��ݍ��݉��Z
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



























