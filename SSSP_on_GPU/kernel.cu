#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "device_functions.h"
#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "head.h"

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
	float* edge_weight_device,
	float* src_vertex_weight_device,
	float* dest_vertex_weight_device,
	float* msg_value_device,
	float* dest_vertex_weight_gathered_device)
{
	const int tid = threadIdx.x;
	const int bid = blockIdx.x;

	msg_value_device[bid * NUM_EDGE_LIST + tid] = edge_weight_device[bid * NUM_EDGE_LIST + tid] + src_vertex_weight_device[bid * NUM_EDGE_LIST + tid];
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
	float* edge_weight_host,
	float* src_vertex_weight_host,
	float* dest_vertex_weight_host,
	float* dest_vertex_weight_gathered_host,
	float* time_for_single_loop
)
{
	float* msg_value_device;
	float* dest_vertex_weight_gathered_device;
	float* edge_weight_device;
	float* src_vertex_weight_device;
	float* dest_vertex_weight_device;

	cudaMalloc(&msg_value_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&edge_weight_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&src_vertex_weight_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&dest_vertex_weight_device, K * NUM_EDGE_LIST * sizeof(float));
	cudaMalloc(&dest_vertex_weight_gathered_device, K * NUM_EDGE_LIST * sizeof(float));

	// �������ݵ�GPU�ڴ���
	cudaMemcpy(edge_weight_device, edge_weight_host, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(src_vertex_weight_device, src_vertex_weight_host, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(dest_vertex_weight_device, dest_vertex_weight_host, K * NUM_EDGE_LIST * sizeof(float), cudaMemcpyHostToDevice);

	InitCUDA();

	cudaEvent_t start, stop;
	float time = 0.f;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
	cudaEventRecord(start, 0);

	scatter_and_gather << <K, NUM_EDGE_LIST >> > (
		edge_weight_device,
		src_vertex_weight_device,
		dest_vertex_weight_device,
		msg_value_device,
		dest_vertex_weight_gathered_device
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
	cudaFree(edge_weight_device);
	cudaFree(src_vertex_weight_device);
	cudaFree(dest_vertex_weight_device);
	cudaFree(dest_vertex_weight_gathered_device);
}