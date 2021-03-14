一个单源最短路径图算法（SSSP）的加速，将基于CPU/GPU/FPGA实现，随着项目进行将开源全部源代码

其中"SSSP_on_CPU"为完全在CPU上实现的单元最短路径算法；
"SSSP_on_GPU"中将算法主要内容放在GPU上运行，最后结果输出CPU和GPU的计算用时对比性能；
"SSSP_on_GPU_structure"中改变了数据在显存中存放的格式，由原本的数组变为结构体，功能正确但性能反而有所下降，与预期不符，推测可能和GPU中多个线程访问DRAM的方式有关；
"Graph2020"参考了Graph500中生成图的算法，在给定顶点和边的数目后，可在Octave上运行得到随机生成的图用于测试程序。

部分思路参考论文《High-throughput and Energy-efficient Graph Processing on FPGA》
