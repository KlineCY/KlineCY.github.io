"""Parse all tutorial snippets, execute lightweight deterministic examples."""
import ast
import json
import pathlib
import subprocess
import sys
import unittest

ROOT=pathlib.Path(__file__).resolve().parent
FILES=["lessons-classic.json","lessons-deep.json","lessons-rl-finance.json","lessons-applied.json"]
LESSONS=[x for f in FILES for x in json.loads((ROOT/f).read_text(encoding="utf-8"))["lessons"]]
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
    "因子中性化","随机种子","数据延迟与可用时间"
}
class MLExamplesTest(unittest.TestCase):
    def test_all_code_compiles(self):
        for item in LESSONS:
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
        self.assertEqual(len(LESSONS),60)
        self.assertEqual(len({x["node_id"] for x in LESSONS}),60)
if __name__=="__main__":
    unittest.main()
