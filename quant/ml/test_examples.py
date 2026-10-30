"""Parse all tutorial snippets, execute lightweight deterministic examples."""
import ast
import json
import pathlib
import subprocess
import sys
import unittest

ROOT=pathlib.Path(__file__).resolve().parent
MANIFEST=json.loads((ROOT/"manifest.json").read_text(encoding="utf-8"))
FILES=MANIFEST["lesson_files"]
LESSONS=[x for f in FILES for x in json.loads((ROOT/f).read_text(encoding="utf-8"))["lessons"]]
NOTES=[x for f in MANIFEST.get("note_files",[]) for x in json.loads((ROOT/f).read_text(encoding="utf-8"))["notes"]]
EXAMPLES=[x for file in sorted(ROOT.glob("lessons-full-*.json")) for x in json.loads(file.read_text(encoding="utf-8"))["lessons"]]
SMOKE={
    "经验风险最小化ERM","偏差方差权衡","数据泄漏","标准化StandardScaler",
    "PCA","线性回归与OLS","Ridge回归","LASSO回归","Logistic回归",
    "SVM软间隔","ROC曲线和AUC","分类回归树CART",
    "随机森林Random Forest","梯度提升Gradient Boosting","KMeans",
    "Markov Decision Process","Bellman最优方程","值迭代Value Iteration",
    "TD(0)","Q-learning","滞后特征","滚动验证","GARCH",
    "横截面RankIC","Purged交叉验证","净成本回测",
    "贝叶斯定理","EM算法","隐马尔可夫模型HMM","ROC-AUC","Bootstrap","SHAP",
    "潜在结果","Double Machine Learning","三重障碍标签Triple Barrier",
    "因子中性化","随机种子","数据延迟与可用时间",
    "监督学习","无监督学习","训练集验证集测试集","时间序列划分",
    "ndarray形状dtype","广播规则","GroupBy与窗口","scikit-learn Pipeline",
    "缺失值填补","One-Hot编码","稳健缩放RobustScaler","互信息",
    "混淆矩阵","PR曲线和AP","基尼不纯度","袋外估计OOB",
    "DBSCAN","Isolation Forest","MAP估计","MAE与RMSE","Brier Score",
    "Naive Forecast","实验跟踪MLflow","状态价值V","动作价值Q",
    "策略评估","策略迭代Policy Iteration","SARSA","ε-greedy",
    "Direct vs Recursive","Embargo","分层组合","滑点与冲击模型",
    "节点边邻接矩阵","图Laplace","DAG因果图","倾向得分",
    "逆概率加权IPW","差分中的差分","Cross-Fitting","频谱图",
    "数据延迟与可用时间","Alpha和Beta估计","Elastic Net",
    "分位数回归","残差分析","概率校准",
    "Permutation Importance","RAG检索增强","窗口滚动特征",
    "最大似然MLE","指数族","共轭先验","后验预测",
    "高斯混合模型GMM","状态空间模型","Kalman滤波",
    "Metropolis-Hastings","Conformal Prediction","Mini-batch SGD",
    "Nesterov加速","RMSProp","Newton法","近端梯度",
    "N-gram","新闻情绪得分","披露时点对齐","位置编码RoPE",
    "Top-k与Top-p","文本Embedding","图像归一化","图像增强",
    "Dice/IoU","生成数据风险检验",
    "复权OHLCV","PIT因子","未来收益标签","动量与均值回复",
    "多重检验修正","风险平价","均值方差组合","Black-Litterman",
    "R²","数据漂移","OOD检测","测试与CI","线上特征一致性",
    "k近邻kNN","高斯朴素贝叶斯","线性判别分析LDA",
    "熵和信息增益","层次聚类","嵌套交叉验证","实验假设和消融",
    "谱聚类","聚类稳定性","核PCA","Local Outlier Factor","One-Class SVM",
    "粒子滤波","变分推断VI","Hamiltonian Monte Carlo","AdaGrad",
    "投影梯度","BFGS/L-BFGS","避免图像数据泄漏","跨ETF泛化",
    "Importance Sampling","UCB","最优停止","事件采样CUSUM",
    "样本唯一性","Deflated Sharpe","Probability of Backtest Overfitting",
    "工具变量","断点回归","事件研究","Beam Search","GloVe",
    "Tokenization BPE","重排序Reranking","特征存储Feature Store",
    "漂移与再训练","Shadow Deployment","回滚与审计"
}
class MLExamplesTest(unittest.TestCase):
    def test_all_code_compiles(self):
        for item in LESSONS+NOTES+EXAMPLES:
            with self.subTest(lesson=item["title"]):
                compile(item["code"],item["title"],"exec",dont_inherit=True)
    def test_selected_snippets_run(self):
        for item in LESSONS:
            if item["title"] not in SMOKE:continue
            with self.subTest(lesson=item["title"]):
                result=subprocess.run(
                    [sys.executable,"-c",item["code"]],
                    capture_output=True,text=True,timeout=40,
                    env=None
                )
                self.assertEqual(result.returncode,0,
                    f"{item['title']}:\n{result.stderr[-2000:]}")
    def test_no_duplicate_tutorials(self):
        self.assertEqual(len(LESSONS),MANIFEST["total_lessons"])
        self.assertEqual(len({x["node_id"] for x in LESSONS}),len(LESSONS))
        self.assertEqual(len(NOTES),MANIFEST["notes_count"])
        active_notes=[n for n in NOTES if n["node_id"] not in {x["node_id"] for x in LESSONS}]
        self.assertEqual(len({x["node_id"] for x in LESSONS+active_notes}),720)
        self.assertEqual(len(LESSONS)+len(active_notes),720)
if __name__=="__main__":
    unittest.main()
