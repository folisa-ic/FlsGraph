#include "head.h"

// ��ʼ�������б�
void Vertex_Set_init(Vertex_Set_P* Vertex_Set)
{
	for (int i = 0; i < K; i++)
	{
		Vertex_Set[i] = (Vertex_Set_P)malloc(sizeof(struct Vertex_Set));
		for (int j = 0; j < M; j++)
		{
			Vertex_Set[i]->vertex[j] = (Vertex_P)malloc(sizeof(struct vertex));
			Vertex_Set[i]->vertex[j]->vertex_id = i * M + j;
			Vertex_Set[i]->vertex[j]->last_id = -1;
			Vertex_Set[i]->vertex[j]->vertex_weight = MAX_WEIGHT;
		}
	}
}

// ��ʼ�����б�
void Edge_Set_init(Edge_Set_P* Edge_Set, const char* filename)
{
	// ��ʼ���߼�
	for (int i = 0; i < K; i++)
	{
		Edge_Set[i] = (Edge_Set_P)malloc(sizeof(struct Edge_Set));
		Edge_Set[i]->num_edge = 0;
	}

	// ���ļ�����ÿ���߱���һ������Ӧ�ı߼�
	FILE* fp;
	errno_t err;
	err = fopen_s(&fp, filename, "r");
	if (err != 0)
		printf("the file was not opened!\n");
	else
	{
		int scr_id;
		int dest_id;
		float edge_weight;
		int partition;
		int num_edge;
		for (int i = 0; i < NUM_EDGE_ALL; i++)
		{
			if (fscanf_s(fp, "%d %d %f", &scr_id, &dest_id, &edge_weight) != EOF)
			{
				partition = scr_id / M;
				num_edge = Edge_Set[partition]->num_edge;
				Edge_Set[partition]->edge[num_edge] = (Edge_P)malloc(sizeof(struct edge));
				Edge_Set[partition]->edge[num_edge]->scr_id = scr_id;
				Edge_Set[partition]->edge[num_edge]->dest_id = dest_id;
				Edge_Set[partition]->edge[num_edge]->edge_weight = edge_weight;
				Edge_Set[partition]->num_edge++;
			}
			else break;
		}
	}
	fclose(fp);
}

// �򵥵�ð������
void sort(Edge_P* edge, int num_edge)
{
	Edge_P temp;
	for (int i = 0; i < num_edge - 1; i++)
	{
		for (int j = 0; j < num_edge - 1; j++)
		{
			if (edge[j]->dest_id > edge[j + 1]->dest_id)
			{
				temp = edge[j];
				edge[j] = edge[j + 1];
				edge[j + 1] = temp;
			}
			else if (edge[j]->dest_id == edge[j + 1]->dest_id)
			{
				if (edge[j]->scr_id > edge[j + 1]->scr_id)
				{
					temp = edge[j];
					edge[j] = edge[j + 1];
					edge[j + 1] = temp;
				}
				else;
			}
			else;
		}
	}
}

// ��ÿ��Edge_Set�е�edge����dest_id��������
void sort_dest(Edge_Set_P* Edge_Set)
{
	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;
		sort(Edge_Set[i]->edge, num_edge);
	}
}

// ��ʼ��Msg_Set
void Msg_Set_init(Msg_Set_P* Msg_Set)
{
	for (int i = 0; i < K; i++)
	{
		Msg_Set[i] = (Msg_Set_P)malloc(sizeof(struct Msg_Set));		// ��Ϊ���������˿ռ䣬�����msg�ȴ�����ʱ������
		Msg_Set[i]->num_msg = 0;
	}
}