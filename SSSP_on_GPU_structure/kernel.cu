#include "head.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "device_functions.h"
#include <cuda.h>
#include <time.h>

// 初始化GPU
bool InitCUDA()
{
	int count;
	cudaGetDeviceCount(&count);		// 取得支持 CUDA 的设备的数目，如果系统上没有支持CUDA的装置，则会传回1
	if (count == 0) {
		fprintf(stderr, "There is no device.\n");
		return false;
	}
	int i;
	for (i = 0; i < count; i++) {	// device0是一个仿真设备，不支持CUDA1.0以上的功能
		cudaDeviceProp prop;
		if (cudaGetDeviceProperties(&prop, i) == cudaSuccess) {		// cudaGetDeviceProperties：取得装置的各项数据（这里是为了获取支持版本号prop.major）
			if (prop.major >= 1) {
				break;				// 如果有支持CUDA1.0以上版本的设备，跳出
			}
		}
	}
	if (i == count) {				// 表示没有找到支持CUDA1.0以上版本的设备
		fprintf(stderr, "There is no device supporting CUDA 1.x.\n");
		return false;
	}
	cudaSetDevice(i);				// 将找到的设备设为当前使用的设备
	return true;
}

// GPU kernal函数scatter
__global__ void scatter_and_gather(
	float* dest_vertex_weight_device,
	float* msg_value_device,
	float* dest_vertex_weight_gathered_device,
	struct edge_in_GPU* edge_in_GPU_struct)
{
	const int tid = threadIdx.x;
	const int bid = blockIdx.x;

	msg_value_device[bid * NUM_EDGE_LIST + tid] = edge_in_GPU_struct[bid * NUM_EDGE_LIST + tid].edge_weight + edge_in_GPU_struct[bid * NUM_EDGE_LIST + tid].src_vertex_weight;
	if (msg_value_device[bid * NUM_EDGE_LIST + tid] < dest_vertex_weight_device[bid * NUM_EDGE_LIST + tid])
	{
		dest_vertex_weight_gathered_device[bid * NUM_EDGE_LIST + tid] = msg_value_device[bid * NUM_EDGE_LIST + tid];
		// __syncthreads();
	}
	else
	{
		dest_vertex_weight_gathered_device[bid * NUM_EDGE_LIST + tid] = 10000;
		// __syncthreads();
	}
}

extern "C" void GPU_KERNEL(
	float* dest_vertex_weight_host,
	float* dest_vertex_weight_gathered_host,
	float* time_for_single_loop,
	edge_in_CPU_P * edge_in_CPU_struct,
	Edge_Set_P * Edge_Set
)
{
	float* msg_value_device;
	float* dest_vertex_weight_gathered_device;
	float* dest_vertex_weight_device;

	cudaMalloc(&msg_value_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&dest_vertex_weight_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&dest_vertex_weight_gathered_device, K * NUM_EDGE_LIST * sizeof(float));

	struct edge_in_GPU* edge_in_GPU_struct;
	// 在显存中声明结构体指针的数组空间
	cudaMalloc(&edge_in_GPU_struct, K * NUM_EDGE_LIST * sizeof(edge_in_GPU));

	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;

		// 将CPU结构体内的数据转移到新声明的host数组中
		for (int j = 0; j < num_edge; j++)
		{
			int num = i * NUM_EDGE_LIST + j;
			cudaMemcpy(&edge_in_GPU_struct[num], edge_in_CPU_struct[num], sizeof(struct edge_in_CPU), cudaMemcpyHostToDevice);
		}
	}

	// 传递数据到GPU内存上
	cudaMemcpy(dest_vertex_weight_device, dest_vertex_weight_host, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyHostToDevice);

	InitCUDA();

	cudaEvent_t start, stop;
	float time = 0.f;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);

	scatter_and_gather << <K, NUM_EDGE_LIST >> > (
		dest_vertex_weight_device,
		msg_value_device,
		dest_vertex_weight_gathered_device,
		edge_in_GPU_struct
		);

	cudaEventRecord(stop, 0);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&time, start, stop);
	*time_for_single_loop = time;
	cudaEventDestroy(start);
	cudaEventDestroy(stop);

	// 将计算结果传回CPU中用于接收的内存上
	cudaMemcpy(dest_vertex_weight_gathered_host, dest_vertex_weight_gathered_device, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyDeviceToHost);

	// 释放掉GPU内存
	cudaFree(msg_value_device);
	cudaFree(dest_vertex_weight_device);
	cudaFree(dest_vertex_weight_gathered_device);
}