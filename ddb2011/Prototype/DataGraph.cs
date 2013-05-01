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
    // Class for DataGraph, basic attributes and ArrayList for nodeList are set, basic operation provided in IDataGraph
    // ============================================================================
    // WARNING: Unique test for each element not implemented
    // ============================================================================
    public class DataGraph : IDataGraph<DataGraphNode>, System.Collections.IEnumerable
    {
        public int graphID { set; get; }
        public int nodeNum { set; get; }
        public string graphName { set; get; }
        public ArrayList nodeList;

        public DataGraph()
        {
            graphID = -1;
            nodeNum = 0;
            graphName = null;
            nodeList = new ArrayList();
        }

        // Construct an initial graph, can applied to many imported data formats.
        public void Initialize()
        {

        }

        public void DrawGraph(Color c)
        {
            throw new NotImplementedException();
        }

        public void InsertNode(DataGraphNode n)
        {
            nodeList.Add(n);
        }

        public void RemoveNode(DataGraphNode n)
        {
            nodeList.RemoveAt(nodeList.IndexOf(n));
        }

        public void Mark(DataGraphNode[] seq, Color c)
        {
            throw new NotImplementedException();
        }

        public void Mark(DataGraphNode n, Color c)
        {
            throw new NotImplementedException();
        }

        public DataGraphNode[] GetNodeList()
        {
            DataGraphNode[] nList = new DataGraphNode[nodeList.Count];
            return nList;
        }

        public DataGraphNode GetNodeByID(int id)
        {
            DataGraphNode[] nList = GetNodeList();
            nodeList.CopyTo(nList);
            for (int i = 0; i < nList.Length; i++)
            {
                if (nList[i].nodeID == id)
                    return nList[i];
            }
            return null;
        }

        public DataGraphNode GetNodeByName(int name)
        {
            DataGraphNode[] nList = new DataGraphNode[nodeList.Count];
            nodeList.CopyTo(nList);
            for (int i = 0; i < nodeList.Count; i++)
            {
                if (nList[i].nodeName.Equals(name))
                    return nList[i];    
            }
            return null;
        }

        public IEnumerator GetEnumerator()
        {
            for (int i = 0; i < nodeList.Count; i++)
            {
                yield return nodeList[i];
            }
        }
    }

    // Class for DataGraphNode, basic attributes and ArrayList for edgeList are set, turn edges to neighbors if necessary, 
    // basic operation provided in IDataGraphNode
    // ============================================================================
    // WARNING: Unique test for each element not implemented
    // ============================================================================
    public class DataGraphNode : IDataGraphNode, System.Collections.IEnumerable
    {
        public int edgeNum { set; get; }
        public int nodeID { get; set; }
        public string nodeName { get; set; }
        public ArrayList edges;

        public DataGraphNode()
        {
            edgeNum = 0;
            nodeID = -1;
            nodeName = null;
            edges = new ArrayList();
        }

        public int[] GetNeighbors()
        {
            DataGraphEdge[] eList = new DataGraphEdge[edges.Count];
            int[] destList = new int[edges.Count];
            edges.CopyTo(eList);
            for (int i = 0; i < edges.Count; i++)
            {
                destList[i] = eList[i].destID;
            }
            return destList;
        }

        public void AddEdge(DataGraphEdge e)
        {
            edges.Add(e);
        }

        public ArrayList GetEdges()
        {
            return edges;
        }

        public DataGraphEdge GetEdgeByDest(int dest)
        {
            DataGraphEdge[] eList = new DataGraphEdge[edges.Count];
            edges.CopyTo(eList);
            for (int i = 0; i < edges.Count; i++)
            {
                if (eList[i].destID == dest)
                    return eList[i];                    
            }
            return null;    // not found
        }

        public DataGraphEdge GetEdgeByName(string name)
        {
            DataGraphEdge[] eList = new DataGraphEdge[edges.Count];
            edges.CopyTo(eList);
            for (int i = 0; i < edges.Count; i++)
            {
                if (eList[i].edgeName.Equals(name))
                    return eList[i];
            }
            return null;    // not found
        }

        public IEnumerator GetEnumerator()
        {
            for (int i = 0; i < edges.Count; i++)
            {
                yield return edges[i];
            }
        }
    }

    public class DataGraphEdge
    {
        public int sourceID { get; set; }
        public int destID { get; set; }
        public string edgeName { get; set; }
        public int edgeWeight { get; set; }

        public DataGraphEdge()
        {
            sourceID = -1;
            destID = -1;
            edgeName = null;
            edgeWeight = 0;
        }
    }
}
