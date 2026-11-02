# Kline Quant / Machine Learning Atlas (第119轮)
正式机器学习大类位于 quant/ml/。默认页 index.html 加载第119轮Memory Atlas的720篇完整系统讲义，覆盖24专题和120章；旧版保留为legacy.html。
完整数据是 atlas/data.part1 至 atlas/data.part4 四个有序拼接的gzip二进制片段，拼接后长度1,969,862字节。前端使用原生DecompressionStream(gzip)解压，校验720篇与24专题。数学从SVG的LaTeX alt恢复，由KaTeX渲染。
来源：Memory Atlas 第119轮；Python源码在本轮迁移中未修改；不应将运行验证等同于对全部数学结论的形式证明。
