using System;
using System.Collections;
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
    /// Class for DataGraph, basic attributes and ArrayList for nodeList are set, basic operation provided in IDataGraph
    /// ============================================================================
    /// WARNING: Unique test for each element not implemented
    /// ============================================================================
    /// 
    /// </summary>
    public class DataGraphGeometry : IDataGraphGeometry, System.Collections.IEnumerable
    {
        #region constant
        const int H_DIST = 50, V_DIST = 50; //定义图中两点间的水平垂直间隔
        const int GRAPH_RADIUS = 3, TREE_RADIUS = 4;
        const int H_GAP = 15, V_GAP = 10;    //定义字体位置
        const string FONT_URI = @"C:\WINDOWS\Fonts\TIMES.TTF";
        const int FONT_SIZE = 11;

        #endregion

        /// <summary>
        /// 图的名字
        /// </summary>
        public string graphName { set; get; }
        /// <summary>
        /// 图的唯一编号
        /// </summary>
        public int graphID { set; get; }
        private int nodeNo { set; get; }     //当前表中node序号
        private int tableNo { set; get; }    //表数（序号）

        /// <summary>
        /// 最终返回用到的几何图形组
        /// </summary>
        public PathGeometry dataGraphShape, dataGraphLine, steinertreeShape, steinertreeLine;      //最终返回用
        /// <summary>
        /// ConstructGeometry用到的边集
        /// </summary>
        public GeometryGroup edges;         //边集
        /// <summary>
        /// 按表分类的点集，将点集合成GeometryGroup放入ArrayList
        /// </summary>
        public ArrayList tables;            //按表分类的点集，将点集合成GeometryGroup放入ArrayList
        int[,] tableIndex;                  //存储每个点对应的表和序号 0是表 1是序号
        /// <summary>
        /// 用于存放和表对应的key值，ConstructGeometry专用
        /// </summary>
        public ArrayList keys;              //用于存放和表对应的key值

        /// <summary>
        /// 未初始化严禁使用
        /// </summary>
        public DataGraphGeometry()
        {
            graphID = -1;
            nodeNo = tableNo = 0;
            graphName = null;
            edges = new GeometryGroup();
            tables = new ArrayList();
            dataGraphShape = new PathGeometry();
            dataGraphLine = new PathGeometry();
            steinertreeShape = new PathGeometry();
            steinertreeLine = new PathGeometry();
            tableIndex = null;
        }

        /// <summary>
        /// Not Implemented Yet
        /// </summary>
        /// <returns></returns>
        public IEnumerator GetEnumerator()
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 从数据图构造几何图形
        /// </summary>
        /// <param name="gm">数据图GraphManager</param>
        /// <param name="fontsize">字体大小</param>
        /// <param name="radius">点半径</param>
        /// <returns>返回几何图形集</returns>
        public PathGeometry[] ConstructGeometry(GraphManager gm, int radius, int fontsize)
        {
            tableIndex = new int[gm.nodeNum, 2];
            string currentKey = null;                               //获得第一个主键
            nodeNo = tableNo = 0;
            GeometryGroup[] group = new GeometryGroup[1];           //遍历用临时group，只有一个组
            for (int i = 0; i < gm.nodeNum; i++)
            {                    //遍历所有主键
                if (gm.arrayKey.GetValue(i).Equals(currentKey))
                {     //仍然是相同key
                    tables.CopyTo(tables.Count - 1, group, 0, 1);   //取出最后一个，插入点
                    group[0].Children.Add(new EllipseGeometry(new Point(tableNo * H_DIST, nodeNo * H_DIST), radius, radius));
                    tableIndex[i, 1] = nodeNo++;                    //计数
                }
                else
                {
                    currentKey = (string)gm.arrayKey.GetValue(i);   //更换key
                    keys.Add(currentKey);                           //计入key
                    nodeNo = 0;
                    tableIndex[i, 1] = nodeNo;                      //记录node编号
                    tables.Add(group);                              //放入旧group
                    tableIndex[i, 0] = tableNo++;                   //记录新表编号
                    group = new GeometryGroup[1];                   //建立新group，插入下一组的第一个node，group只有一个元素
                    group[0].Children.Add(new EllipseGeometry(new Point(tableNo * H_DIST, nodeNo * H_DIST), radius, radius));
                }
            }

            EllipseGeometry[] cp = new EllipseGeometry[1];          //临时用ellipse，只有一个元素
            for (int i = 0; i < gm.nodeNum; i++)
            {
                for (int j = 0; j < gm.nodeNum; j++)
                {
                    #region DEBUG
                    Console.Write(gm.graph[i, j]);
                    Console.Write(" ");
                    #endregion
                    if (gm.graph[i, j] == 1)
                    {
                        tables.CopyTo(tableIndex[i, 0], group, 0, 1);//找到对应表
                        group[0].Children.CopyTo(cp, tableIndex[i, 1]);//取出对应形状
                        Point s = cp[0].Center;                     //取点
                        tables.CopyTo(tableIndex[j, 0], group, 0, 1);
                        group[0].Children.CopyTo(cp, tableIndex[j, 1]);
                        Point d = cp[0].Center;
                        edges.Children.Add(new LineGeometry(s, d));
                    }
                }
            }

            for (int i = 0; i < tables.Count; i++)
            {
                tables.CopyTo(i, group, 0, 1);                  //提取第i个表的group
                dataGraphShape.AddGeometry(group[0]);                //放入总图
            }
            dataGraphLine.AddGeometry(edges);

            return new PathGeometry[] { dataGraphShape, dataGraphLine };
        }

        /// <summary>
        /// 从数据图构造几何图形
        /// </summary>
        /// <param name="gm">数据图GraphManager</param>
        /// <param name="radius">每個點的半徑</param>
        /// <param name="fontsize">字体大小</param>
        /// <returns></returns>
        public PathGeometry[] ConstructGeometrySimple(GraphManager gm, int radius, int fontsize)
        {
            #region 画点
            int PCount = 0;
            int ACount = 0;
            int WCount = 0;
            int CCount = 0;
            dataGraphShape = new PathGeometry();
            dataGraphLine = new PathGeometry();

            for (int i = 0; i < gm.nodeNum; i++)
            {
                if (gm.arrayKey.GetValue(i).ToString()[0] == 't')
                {
                    Point centerP = new Point(H_DIST * PCount + H_DIST, 2 * V_DIST);
                    dataGraphShape.AddGeometry(new EllipseGeometry(centerP, radius, radius));
                    gm.arrayPos.SetValue(new Point(H_DIST * PCount + H_DIST, 2 * V_DIST), i);
                    Point ptextP = new Point(H_DIST * PCount + H_DIST - H_GAP, 2 * V_DIST - V_GAP);
                    dataGraphShape.AddGeometry(DataGraphPrintText(gm.arrayKey.GetValue(i).ToString(), ptextP, fontsize).BuildGeometry());
                    PCount++;
                }
                if (gm.arrayKey.GetValue(i).ToString()[0] == 'a')
                {
                    Point centerA = new Point(H_DIST * ACount + H_DIST, 4 * V_DIST);
                    dataGraphShape.AddGeometry(new EllipseGeometry(centerA, radius, radius));
                    gm.arrayPos.SetValue(new Point(H_DIST * ACount + H_DIST, 4 * V_DIST), i);
                    Point ptextA = new Point(H_DIST * ACount + H_DIST - H_GAP, 4 * V_DIST - V_GAP);
                    dataGraphShape.AddGeometry(DataGraphPrintText(gm.arrayKey.GetValue(i).ToString(), ptextA, fontsize).BuildGeometry());
                    ACount++;
                }
                if (gm.arrayKey.GetValue(i).ToString()[0] == 'C')
                {
                    Point centerC = new Point(H_DIST * CCount + H_DIST, V_DIST);
                    dataGraphShape.AddGeometry(new EllipseGeometry(centerC, radius, radius));
                    gm.arrayPos.SetValue(new Point(H_DIST * CCount + H_DIST, V_DIST), i);
                    Point ptextC = new Point(H_DIST * CCount + H_DIST - H_GAP, V_DIST - V_GAP);
                    dataGraphShape.AddGeometry(DataGraphPrintText(gm.arrayKey.GetValue(i).ToString(), ptextC, fontsize).BuildGeometry());
                    CCount++;
                }
                if (gm.arrayKey.GetValue(i).ToString()[0] == 'W')
                {
                    Point centerW = new Point(H_DIST * WCount + H_DIST, 3 * V_DIST);
                    dataGraphShape.AddGeometry(new EllipseGeometry(centerW, radius, radius));
                    gm.arrayPos.SetValue(new Point(H_DIST * WCount + H_DIST, 3 * V_DIST), i);
                    Point ptextW = new Point(H_DIST * WCount + H_DIST - H_GAP, 3 * V_DIST - V_GAP);
                    dataGraphShape.AddGeometry(DataGraphPrintText(gm.arrayKey.GetValue(i).ToString(), ptextW, fontsize).BuildGeometry());
                    WCount++;
                }
            }
            #endregion

            #region 画线
            for (int i = 0; i < gm.nodeNum; i++)
            {
                for (int j = 0; j < i; j++)
                {
                    if (gm.graph[i, j] == 1)
                    {
                        dataGraphLine.AddGeometry(new LineGeometry((Point)gm.arrayPos.GetValue(i), (Point)gm.arrayPos.GetValue(j)));
                    }
                }
            }
            #endregion

            return new PathGeometry[] { dataGraphShape, dataGraphLine };
        }

        /// <summary>
        /// 构造steiner树的几何图形
        /// </summary>
        /// <param name="st">steinertree类</param>
        /// <param name="gm">GraphManager类</param>
        /// <param name="radius">每个点的半径</param>
        /// <param name="fontsize">每个点说明文字的大小</param>
        /// <returns>几何图形组。[0]为点集，[1]为边集</returns>
        public PathGeometry[] ConstructSteinterGeometry(SteinerTree st, GraphManager gm, int radius, int fontsize)
        {
            steinertreeShape = new PathGeometry();
            steinertreeLine = new PathGeometry();
            //i即要绘制的节点
            foreach (int i in st.steinerTreeNode)
            {
                steinertreeShape.AddGeometry(new EllipseGeometry((Point)gm.arrayPos.GetValue(i), radius, radius));
                Point ptext = new Point(((Point)gm.arrayPos.GetValue(i)).X - H_GAP, ((Point)gm.arrayPos.GetValue(i)).Y - V_GAP);
                steinertreeShape.AddGeometry(DataGraphPrintText(gm.arrayKey.GetValue(i).ToString(), ptext, fontsize).BuildGeometry());
            }
            //画线
            foreach (edge i in st.steinerTreeEdge)
            {
                steinertreeLine.AddGeometry(new LineGeometry((Point)gm.arrayPos.GetValue(i.x), (Point)gm.arrayPos.GetValue(i.y)));
            }

            return new PathGeometry[] { steinertreeShape, steinertreeLine };
        }

        /// <summary>
        /// not implemented yet
        /// </summary>
        /// <param name="st"></param>
        /// <param name="gm"></param>
        /// <param name="radius"></param>
        /// <param name="fontsize"></param>
        /// <returns></returns>
        public PathGeometry[] ConstructSteinterExactGeometry(SteinerTree st, GraphManager gm, int radius, int fontsize)
        {
            throw new NotImplementedException();
        }

        /// <summary>
        /// 在指定点画出文字
        /// </summary>
        /// <param name="s">所画的文字内容</param>
        /// <param name="p">文字坐标</param>
        /// <param name="fontsize">文字大小</param>
        /// <returns>返回文字形状</returns>
        private GlyphRun DataGraphPrintText(string s, Point p, int fontsize)
        {
            Glyphs text = new Glyphs();
            text.OriginX = p.X;
            text.OriginY = p.Y;
            text.UnicodeString = s;
            text.FontUri = new Uri(FONT_URI);
            text.FontRenderingEmSize = fontsize;
            return text.ToGlyphRun();
        }
    }
}
