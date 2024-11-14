# Games101_homework

#### pa0

output:

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20240922164429281.png" alt="image-20240922164429281" style="width: 10%;" />

#### hw1

[1]: https://blog.poivrehxx.site/2024/10/27/games101_hw1/	"hw1数学推导过程"

实现了mvp矩阵 & 围绕某一个绕原点的轴旋转。

output：

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/GIF%202024-10-27%2020-00-39.gif" alt="GIF 2024-10-27 20-00-39" style="width:25%;" />

#### hw2

一开始没有判定着色的范围，导致了深度大一点的三角形少了一块（应该是先绘制了绿色的三角形，导致后面三角形没有着上色）。

##### PS. 

这里的黑边倒是还挺明显的。

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20241110152824588.png" alt="image-20241110152824588" style="width:40%;" />

这是修改了修改帧缓冲规则后的z-buffer效果：

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20241110153017629.png" alt="image-20241110153017629" style="width:40%;" />

这是对每一个像素使用z-buffer和使用MSAA的效果：

感觉效果不是  很显著。

没前面重绘整个包围盒的大黑边效果好。

![image-20241110153111166](https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20241110155038379.png)

然后试了下分16个区域，好像还是没什么变化。。。。

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20241110161205559.png" alt="image-20241110161205559" style="width:40%;" />

##### PS.. 

发现问题是处在判定点是否在三角形内的函数 `insideTriangle()` 上（曹操盖饭），原来的定义里传入的坐标形参是整形的！

```c++
// 问题出在这里！！
static bool insideTriangle(int x, int y, const Vector3f* _v) { }
```

<img src="https://pic-poivre.oss-cn-hangzhou.aliyuncs.com/pics/image-20241114135243245.png" alt="image-20241114135243245" style="width:40%;" />
