#pragma once
#include"head.h"
#include <stdio.h>
/*
真正有意义的注释：
数据预处理阶段可以使用OpenMP来加速，但测试效果不佳
scatter阶段在GPU上的加速效果较好（100倍左右的加速比）
gather阶段尚未尝试（相较于scatter阶段较困难），未来有时间可以一并移植到GPU上
以上在GPU的加速均未考虑数据在GPU和CPU中间的传输，对全图的多次遍历将造成大量的数据传输浪费
最好将全部的迭代过程全部放在GPU上（非常困难，应该需要对CUDA有更深入的理解）

可以考虑对顶点集按出度数排序后分组，将有利于平均每个边集的边数，提高在GPU和FPGA上的并行度
目前的按顶点编号分组的方法导致边集数量非常不平均，一些block中存在大量被浪费的thread
*/

// 声明静态变量
Vertex_Set_P Vertex_Set[K];
Edge_Set_P Edge_Set[K];
Msg_Set_P Msg_Set[K];

int main()
{
	Vertex_Set_init(Vertex_Set);
	Edge_Set_init(Edge_Set, "C:\\Users\\great\\Desktop\\data.txt");
	sort_dest(Edge_Set);
	Msg_Set_init(Msg_Set);

	if (!InitCUDA()) {
		return 0;
	}
	printf("finish initialization of GPU!\n");
	int source_id;
	printf("please enter the soucer_id: \n");
	scanf_s("%d", &source_id);
	SSSP_on_GPU(Vertex_Set, Edge_Set, Msg_Set, source_id);
	extern float GPU_time_all;
	printf("GPU time: %fms\n", GPU_time_all);
	SSSP_on_CPU(source_id);
	result_output("C:\\Users\\great\\Desktop\\", "result.txt", Vertex_Set);
}