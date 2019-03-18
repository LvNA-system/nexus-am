# PARD perf-counters配套测试用例

## 编译指南

编译前需要设置环境变量AM_HOME为AM项目所在的**绝对路径**。

## 编译
`make ARCH=riscv64-rocket`

## 运行
1. 删除PARD/fpga/build目录下的bin.txt、linux.bin、mem.bin
2. 将build目录下的bin文件移动到PARD/fpga/build目录下
3. 更改PARD配置，使每核有至少128MB内存。例如：如果是单核系统，则ExtMem配置为128MB，双核则为256MB。
4. 到PARD/fpga/emu目录下，运行`make run-emu -j4`
