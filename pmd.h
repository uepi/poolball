
class pmd {
public:
#pragma pack(push,1)	//アラインメント制御をオフる
	//ヘッダー
	struct t_header {
		unsigned char magic[3];
		float version;
		unsigned char model_name[20];
		unsigned char comment[256];
	};
	t_header header;
	//頂点リスト
	unsigned long vert_count;
	struct t_vertex {
		float pos[3];
		float normal_vec[3];
		float uv[2];
		unsigned short bone_num[2];
		unsigned char bone_weight;
		unsigned char edge_flag;
	};
	t_vertex *vertex;
	//面頂点リスト
	unsigned long face_vert_count;
	unsigned short *face_vert_index;

	//Material 
	//材質リスト
	unsigned long material_count;
	struct t_material {
		float diffuse_color[3];
		float alpha;
		float specularity;
		float specular_color[3];
		float mirror_color[3];
		unsigned char toon_index;
		unsigned char edge_flag;
		unsigned long face_vert_count;
		char texture_file_name[20];
	};
	t_material *material;
#pragma pack(pop)	//アラインメント制御エンド

	//コンストラクタ
	pmd(char *);

	//デストラクタ
	~pmd();
};