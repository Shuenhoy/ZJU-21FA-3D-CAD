# ZJU-21FA-3D-CAD

浙江大学 2021 年秋季学期《三维CAD建模基础》作业：基于欧拉操作的扫掠体生成。

## 运行环境
仅测试 `ArchLinux`。

## 依赖

* `CMake >= 3.14` 构建工具。
* `GNU GCC >= 11.1` 编译器。

以下依赖会在编译时自动从Github下载：

* `Eigen 3.4` 本项目使用的线性代数库。
* `exprtk` 用于解析数学表达式。
* `clip2tri` 用于将面片三角化，以用于后续渲染。
* `libigl` 用于三维模型的渲染。

## 编译方式
1. 配置。
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```
该步进行时，将会自动从Github下载上述依赖。若你的网络状况不佳，此步可能会遭遇失败。此种情形下，建议使用`proxychains`，以代理方式执行该命令：`proxychains cmake -S . -B build`。

2. 编译。
```bash
cmake --build build
```
经过编译，可执行文件位于`./build/brep_sweep`。

## 运行方式
```
build/brep_sweep <string:底面描述文件> <string:扫掠描述文件> <double:扫掠步长> <int:扫掠步数>
```

若底面描述文件与扫掠描述文件描述的扫掠体在几何上有效，即会出现窗口展示结果。可以按`l`键关闭三角形网格线的渲染，按`i`键翻转法向以便于观察内环面。更多由`libigl`支持的按键参见命令行输出。
