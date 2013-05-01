using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DDB2011Prototype
{
    /// <summary>
    /// 边结构
    /// </summary>
    public struct edge
    {
        /// <summary>
        /// 起点
        /// </summary>
        public int x;
        /// <summary>
        /// 终点
        /// </summary>
        public int y;
    }
    /// <summary>
    /// 此类说明方法：
    ///     // 这里获得用户输入的关键字
    ///     char[] blank = { ' ' };
    ///     string[] keyword = textBoxKeyword.Text.Split(blank, StringSplitOptions.RemoveEmptyEntries);
    ///     
    ///     // 查询数据库
    ///     DBManager dbm = new DBManager();
    ///     int count = dbm.getNodeNum();
    ///     GraphManager gm = new GraphManager(count);
    ///     dbm.initGraph(gm);
    ///
    ///     SteinerTree st = new SteinerTree(gm, dbm, keyword);
    ///     st.execute();
    ///        
    /// 绘图：
    ///     foreach(int i in st.steinerTreeNode)
    ///     {
    ///         //i即要绘制的节点
    ///     }
    ///     //画线
    ///     foreach (edge i in st.steinerTreeEdge)
    ///     {
    ///         dc.DrawLine(bluePen, (Point)gm.arrayPos.GetValue(i.x), (Point)gm.arrayPos.GetValue(i.y));
    ///     }    
    /// </summary>
    public class SteinerTree
    {
        /// <summary>
        /// 此变量用于保存节点是否被取出过
        /// </summary>
        bool[] isReached;

        /// <summary>
        /// 关键词签到集合
        /// </summary>
        Dictionary<int, List<int>> dictVL;

        /// <summary>
        /// 关键词节点签到集合
        /// </summary>
        Dictionary<int, List<int>> dictVL2;

        /// <summary>
        /// 根据源节点序号获得Dijkstra迭代器
        /// </summary>
        Dictionary<int, Dijkstra> dictDijkstra;

        /// <summary>
        /// Dijkstra迭代器集合
        /// </summary>
        List<Dijkstra> iterator;

        /// <summary>
        /// 所有的关键字节点集合
        /// </summary>
        List<int> keywordIndex;
        List<List<int>> keywordSet;  

        /// <summary>
        /// steiner树的节点集合
        /// </summary>
        public List<int> steinerTreeNode;
        /// <summary>
        /// steiner树的边集合
        /// </summary>
        public List<edge> steinerTreeEdge;

        /// <summary>
        /// 构造函数
        /// </summary>
        /// <param name="gm">图结构</param>
        /// <param name="dbm">数据库管理类</param>
        /// <param name="keyword">关键词集合</param>
        public SteinerTree(GraphManager gm, DBManager dbm, string[] keyword)
        {
            //this.gm = gm;
            isReached = new bool[gm.nodeNum];
            for (int i = 0; i < isReached.Length; i++)
            {
                isReached[i] = false;
            }
            dictVL = new Dictionary<int, List<int>>();
            dictVL2 = new Dictionary<int, List<int>>();
            dictDijkstra = new Dictionary<int, Dijkstra>();
            iterator = new List<Dijkstra>();
            keywordSet = new List<List<int>>();
            keywordIndex = dbm.getKeywordSets(keyword, gm, keywordSet);
            foreach (int item in keywordIndex)
            {
                Dijkstra dkInterator = new Dijkstra(gm, item);
                dictDijkstra.Add(item, dkInterator);
                iterator.Add(dkInterator);
                dkInterator.execute();
                dkInterator.getNextNode();
            }
            steinerTreeNode = new List<int>();
            steinerTreeEdge = new List<edge>();
        }

        /// <summary>
        /// 算法执行
        /// </summary>
        public void execute()
        {

            int min = Util.INFINITE;
            int node = -1;
            int keywordNode = 0;
            int iteratorIndex = 0;
            int root = 0;
            while (true)
            {
                min = Util.INFINITE;
                for (int i = 0; i < iterator.Count; i++)
                {
                    if (iterator[i].nextDistance < min)
                    {
                        keywordNode = iterator[i].v0;
                        min = iterator[i].nextDistance;
                        node = iterator[i].nextV;
                        iteratorIndex = i;
                    }
                }
                // 没有关键词节点
                if (node == -1)
                {
                    return;
                }
                if (isReached[node] == false)
                {
                    // 节点没被取出过
                    isReached[node] = true;
                    List<int> setList = new List<int>();
                    List<int> nodeList = new List<int>();
                    for (int i = 0; i < keywordSet.Count; i++)
                    {
                        if (keywordSet[i].Contains(keywordNode))
                        {
                            setList.Add(i);
                            if (!nodeList.Contains(keywordNode))
                            {
                                nodeList.Add(keywordNode);
                            }
                        }
                    }

                    dictVL.Add(node, setList);
                    dictVL2.Add(node, nodeList);
                    if (dictVL[node].Count == keywordSet.Count)
                    {
                        // 此乃根节点
                        root = node;
                        //Console.WriteLine("root");
                        //Console.WriteLine(node);
                        break;
                    }
                }
                else
                {
                    // 节点被取出过
                    for (int i = 0; i < keywordSet.Count; i++)
                    {
                        if (keywordSet[i].Contains(keywordNode) && !dictVL[node].Contains(i))
                        {
                            dictVL[node].Add(i);
                            if (!dictVL2[node].Contains(keywordNode))
                            {
                                dictVL2[node].Add(keywordNode);
                            }
                        }
                    }
                    if (dictVL[node].Count == keywordSet.Count)
                    {
                        // 此乃根节点
                        root = node;
                        break;
                    }
                }
                // 此时不存在这样的根节点，算法直接退出
                if (iterator.Count == 0)
                {
                    return;
                }
                iterator[iteratorIndex].getNextNode();
                int nextDistance = iterator[iteratorIndex].nextDistance;
                if (nextDistance == Util.INFINITE)
                {
                    iterator.RemoveAt(iteratorIndex);
                }
            } // while

            for (int i = 0; i < dictVL2[root].Count; i++)
            {
                Console.WriteLine(dictVL2[root][i]);
                // 获取steiner树
                int source = root;
                if (!steinerTreeNode.Contains(source))
                {

                    steinerTreeNode.Add(source);
                }
                while (source != dictVL2[root][i])
                {
                    edge myedge = new edge();
                    myedge.x = source;
                    myedge.y = dictDijkstra[dictVL2[root][i]].pre[source];
                    steinerTreeEdge.Add(myedge);
                    if (!steinerTreeNode.Contains(source))
                    {
                        steinerTreeNode.Add(source);
                    }
                    if (!steinerTreeNode.Contains(dictDijkstra[dictVL2[root][i]].pre[source]))
                    {
                        steinerTreeNode.Add(dictDijkstra[dictVL2[root][i]].pre[source]);
                    }
                    source = dictDijkstra[dictVL2[root][i]].pre[source];

                }
            }
            steinerTreeNode.Sort();
        }
    }
}
