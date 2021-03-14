#include "head.h"
#include <string.h>

// 创建文件输出结果
void result_output(const char* path, const char* filename, Vertex_Set_P* Vertex_Set)
{
	char path_filename[100];

	strcpy_s(path_filename, sizeof(path_filename), path);
	strcat_s(path_filename, sizeof(path_filename), filename);
	strcat_s(path_filename, sizeof(path_filename), ".txt");

	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, path_filename, "w+");
	if (err != 0)
		printf("the file was not opened!\n");
	else
	{
		char dest_id[5];
		char weight[20];
		for (int i = 0; i < K; i++)
		{
			for (int j = 0; j < M; j++)
			{
				sprintf_s(dest_id, sizeof(dest_id), "%d", Vertex_Set[i]->vertex[j]->vertex_id);
				fputs(dest_id, fp);
				fputs(" ", fp);
				sprintf_s(weight, sizeof(weight), "%f", Vertex_Set[i]->vertex[j]->vertex_weight);
				fputs(weight, fp);
				fputs("\n", fp);
			}
		}
		fclose(fp);
	}
}