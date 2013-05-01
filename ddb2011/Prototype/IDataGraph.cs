using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections;
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
    interface IDataGraph<T>
    {
        void Initialize();

        void DrawGraph(Color c);
        void Mark(T[] seq, Color c);
        void Mark(T n, Color c);

        DataGraphNode[] GetNodeList();
        DataGraphNode GetNodeByID(int id);
        DataGraphNode GetNodeByName(int name);
        void InsertNode(T n);
        void RemoveNode(T n);
    }

    interface IDataGraphNode
    {
        int[] GetNeighbors();

        ArrayList GetEdges();
        void AddEdge(DataGraphEdge e);
        DataGraphEdge GetEdgeByDest(int id);
        DataGraphEdge GetEdgeByName(string name);
    }
}
