#include<stdio.h>
#include<stdlib.h>
#include<time.h>

// 测试集包含
// 顶点数：	1024个（0-1023）
// 边数：	10148条
/*
直接按照顶点编号对顶点进行分组，会导致边集合的大小不一，后续可以思考改进的方法
测试完成，功能基本正确，可输出每个顶点到源顶点的最短路径及该路径的上一个顶点
Graph500生成的图中存在两个顶点之间有多条边的情况，但是不影响SSSP，对其他算法如BFS的影响未验证
*/

#define M 50
#define K 21
#define NUM_EDGE_LIST 1280
#define NUM_MSG_LIST 1280
#define NUM_EDGE_ALL 10148
#define MAX_WEIGHT 1000

// src顶点数据结构
typedef struct vertex* Vertex_P;
struct vertex {
	int vertex_id;
	int last_id;
	float vertex_weight;
};

// 顶点集数据结构
typedef struct Vertex_Set* Vertex_Set_P;
struct Vertex_Set {
	Vertex_P vertex[M];
};

// 边列表数据结构
typedef struct edge* Edge_P;
struct edge {
	int scr_id;
	int dest_id;
	float edge_weight;
};

// 边集数据结构
typedef struct Edge_Set* Edge_Set_P;
struct Edge_Set {
	Edge_P edge[NUM_EDGE_LIST];
	int num_edge;
};

// msg数据结构
typedef struct msg* Msg_P;
struct msg {
	int last_id;
	int dest_id;
	float value;
};

// 信息集数据结构
typedef struct Msg_Set* Msg_Set_P;
struct Msg_Set {
	Msg_P msg[NUM_MSG_LIST];
	int num_msg;
};

// 初始化顶点列表
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

// 初始化边列表
void Edge_Set_init(Edge_Set_P* Edge_Set, const char* filename)
{
	// 初始化边集
	for (int i = 0; i < K; i++)
	{
		Edge_Set[i] = (Edge_Set_P)malloc(sizeof(struct Edge_Set));
		Edge_Set[i]->num_edge = 0;
	}

	// 打开文件，将每条边边逐一导入相应的边集
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

// 简单的冒泡排序
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

// 对每个Edge_Set中的edge按照dest_id进行排序
void sort_dest(Edge_Set_P* Edge_Set)
{
	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;
		sort(Edge_Set[i]->edge, num_edge);
	}
}

// 初始化Msg_Set
void Msg_Set_init(Msg_Set_P* Msg_Set)
{
	for (int i = 0; i < K; i++)
	{
		Msg_Set[i] = (Msg_Set_P)malloc(sizeof(struct Msg_Set));		// 仅为集合申请了空间，具体的msg等待生成时在申请
		Msg_Set[i]->num_msg = 0;
	}
}

double CPU_time_all = 0;

// SSSP内核排序
void SSSP(Vertex_Set_P* Vertex_Set, Edge_Set_P* Edge_Set, Msg_Set_P* Msg_Set, int source_id)
{
	// 设置源顶点的权重为0
	int partition = source_id / M;
	int num_in_set = source_id % M;
	Vertex_Set[partition]->vertex[num_in_set]->vertex_weight = 0;

	int num_iteration = 0;
	CPU_time_all = 0;
	while (1)
	{
		// Scatter

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

		num_iteration++;

		if (finish == 1)
		{
			printf("finish the SSSP!\n");
			printf("number of iteration: %d\n", num_iteration);
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

// 声明静态变量
Vertex_Set_P Vertex_Set[K];
Edge_Set_P Edge_Set[K];
Msg_Set_P Msg_Set[K];

// CPU程序计时
clock_t start_time, finish_time;
double cord_time;

int main()
{
	clock_t start;
	clock_t end;

	Vertex_Set_init(Vertex_Set);
	Edge_Set_init(Edge_Set, "C:\\Users\\great\\Desktop\\data.txt");
	sort_dest(Edge_Set);
	Msg_Set_init(Msg_Set);
	start = clock();
	SSSP(Vertex_Set, Edge_Set, Msg_Set, 1000);
	end = clock();
	double time = (double)(end - start) / CLOCKS_PER_SEC;
	printf("CPU time: %fms\n", time * 1000);
	/*
	printf("%f\n", Vertex_Set[0]->vertex[2]->vertex_weight);
	printf("%d\n", Vertex_Set[0]->vertex[2]->last_id);
	printf("%d\n", Vertex_Set[11]->vertex[1]->last_id);
	printf("%d\n", Vertex_Set[19]->vertex[48]->last_id);
	printf("%d\n", Vertex_Set[7]->vertex[7]->last_id);
	printf("%d\n", Vertex_Set[4]->vertex[28]->last_id);
	printf("%d\n", Vertex_Set[6]->vertex[38]->last_id);
	*/
}