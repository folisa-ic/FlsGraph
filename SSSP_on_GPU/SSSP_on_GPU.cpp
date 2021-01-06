#include "head.h"

extern "C" void GPU_KERNEL(float*, float*, float*, float*, float*);
float GPU_time_all = 0;

// SSSP内核排序
void SSSP_on_GPU(Vertex_Set_P* Vertex_Set, Edge_Set_P* Edge_Set, Msg_Set_P* Msg_Set, int source_id)
{
	// 设置源顶点的权重为0
	int partition = source_id / M;
	int num_in_set = source_id % M;
	Vertex_Set[partition]->vertex[num_in_set]->vertex_weight = 0;
	int num_iteration = 0;

	// 创建CPU内存空间用于接收GPU运算结果
	float* dest_vertex_weight_gathered_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// 创建CPU内存空间用于辅助更新顶点的last_id
	int* src_id_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// 创建CPU的内存空间用于向GPU传递数据
	float* edge_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));
	float* src_vertex_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));
	float* dest_vertex_weight_host = (float*)malloc(K * NUM_EDGE_LIST * sizeof(float));

	// 存储所有的顶点在其顶点集中的编号（或其顶点集的编号）
	int* src_id_in_Vertex_Set_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));
	int* dest_id_in_Vertex_Set_host = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));
	int* dest_id_in_which_Vertex_Set = (int*)malloc(K * NUM_EDGE_LIST * sizeof(int));

	// 部分固定不变的数据放在循环外赋值
	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;

		// 将CPU结构体内的数据转移到新声明的host数组中
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
		// 将结构体的数据送入创建的数组中
		for (int i = 0; i < K; i++)
		{
			int num_edge = Edge_Set[i]->num_edge;

			// 将CPU结构体内的数据转移到新声明的host数组中
			for (int j = 0; j < num_edge; j++)
			{
				int num = i * NUM_EDGE_LIST + j;
				src_vertex_weight_host[num] = Vertex_Set[i]->vertex[src_id_in_Vertex_Set_host[num]]->vertex_weight;
				dest_vertex_weight_host[num] = Vertex_Set[dest_id_in_which_Vertex_Set[num]]->vertex[dest_id_in_Vertex_Set_host[num]]->vertex_weight;	// 后续gather阶段需要，最终bug的根源就在于没有使用 dest_id_in_which_Vertex_Set[num]
			}
		}

		float time_for_single_loop;
		// 运行被CPU封装后的kernel函数
		GPU_KERNEL(edge_weight_host,
			src_vertex_weight_host,
			dest_vertex_weight_host,
			dest_vertex_weight_gathered_host,
			&time_for_single_loop
		);

		GPU_time_all += time_for_single_loop;

		// 更新全部顶点权值
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

	// 释放申请的CPU内存空间
	free(dest_vertex_weight_gathered_host);
	free(src_id_host);
	free(edge_weight_host);
	free(src_vertex_weight_host);
	free(dest_vertex_weight_host);
	free(src_id_in_Vertex_Set_host);
	free(dest_id_in_Vertex_Set_host);
	free(dest_id_in_which_Vertex_Set);
}