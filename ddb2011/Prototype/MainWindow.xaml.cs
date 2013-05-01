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
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        //绘图常量
        #region Constant
        const int RADIUS = 2;
        const int WEIGHT = 1;
        const int DIST = 80;
        const int THICKNESS = 2;
        const int FONTSIZE = 13;
        #endregion

        DBManager dbm;                  //访问数据库
        GraphManager gm;                //获得数据图
        SteinerTree st;                 //
        DataGraphGeometry datagraph;    //数据图的几何图形

        int count;
        char[] blank = { ' ' };
        //int buttonMode;
        //const int BUTTON_MODE = 2;      //主按钮模式切换 0画数据图 1画Steiner树

        /// <summary>
        /// 主窗口
        /// </summary>
        public MainWindow()
        {
            InitializeComponent();

            // 初始化
            dbm = new DBManager();
            count = dbm.getNodeNum();
            gm = new GraphManager(count);
            dbm.initGraph(gm);
            datagraph = new DataGraphGeometry();
            textBlockInfo.Text = "Ready.";          //状态信息显示
        }

        /// <summary>
        /// 单击显示数据图按钮的相应事件
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonStartDraw_Click(object sender, RoutedEventArgs e)
        {
            DateTime startTime = DateTime.Now;      //求开始时间
            textBlockInfo.Text = "Start Time:" + startTime.ToString("HH:mm:ss.fff");
            canvas.Children.Clear();                //清理左繪圖區
            imageDisplay(datagraph.ConstructGeometrySimple(gm, 4, FONTSIZE), Brushes.Black, canvas, false, 1, false);
            DateTime endTime = DateTime.Now;        //求结束时间
            textBlockInfo.Text += "\t" + "End Time" + endTime.ToString("HH:mm:ss.fff");
            textBlockInfo2.Text = "Duration: " + (endTime - startTime).Milliseconds.ToString() + "ms";  //转换为ms
        }

        /// <summary>
        /// 单击搜索按钮的响应
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void buttonGO_Click(object sender, RoutedEventArgs e)
        {
            canvasTree.Children.Clear();            //右繪圖區清理
            canvas.Children.Clear();                //清理左繪圖區
            string[] keyword = textBoxSearch.Text.Split(blank, StringSplitOptions.RemoveEmptyEntries);
            DateTime startTime = DateTime.Now;      //求开始时间
            textBlockInfo.Text = "Start Time:" + startTime.ToString("HH:mm:ss.fff");
            st = new SteinerTree(gm, dbm, keyword);
            st.execute();
            imageDisplay(datagraph.ConstructGeometrySimple(gm, 4, FONTSIZE), Brushes.Black, canvas, false, 1, false);
            imageDisplay(datagraph.ConstructSteinterGeometry(st, gm, 5, FONTSIZE), Brushes.Gold, canvas, false, 2, false);
            imageDisplay(datagraph.ConstructSteinterGeometry(st, gm, 5, FONTSIZE), Brushes.Gold, canvasTree, true, 1, true);
            DateTime endTime = DateTime.Now;        //求结束时间
            textBlockInfo.Text += "\t" + "End Time" + endTime.ToString("HH:mm:ss.fff");
            textBlockInfo2.Text = "Duration: " + (endTime - startTime).Milliseconds.ToString() + "ms";  //转换为ms
            //buttonMode = 0;                         //改變按鈕形態
            //buttonGO.Content = "GRAPH";             //改变文字
        }

        /// <summary>
        /// 将数据图绘制到屏幕
        /// </summary>
        /// <param name="dataGraph">GraphManager数据图</param>
        /// <param name="brush">笔刷 用于调节颜色</param>
        /// <param name="grid">Grid 用于指定绘图板</param>
        /// <param name="uniform">用于指定是否自动缩放</param>
        /// <param name="lineThickness">用于指定线条宽度</param>
        /// <param name="shapeStroke">用于指定是否绘制边线</param>
        private void imageDisplay(PathGeometry[] dataGraph, Brush brush, Grid grid,
            bool uniform, int lineThickness, bool shapeStroke)
        {
            Path shapes = new Path();            
            shapes.Data = dataGraph[0];

            Path lines = new Path();
            lines.Data = dataGraph[1];

            shapes.Fill = brush;
            lines.Stroke = brush;
            lines.StrokeThickness = lineThickness;
            if (uniform)
            {
                dataGraph[0].AddGeometry(dataGraph[1]);
                shapes.Stroke = brush;
                shapes.StrokeThickness = lineThickness;
                shapes.Data = dataGraph[0];
                shapes.Stretch = Stretch.Uniform;
                grid.Children.Add(shapes);
            }
            else
            {
                if (shapeStroke)
                {
                    shapes.Stroke = brush;
                    shapes.StrokeThickness = lineThickness;
                }
                else
                {
                    shapes.StrokeThickness = 0;
                }
                grid.Children.Add(shapes);
                grid.Children.Add(lines);
            }
        }

    }
}
