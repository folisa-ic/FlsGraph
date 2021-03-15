一个单源最短路径图算法（SSSP）的加速，将基于CPU/GPU/FPGA实现，随着项目进行将开源全部源代码

其中：  
"Graph2020"参考了Graph500(https://github.com/graph500/graph500)中生成图的算法，可自由给定顶点和边的数目，最终在Octave上运行得到随机生成的图用于测试程序。  
"SSSP_on_CPU"为完全在CPU上实现的单源最短路径算法。  
"SSSP_on_GPU"中将算法主要内容放在GPU上运行，数据在显存上以数组的形式存储，最后结果输出CPU和GPU的计算用时对比性能，获得了大约100倍左右的加速比。  
"SSSP_on_GPU_structure"中数据在显存中以结构体的形式存储，功能正确但算法速度有所下降，分析可能的原因：结构体的数据结构并不利于GPU上的线程并行访问DRAM，会使DRAM的compulsory row-conflict变为之前的两倍，故最终的GPU运算时间相比数组结构的版本有增加；同时，由于每个结构体的传输都要调用一次cudaMemcpy函数，而结构体数组元素数等同于边的数目（非常大），包含数据传输的总运行时间则有极大的增加。  
    
部分思路参考论文《High-throughput and Energy-efficient Graph Processing on FPGA》
