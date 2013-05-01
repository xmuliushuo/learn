using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace DDB2011Prototype
{
    /// <summary>
    /// 执行Dijkstra算法
    /// </summary>
    class Dijkstra
    {
        GraphManager G;

        /// <summary>
        /// 源节点
        /// </summary>
        public int v0;

        /// <summary>
        /// 记录最短路径中该节点的上一个节点号
        /// </summary>
        public int[] pre;

        // 算法执行过程中的辅助变量
        int[] D;
        bool[] final;
        int count;

        /// <summary>
        /// 迭代器得到的下一个节点
        /// </summary>
        public int nextV;

        /// <summary>
        /// 迭代器得到下个节点到源节点的距离
        /// </summary>
        public int nextDistance;
        
        /// <summary>
        /// 构造函数
        /// </summary>
        /// <param name="gm"></param>
        /// <param name="v0"></param>
        public Dijkstra(GraphManager gm, int v0)
        {
            G = gm;
            final = new bool[gm.nodeNum];
            D = new int[gm.nodeNum];
            this.v0 = v0;
            count = 0;
            pre = new int[gm.nodeNum];
        }

        /// <summary>
        /// 算法初始化执行
        /// </summary>
        public void execute()
        {
            int v;
            for (v = 0; v < G.nodeNum; v++)
            {
                final[v] = false;
                D[v] = G.graph[v0, v];
                for (int w = 0; w < G.nodeNum; w++)
                {
                    pre[w] = v0;
                }
            }
        }

        /// <summary>
        /// 算法的迭代执行
        /// </summary>
        /// <returns>下一个节点编号</returns>
        public int getNextNode()
        {
            if (final[v0] != true)
            {
                D[v0] = 0;
                final[v0] = true;
                nextV = v0;
                nextDistance = D[v0];
                count++;
                return v0;
            }
            else
            {
                int min = Util.INFINITE;
                int v = 0;
                for (int w = 0; w < G.nodeNum; w++)
                {
                    if (!final[w])
                    {
                        if (D[w] < min)
                        {
                            v = w;
                            min = D[w];
                        }
                    }
                }
                final[v] = true;
                for (int w = 0; w < G.nodeNum; w++)
                {
                    if (!final[w] && G.graph[v, w] != Util.INFINITE && (min + G.graph[v, w]) < D[w])
                    {
                        D[w] = min + G.graph[v, w];
                        pre[w] = v;
                    }
                }
                count++;
                if (count > G.nodeNum)
                {
                    nextDistance = Util.INFINITE;
                }
                else
                {
                    nextV = v;
                    nextDistance = D[v];
                }
                return v;
            }
        }
    }
}
