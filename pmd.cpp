#include "pmd.h"
#include <windows.h>
#include <stdio.h>

//コンストラクタ
pmd::pmd(char *filename)
{
	FILE *hFP;
	errno_t error;
	if (!(error = fopen_s(&hFP, filename, "rb"))) {
		//ヘッダー読み込み
		fread(&header, sizeof(struct t_header), 1, hFP);

		//頂点数読み込み
		fread(&vert_count, sizeof(vert_count), 1, hFP);

		//頂点データ読み込み
		vertex = new t_vertex[vert_count];
		fread(vertex, 38, vert_count, hFP);

		//面頂点リスト読み込み
		fread(&face_vert_count, sizeof(face_vert_count), 1, hFP);

		//面頂点リストデータ読み込み
		face_vert_index = new unsigned short[face_vert_count];
		fread(face_vert_index, 2, face_vert_count, hFP);

		//材質リスト材質数
		fread(&material_count, sizeof(material_count), 1, hFP);

		//材質リストデータ読み込み
		material = new t_material[material_count];
		fread(material, 70, material_count, hFP);

		fclose(hFP);
	}
	else {
		MessageBoxW(NULL, L"fopen", L"Err", MB_ICONSTOP);
	}
}

//デストラクタ
pmd::~pmd(void)
{
}