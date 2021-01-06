#include "head.h"

extern "C" void GPU_KERNEL(float*, float*, float*, float*, float*);
float GPU_time_all = 0;

// SSSP�ں�����
void SSSP_on_GPU(Vertex_Set_P* Vertex_Set, Edge_Set_P* Edge_Set, Msg_Set_P* Msg_Set, int source_id)
{
	// ����Դ�����Ȩ��Ϊ0
	int partition = source_id / M;
	int num_in_set = source_id % M;
	Vertex_Set[partition]->vertex[num_in_set]->vertex_weight = 0;
	int num_iteration = 0;

	// ����CPU�ڴ�ռ����ڽ���GPU������
	float* dest_vertex_weight_gathered_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// ����CPU�ڴ�ռ����ڸ������¶����last_id
	int* src_id_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// ����CPU���ڴ�ռ�������GPU��������
	float* edge_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));
	float* src_vertex_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));
	float* dest_vertex_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));

	// �洢���еĶ������䶥�㼯�еı�ţ����䶥�㼯�ı�ţ�
	int* src_id_in_Vertex_Set_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));
	int* dest_id_in_Vertex_Set_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));
	int* dest_id_in_which_Vertex_Set = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// ���̶ֹ���������ݷ���ѭ���⸳ֵ
	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;

		// ��CPU�ṹ���ڵ�����ת�Ƶ���������host������
		for (int j = 0; j < num_edge; j++)
		{
			int num = i * NUM_EDGE_LIST + j;
			src_id_in_Vertex_Set_host[num] = Edge_Set[i]->edge[j]->scr_id % M;
			dest_id_in_Vertex_Set_host[num] = Edge_Set[i]->edge[j]->dest_id % M;
			dest_id_in_which_Vertex_Set[num] = Edge_Set[i]->edge[j]->dest_id / M;
			src_id_host[num] = Edge_Set[i]->edge[j]->scr_id;
			edge_weight_host[num] = Edge_Set[i]->edge[j]->edge_weight;
		}
	}

	while (1)
	{
		// ���ṹ����������봴����������
		for (int i = 0; i < K; i++)
		{
			int num_edge = Edge_Set[i]->num_edge;

			// ��CPU�ṹ���ڵ�����ת�Ƶ���������host������
			for (int j = 0; j < num_edge; j++)
			{
				int num = i * NUM_EDGE_LIST + j;
				src_vertex_weight_host[num] = Vertex_Set[i]->vertex[src_id_in_Vertex_Set_host[num]]->vertex_weight;
				dest_vertex_weight_host[num] = Vertex_Set[dest_id_in_which_Vertex_Set[num]]->vertex[dest_id_in_Vertex_Set_host[num]]->vertex_weight;	// ����gather�׶���Ҫ������bug�ĸ�Դ������û��ʹ�� dest_id_in_which_Vertex_Set[num]
			}
		}

		float time_for_single_loop;
		// ���б�CPU��װ���kernel����
		GPU_KERNEL(edge_weight_host,
			src_vertex_weight_host,
			dest_vertex_weight_host,
			dest_vertex_weight_gathered_host,
			&time_for_single_loop
		);

		GPU_time_all += time_for_single_loop;

		// ����ȫ������Ȩֵ
		int finish = 1;
		for (int i = 0; i < K; i++)
		{
			int num_edge = Edge_Set[i]->num_edge;
			for (int j = 0; j < num_edge; j++)
			{
				int num = i * NUM_EDGE_LIST + j;
				if (dest_vertex_weight_gathered_host[num] < Vertex_Set[dest_id_in_which_Vertex_Set[num]]->vertex[dest_id_in_Vertex_Set_host[num]]->vertex_weight)
				{
					Vertex_Set[dest_id_in_which_Vertex_Set[num]]->vertex[dest_id_in_Vertex_Set_host[num]]->vertex_weight = dest_vertex_weight_gathered_host[num];
					Vertex_Set[dest_id_in_which_Vertex_Set[num]]->vertex[dest_id_in_Vertex_Set_host[num]]->last_id = src_id_host[num];
					finish = 0;
				}
			}
		}
		num_iteration++;
		if (finish == 1)
		{
			printf("finish the SSSP!\n");
			printf("number of iteration: %d\n", num_iteration);
			break;
		}
	}

	// �ͷ������CPU�ڴ�ռ�
	free(dest_vertex_weight_gathered_host);
	free(src_id_host);
	free(edge_weight_host);
	free(src_vertex_weight_host);
	free(dest_vertex_weight_host);
	free(src_id_in_Vertex_Set_host);
	free(dest_id_in_Vertex_Set_host);
	free(dest_id_in_which_Vertex_Set);
}