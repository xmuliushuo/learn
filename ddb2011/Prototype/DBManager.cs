using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using MySql.Data.MySqlClient;
using System.Configuration;

namespace DDB2011Prototype
{
    /// <summary>
    /// 此类用于处理数据库的相关操作
    /// </summary>
    public class DBManager
    {
        MySqlConnection myConnection;
        /// <summary>
        /// 数据库操作接口
        /// </summary>
        public DBManager()
        {
            string mysqlConnectionString =
                "server=" + ConfigurationManager.AppSettings["host"] +
                ";uid=" + ConfigurationManager.AppSettings["user"] +
                ";pwd=" + ConfigurationManager.AppSettings["passwd"] +
                ";database=" + ConfigurationManager.AppSettings["database"];
            myConnection = new MySqlConnection(mysqlConnectionString);
        }

        /// <summary>
        /// 获得数据库中所有节点的数目
        /// </summary>
        /// <returns>节点数目</returns>
        public int getNodeNum()
        {
            int count = 0;// 节点数统计
            try
            {
                myConnection.Open();
                string setMySqlMode = "set sql_mode='ANSI_QUOTES'";
                using (MySqlCommand setCommand = new MySqlCommand(setMySqlMode, myConnection))
                {
                    setCommand.ExecuteNonQuery();
                }
                string sql1 = "select count(*) from paper";
                string sql2 = "select count(*) from author";
                string sql3 = "select count(*) from citation";
                string sql4 = "select count(*) from \"paper-author\"";
                MySqlDataReader dr;
                using (MySqlCommand myCommand1 = new MySqlCommand(sql1, myConnection))
                {
                    dr = myCommand1.ExecuteReader();
                    while (dr.Read())
                    {
                        count += dr.GetInt32(0);
                    }
                    dr.Close();
                }

                using (MySqlCommand myCommand2 = new MySqlCommand(sql2, myConnection))
                {
                    dr = myCommand2.ExecuteReader();
                    while (dr.Read())
                    {
                        count += dr.GetInt32(0);
                    }
                    dr.Close();
                }

                using (MySqlCommand myCommand3 = new MySqlCommand(sql3, myConnection))
                {
                    dr = myCommand3.ExecuteReader();
                    while (dr.Read())
                    {
                        count += dr.GetInt32(0);
                    }
                    dr.Close();
                }

                using (MySqlCommand myCommand4 = new MySqlCommand(sql4, myConnection))
                {
                    dr = myCommand4.ExecuteReader();
                    while (dr.Read())
                    {
                        count += dr.GetInt32(0);
                    }
                    dr.Close();
                }
                myConnection.Close();
            }
            catch (MySqlException e)
            {
                Console.WriteLine(e.Message);
                App.Current.Shutdown();
            }
            return count;
        }

        /// <summary>
        /// 初始化数据图
        /// </summary>
        /// <param name="gm"></param>
        public void initGraph(GraphManager gm)
        {
            try
            {
                myConnection.Open();
            }
            catch (MySqlException e)
            {
                Console.WriteLine(e.Message);
                App.Current.Shutdown();
            }

            MySqlDataReader dr;
            int index = 0;

            string sql = "select * from paper";
            using (MySqlCommand paperCommand = new MySqlCommand(sql, myConnection))
            {
                dr = paperCommand.ExecuteReader();
                while (dr.Read())
                {
                    gm.dicKey[dr.GetString("PID")] = index;
                    gm.arrayKey.SetValue(dr.GetString("PID"), index);
                    index++;
                }
                dr.Close();
            }

            sql = "select * from author";
            using (MySqlCommand authorCommand = new MySqlCommand(sql, myConnection))
            {
                dr = authorCommand.ExecuteReader();
                while (dr.Read())
                {
                    gm.dicKey[dr.GetString("AID")] = index;
                    gm.arrayKey.SetValue(dr.GetString("AID"), index);
                    index++;
                }
                dr.Close();
            }

            sql = "select * from \"paper-author\"";
            using (MySqlCommand paper_authorCommand = new MySqlCommand(sql, myConnection))
            {
                dr = paper_authorCommand.ExecuteReader();
                int paperIndex;
                int authorIndex;
                int count = 0;
                string key = "";
                while (dr.Read())
                {
                    key = string.Format("W{0}", count);
                    count++;
                    gm.arrayKey.SetValue(key, index);
                    paperIndex = gm.dicKey[dr.GetString("PID")];
                    authorIndex = gm.dicKey[dr.GetString("AID")];
                    gm.graph[paperIndex, index] = 1;
                    gm.graph[index, paperIndex] = 1;
                    gm.graph[authorIndex, index] = 1;
                    gm.graph[index, authorIndex] = 1;
                    index++;
                }
                dr.Close();
            }
            sql = "select * from citation";
            using (MySqlCommand citationCommand = new MySqlCommand(sql, myConnection))
            {
                dr = citationCommand.ExecuteReader();
                int citeIndex;
                int citedIndex;
                int count = 0;
                string key = "";
                while (dr.Read())
                {
                    key = string.Format("C{0}", count);
                    count++;
                    gm.arrayKey.SetValue(key, index);
                    citeIndex = gm.dicKey[dr.GetString("Cite")];
                    citedIndex = gm.dicKey[dr.GetString("Cited")];
                    gm.graph[citeIndex, index] = 1;
                    gm.graph[index, citeIndex] = 1;
                    gm.graph[citedIndex, index] = 1;
                    gm.graph[index, citedIndex] = 1;
                    index++;
                }
                dr.Close();
            }

            myConnection.Close();
        }

        /// <summary>
        /// 根据关键词获得节点集合
        /// </summary>
        /// <param name="keyword">关键词数组</param>
        /// <param name="gm">GraphManager数据图</param>
        /// <param name="keywordSet">？？？</param>
        /// <returns></returns>
        public List<int> getKeywordSets(string[] keyword, GraphManager gm, List<List<int>> keywordSet)
        {
            string sql = "";
            List<int> keywordIndexList = new List<int>();
            string id = "";
            int index = 0;
            #region 获取关键字节点集合
            try
            {
                myConnection.Open();
                MySqlDataReader dr;
                foreach (string item in keyword)
                {
                    List<int> nodeList = new List<int>();
                    sql = "select PID from paper where Title like '%" + item + "%'";
                    using (MySqlCommand commandPaper = new MySqlCommand(sql, myConnection))
                    {
                        dr = commandPaper.ExecuteReader();
                        while (dr.Read())
                        {
                            id = dr.GetString("PID");
                            index = gm.dicKey[id];
                            nodeList.Add(index);
                            if (!keywordIndexList.Contains(index))
                            {
                                keywordIndexList.Add(index);
                            }
                        }
                        dr.Close();
                    }
                    sql = "select AID from author where Name like '%" + item + "%'";
                    using (MySqlCommand commandAuthor = new MySqlCommand(sql, myConnection))
                    {
                        dr = commandAuthor.ExecuteReader();
                        while (dr.Read())
                        {
                            id = dr.GetString("AID");

                            index = gm.dicKey[id];
                            nodeList.Add(index);
                            if (!keywordIndexList.Contains(index))
                            {
                                keywordIndexList.Add(index);
                            }
                        }
                        dr.Close();
                    }
                    keywordSet.Add(nodeList);
                }
                myConnection.Close();
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
            }
            #endregion
            //foreach (int i in keywordIndexList)
            //{
            //    Console.WriteLine(i);
            //}
            return keywordIndexList;
        }
    }
}
