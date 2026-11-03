# Kline Quant / Machine Learning Atlas

正式课程入口为 `quant/ml/index.html`，通过侧边栏进入。课程主体包含 **24 个专题、120 章、720 篇知识点讲义**。原版备份不参与正式阅读流程，不能将遗留的独立稿或旧版页面当成已发布教材。

## 数据与正文组成

- `atlas/data.part1` 至 `atlas/data.part4`：按顺序拼接的 gzip 数据。压缩后总长固定为 1,969,862 字节；前端解压后检查 720 个唯一讲义 ID 与 24 个专题。
- `atlas-editorial.json`：针对原始正文进行 20 处定义校订、31 处成立条件/适用边界校订及 2 处数学表达式勘误。仅在确认目标原文匹配时应用，不覆盖原始档案。
- `atlas-depth.json`、`atlas-depth-foundations.json`、`atlas-depth-learning.json`、`atlas-depth-quant.json`：第一阶段进阶讲义。
- `atlas-depth-foundations-2.json`、`atlas-depth-learning-2.json`、`atlas-depth-quant-2.json`：第二阶段进阶讲义。
- 7 个进阶模块合计提供 **56 篇增强正文**。每篇直接附着在已有知识点下，无独立页面或重复目录。24 个专题均有覆盖，48 篇来自分专题审计的相对薄弱文章，另 8 篇来自先前针对 MLOps/金融研究的方法性增强。

页面加载顺序：完整档案 → 精确正文勘误 → 进阶正文附加 → 全文索引及目录。前端 `atlas.js` 校验模块数、目标 ID、原始标题、增强内容结构以及专题覆盖。数学由 KaTeX 渲染。可以通过“只看进阶深化”筛选 56 篇增强正文。

## 持续校验

在仓库根目录运行：

```bash
python quant/ml/tools/verify_atlas.py
node quant/ml/test_ml.mjs
python -m unittest discover quant/ml -p 'test_examples.py' -v
```

其中 Node.js 脚本需要依赖 `katex@0.16.11`，可参照 `.github/workflows/test-quant-ml.yml` 安装。GitHub Actions 在 `quant/ml/**` 更新后运行数学/JavaScript 结构检查及 Python 示例测试。

主要校验包括档案完整性、HTML 结构、720 段 Python 的语法、已内链目标、补充内容与真实讲义标题的对应关系、KaTeX 公式语法，以及所有已识别通用模板段落不再残留。自动测试 **不等于** 正文数学结论的形式化证明、全部示例在真实环境中执行成功、浏览器端所有交互验证或量化策略的实盘可交易性证明。

## 编辑准则

1. 内容必须落到既有讲义 ID；每项深化明确数学对象、符号、条件、推导、工程/研究流程和失败边界。
2. 所有样本内外比较均应说明数据可得时点、特征与标签的时间关系、估计窗和成本/风险口径；不能用回测结果代替验证。
3. 新增模块保留 `source_title` 与原文标题逐字对应，并提供至少两条可被 KaTeX 校验的数学公式及足够具体的推导和限制条件。
4. 不增加“独立稿”“待扩展”“泛化提示”等替代正文的占位模块；临时研究备份不得进入公开导航。
5. 对解释性实验、研究示例和实际可执行交易分别标注，严禁把自动化检查通过表述为经济价值已证实。
