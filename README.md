# DX12-Defferred

学习完了DX12龙书之后想实现一个最简单的延迟渲染，但因为水平不够没能实现，之后在github上看到了别人的实现，就跟着做了一遍，收获很大。
项目代码都使用Visual Studio 2022构建。

参考项目：https://github.com/kaku-iwate/DX12
DX12源码：https://github.com/d3dcoder/d3d12book

# 延迟渲染项目简单介绍
  该渲染使用两个Pass，第一个pass绘制三张图分别存储：pos、normal和color信息；第二个pass采样这三张图实现屏幕空间的光照计算。
![image](https://user-images.githubusercontent.com/55162087/198572971-c27dae86-65f4-4f42-b4d6-a0a7fcff126a.png)
