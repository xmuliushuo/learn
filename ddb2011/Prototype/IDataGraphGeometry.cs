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
    interface IDataGraphGeometry
    {
        /// <summary>
        /// 从GraphManager构造数据图几何表示
        /// </summary>
        /// <param name="gm">GraphManager数据图</param>
        /// <param name="radius">每個點的半徑</param>
        /// <param name="fontsize">文字大小</param>
        /// <returns>返回几何图形集</returns>
        PathGeometry[] ConstructGeometry(GraphManager gm, int radius, int fontsize);

        /// <summary>
        /// DDB2011专用简化绘图
        /// </summary>
        /// <param name="gm">GraphManager数据图</param>
        /// <param name="radius">每個點的半徑</param>
        /// <param name="fontsize">文字大小</param>
        /// <returns>返回几何图形集</returns>
        PathGeometry[] ConstructGeometrySimple(GraphManager gm, int radius, int fontsize);

        /// <summary>
        /// SteinerTree类专用演示绘图（将和数据图重合）
        /// </summary>
        /// <param name="st">最简查询子树</param>
        /// <param name="gm">GraphManager数据图</param>
        /// <param name="radius">每個點的半徑</param>
        /// <param name="fontsize">文字大小</param>
        /// <returns>几何图形</returns>
        PathGeometry[] ConstructSteinterExactGeometry(SteinerTree st, GraphManager gm, int radius, int fontsize);

        /// <summary>
        /// SteinerTree类专用单独绘图
        /// </summary>
        /// <param name="st">最简查询子树</param>
        /// <param name="gm">GraphManager数据图</param>
        /// <param name="radius">每個點的半徑</param>
        /// <param name="fontsize">文字大小</param>
        /// <returns>几何图形</returns>
        PathGeometry[] ConstructSteinterGeometry(SteinerTree st, GraphManager gm, int radius, int fontsize);
    }
}
