#pragma once
#include <stdio.h>
#include <stdlib.h>

// 测试集包含
// 顶点数：	1024个（0-1023）
// 边数：	10148条
/*
直接按照顶点编号对顶点进行分组，会导致边集合的大小不一，后续可以思考改进的方法
测试完成，功能基本正确，可输出每个顶点到源顶点的最短路径及该路径的上一个顶点
Graph500生成的图中存在两个顶点之间有多条边的情况，但是不影响SSSP，对其他算法如BFS的影响未验证
*/

// #define M 21
// #define K 50

// 目的是使GPU的block中thread数被充分利用，使每个block完成一个边集的处理
// 故需要控制边集的大小不能超过1024条
#define M 30
#define K 35
#define NUM_EDGE_LIST 1000
#define NUM_MSG_LIST 1000
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

void Vertex_Set_init(Vertex_Set_P* Vertex_Set);
void Edge_Set_init(Edge_Set_P* Edge_Set, const char* filename);
void sort(Edge_P* edge, int num_edge);
void sort_dest(Edge_Set_P* Edge_Set);
void Msg_Set_init(Msg_Set_P* Msg_Set);
bool InitCUDA();
extern "C" void GPU_KERNEL(float*, float*, float*, float*, float*);
void SSSP_on_GPU(Vertex_Set_P* Vertex_Set, Edge_Set_P* Edge_Set, Msg_Set_P* Msg_Set, int source_id);
void SSSP_on_CPU(int source_id);
void result_output(const char* path, const char* filename, Vertex_Set_P* Vertex_Set);
