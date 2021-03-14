#include "head.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "device_functions.h"
#include <cuda.h>
#include <time.h>

// ��ʼ��GPU
bool InitCUDA()
{
	int count;
	cudaGetDeviceCount(&count);		// ȡ��֧�� CUDA ���豸����Ŀ�����ϵͳ��û��֧��CUDA��װ�ã���ᴫ��1
	if (count == 0) {
		fprintf(stderr, "There is no device.\n");
		return false;
	}
	int i;
	for (i = 0; i < count; i++) {	// device0��һ�������豸����֧��CUDA1.0���ϵĹ���
		cudaDeviceProp prop;
		if (cudaGetDeviceProperties(&prop, i) == cudaSuccess) {		// cudaGetDeviceProperties��ȡ��װ�õĸ������ݣ�������Ϊ�˻�ȡ֧�ְ汾��prop.major��
			if (prop.major >= 1) {
				break;				// �����֧��CUDA1.0���ϰ汾���豸������
			}
		}
	}
	if (i == count) {				// ��ʾû���ҵ�֧��CUDA1.0���ϰ汾���豸
		fprintf(stderr, "There is no device supporting CUDA 1.x.\n");
		return false;
	}
	cudaSetDevice(i);				// ���ҵ����豸��Ϊ��ǰʹ�õ��豸
	return true;
}

// GPU kernal����scatter
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
	// ���Դ��������ṹ��ָ�������ռ�
	cudaMalloc(&edge_in_GPU_struct, K * NUM_EDGE_LIST * sizeof(edge_in_GPU));

	for (int i = 0; i < K; i++)
	{
		int num_edge = Edge_Set[i]->num_edge;

		// ��CPU�ṹ���ڵ�����ת�Ƶ���������host������
		for (int j = 0; j < num_edge; j++)
		{
			int num = i * NUM_EDGE_LIST + j;
			cudaMemcpy(&edge_in_GPU_struct[num], edge_in_CPU_struct[num], sizeof(struct edge_in_CPU), cudaMemcpyHostToDevice);
		}
	}

	// �������ݵ�GPU�ڴ���
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

	// ������������CPU�����ڽ��յ��ڴ���
	cudaMemcpy(dest_vertex_weight_gathered_host, dest_vertex_weight_gathered_device, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyDeviceToHost);

	// �ͷŵ�GPU�ڴ�
	cudaFree(msg_value_device);
	cudaFree(dest_vertex_weight_device);
	cudaFree(dest_vertex_weight_gathered_device);
}