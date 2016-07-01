using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace searchTest
{
    class ScorePosFinder
    {

        static List<int> hej = new List<int>();
        static void Main(string[] args)
        {
            hej.Add(1500);
            hej.Add(1401);
            hej.Add(1302);
            hej.Add(1203);
            hej.Add(1104);
            hej.Add(1005);
            hej.Add(906);
            hej.Add(807);
            hej.Add(708);
            hej.Add(609);
            hej.Add(510);
            hej.Add(411);
            hej.Add(312);
            hej.Add(213);
            hej.Add(114);
            hej.Add(15);
            int val = 0;

            int result = 0;
            /*while(val != -1)
            {
                val = Convert.ToInt32(Console.ReadLine());
                Console.WriteLine("input:" + val);
                result = index(val);
                Console.WriteLine("result:" + result + "\n");
            }*/
            int i = 0;
            while (i < hej.Count)
            {
                val = hej[i];
                i++;
                Console.WriteLine("input:" + val);
                Console.WriteLine("result:" + result + "\n");
            }

            Console.Read();
        }

        {
            int searchIndex = hej.Count / 2;
            int minIndex = 0;
            int maxIndex = hej.Count - 1;
            int arrScore;
            int prevIndex = -1;

            while(prevIndex != searchIndex)
            {
                Console.WriteLine("searchingIndex:" + searchIndex);
                prevIndex = searchIndex;
                arrScore = hej[searchIndex];

                if (score == arrScore)
                {
                    return searchIndex;
                }
                else if(score < arrScore)
                {
                    minIndex = searchIndex + 1;
                    searchIndex = searchIndex + (maxIndex - searchIndex) / 2 + 1;
                }
                else
                {
                    maxIndex = searchIndex - 1;
                    searchIndex = searchIndex - (searchIndex - minIndex) / 2 - 1;
                }
            }
            return -1;
        }
    }
}
