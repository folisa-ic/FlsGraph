#pragma once
#include"head.h"
#include <stdio.h>
/*
�����������ע�ͣ�
����Ԥ����׶ο���ʹ��OpenMP�����٣�������Ч������
scatter�׶���GPU�ϵļ���Ч���Ϻã�100�����ҵļ��ٱȣ�
gather�׶���δ���ԣ������scatter�׶ν����ѣ���δ����ʱ�����һ����ֲ��GPU��
������GPU�ļ��پ�δ����������GPU��CPU�м�Ĵ��䣬��ȫͼ�Ķ�α�������ɴ��������ݴ����˷�
��ý�ȫ���ĵ�������ȫ������GPU�ϣ��ǳ����ѣ�Ӧ����Ҫ��CUDA�и��������⣩

���Կ��ǶԶ��㼯���������������飬��������ƽ��ÿ���߼��ı����������GPU��FPGA�ϵĲ��ж�
Ŀǰ�İ������ŷ���ķ������±߼������ǳ���ƽ����һЩblock�д��ڴ������˷ѵ�thread
*/

// ������̬����
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