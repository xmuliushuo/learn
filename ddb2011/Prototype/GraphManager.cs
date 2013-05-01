using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace DDB2011Prototype
{
    
    /// <summary>
    /// 此类用于保存图的各种数据结构以及算法
    /// </summary>
    public class GraphManager
    {
        /// <summary>
        /// 此乃保存所有节点关系的矩阵，即图的矩阵
        /// </summary>
        public int[,] graph;
        
        /// <summary>
        /// 点的个数
        /// </summary>
        public int nodeNum;

        /// <summary>
        /// 根据节点的主键得到节点的索引
        /// </summary>
        public Dictionary<string, int> dicKey;

        /// <summary>
        /// 根据节点的索引得到节点的主键
        /// </summary>
        public Array arrayKey;
        
        /// <summary>
        /// 绘图时用于记录每一个点的位置
        /// </summary>
        public Array arrayPos;
        
        /// <summary>
        /// 构造函数
        /// </summary>
        /// <param name="nodeNum">节点数目</param>
        public GraphManager(int nodeNum)
        {
            graph = new int[nodeNum, nodeNum];
            for (int i = 0; i < nodeNum; i++)
            {
                for (int j = 0; j < nodeNum; j++)
                {
                    graph[i, j] = Util.INFINITE;
                }
            }
            this.nodeNum = nodeNum;
            dicKey = new Dictionary<string, int>();
            arrayKey = Array.CreateInstance(typeof(string), nodeNum);
            arrayPos = Array.CreateInstance(typeof(Point), nodeNum);
        }

    }
}
