# DX12-Defferred

基于DX12龙书框架的延迟渲染实现。项目代码都使用Visual Studio 2022构建。

参考项目：https://github.com/kaku-iwate/DX12
DX12龙书源码：https://github.com/d3dcoder/d3d12book

  该渲染使用两个Pass：
  - 第一个pass绘制G-Buffer，共三张图分别存储：pos、normal和color的信息；
  - 第二个pass采样这三张图实现屏幕空间的光照计算。
  
  计算了三个光源的光照。
![image](https://user-images.githubusercontent.com/55162087/198572971-c27dae86-65f4-4f42-b4d6-a0a7fcff126a.png)
