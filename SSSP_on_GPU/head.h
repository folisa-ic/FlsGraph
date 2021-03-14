#pragma once
#include <stdio.h>
#include <stdlib.h>

// ���Լ�����
// ��������	1024����0-1023��
// ������	10148��
/*
ֱ�Ӱ��ն����ŶԶ�����з��飬�ᵼ�±߼��ϵĴ�С��һ����������˼���Ľ��ķ���
������ɣ����ܻ�����ȷ�������ÿ�����㵽Դ��������·������·������һ������
Graph500���ɵ�ͼ�д�����������֮���ж����ߵ���������ǲ�Ӱ��SSSP���������㷨��BFS��Ӱ��δ��֤
*/

// #define M 21
// #define K 50

// Ŀ����ʹGPU��block��thread����������ã�ʹÿ��block���һ���߼��Ĵ���
// ����Ҫ���Ʊ߼��Ĵ�С���ܳ���1024��
#define M 30
#define K 35
#define NUM_EDGE_LIST 1000
#define NUM_MSG_LIST 1000
#define NUM_EDGE_ALL 10148
#define MAX_WEIGHT 1000

// src�������ݽṹ
typedef struct vertex* Vertex_P;
struct vertex {
	int vertex_id;
	int last_id;
	float vertex_weight;
};

// ���㼯���ݽṹ
typedef struct Vertex_Set* Vertex_Set_P;
struct Vertex_Set {
	Vertex_P vertex[M];
};

// ���б����ݽṹ
typedef struct edge* Edge_P;
struct edge {
	int scr_id;
	int dest_id;
	float edge_weight;
};

// �߼����ݽṹ
typedef struct Edge_Set* Edge_Set_P;
struct Edge_Set {
	Edge_P edge[NUM_EDGE_LIST];
	int num_edge;
};

// msg���ݽṹ
typedef struct msg* Msg_P;
struct msg {
	int last_id;
	int dest_id;
	float value;
};

// ��Ϣ�����ݽṹ
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
