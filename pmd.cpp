#include "pmd.h"
#include <windows.h>
#include <stdio.h>

//�R���X�g���N�^
pmd::pmd(char *filename)
{
	FILE *hFP;
	errno_t error;
	if (!(error = fopen_s(&hFP, filename, "rb"))) {
		//�w�b�_�[�ǂݍ���
		fread(&header, sizeof(struct t_header), 1, hFP);

		//���_���ǂݍ���
		fread(&vert_count, sizeof(vert_count), 1, hFP);

		//���_�f�[�^�ǂݍ���
		vertex = new t_vertex[vert_count];
		fread(vertex, 38, vert_count, hFP);

		//�ʒ��_���X�g�ǂݍ���
		fread(&face_vert_count, sizeof(face_vert_count), 1, hFP);

		//�ʒ��_���X�g�f�[�^�ǂݍ���
		face_vert_index = new unsigned short[face_vert_count];
		fread(face_vert_index, 2, face_vert_count, hFP);

		//�ގ����X�g�ގ���
		fread(&material_count, sizeof(material_count), 1, hFP);

		//�ގ����X�g�f�[�^�ǂݍ���
		material = new t_material[material_count];
		fread(material, 70, material_count, hFP);

		fclose(hFP);
	}
	else {
		MessageBoxW(NULL, L"fopen", L"Err", MB_ICONSTOP);
	}
}

//�f�X�g���N�^
pmd::~pmd(void)
{
}