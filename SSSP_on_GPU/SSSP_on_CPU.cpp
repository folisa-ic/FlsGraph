#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"head.h"

extern Vertex_Set_P Vertex_Set[];
extern Edge_Set_P Edge_Set[];
extern Msg_Set_P Msg_Set[];

// SSSP内核排序
void SSSP(Vertex_Set_P* Vertex_Set, Edge_Set_P* Edge_Set, Msg_Set_P* Msg_Set, int source_id)
{
	// 设置源顶点的权重为0
	int partition = source_id / M;
	int num_in_set = source_id % M;
	Vertex_Set[partition]->vertex[num_in_set]->vertex_weight = 0;

	int num_iteration = 0;
	double CPU_time_all = 0;
	while (1)
	{
		// Scatter

		// CPU计时
		clock_t start;
		clock_t end;
		start = clock();
		for (int i = 0; i < K; i++)
		{
			int num_edge = Edge_Set[i]->num_edge;
			int e_src_id;
			int e_dest_id;
			int src_id_in_Vertex_Set;
			int dest_id_in_Msg_Set;
			int num_msg;
			float src_vertex_weight;		// paper中的a
			for (int j = 0; j < num_edge; j++)
			{
				e_src_id = Edge_Set[i]->edge[j]->scr_id;
				e_dest_id = Edge_Set[i]->edge[j]->dest_id;
				src_id_in_Vertex_Set = e_src_id % M;
				dest_id_in_Msg_Set = e_dest_id / M;
				if (Vertex_Set[i]->vertex[src_id_in_Vertex_Set]->vertex_id != e_src_id)
				{
					printf("error!\n");
				}
				src_vertex_weight = Vertex_Set[i]->vertex[src_id_in_Vertex_Set]->vertex_weight;

				num_msg = Msg_Set[dest_id_in_Msg_Set]->num_msg;
				Msg_Set[dest_id_in_Msg_Set]->msg[num_msg] = (Msg_P)malloc(sizeof(struct msg));
				Msg_Set[dest_id_in_Msg_Set]->msg[num_msg]->value = Edge_Set[i]->edge[j]->edge_weight + src_vertex_weight;
				Msg_Set[dest_id_in_Msg_Set]->msg[num_msg]->dest_id = e_dest_id;
				Msg_Set[dest_id_in_Msg_Set]->msg[num_msg]->last_id = e_src_id;
				Msg_Set[dest_id_in_Msg_Set]->num_msg++;
			}
		}

		// Gather
		int finish = 1;
		for (int i = 0; i < K; i++)
		{
			int num_msg = Msg_Set[i]->num_msg;
			int msg_dest_id;
			int dest_id_in_Vertex_Set;
			float msg_dest_weight;		// paper中的b

			for (int j = 0; j < num_msg; j++)
			{
				// 找到dest在该Vertex_Set中对应的编号
				msg_dest_id = Msg_Set[i]->msg[j]->dest_id;
				dest_id_in_Vertex_Set = msg_dest_id % M;

				msg_dest_weight = Vertex_Set[i]->vertex[dest_id_in_Vertex_Set]->vertex_weight;
				if (Msg_Set[i]->msg[j]->value < msg_dest_weight)
				{
					Vertex_Set[i]->vertex[dest_id_in_Vertex_Set]->vertex_weight = Msg_Set[i]->msg[j]->value;
					Vertex_Set[i]->vertex[dest_id_in_Vertex_Set]->last_id = Msg_Set[i]->msg[j]->last_id;
					finish = 0;
				}
			}
		}

		end = clock();
		double time = (double)(end - start) / CLOCKS_PER_SEC;
		CPU_time_all += time;

		num_iteration++;

		if (finish == 1)
		{
			printf("finish the SSSP!\n");
			printf("number of iteration: %d\n", num_iteration);
			printf("CPU time: %fms\n", CPU_time_all * 1000);
			break;
		}

		// 释放掉为Msg申请的空间，并将num_msg清零
		for (int i = 0; i < K; i++)
		{
			for (int j = 0; j < Msg_Set[i]->num_msg; j++)
			{
				free(Msg_Set[i]->msg[j]);
			}
			Msg_Set[i]->num_msg = 0;
		}
	}
}

void SSSP_on_CPU(int source_id)
{
	Vertex_Set_init(Vertex_Set);
	Edge_Set_init(Edge_Set, "C:\\Users\\great\\Desktop\\data.txt");
	sort_dest(Edge_Set);
	Msg_Set_init(Msg_Set);
	SSSP(Vertex_Set, Edge_Set, Msg_Set, source_id);
}