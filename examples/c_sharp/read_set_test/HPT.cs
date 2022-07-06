using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace PerfTest
{
    static class HPT
    {
        #region constructor
        static HPT()
        {
        }
        #endregion

        public static double ToMillisesonds(long tics)
        {
            TimeSpan sp = TimeSpan.FromTicks(tics);
            return sp.TotalMilliseconds;
        }

        public static long GetTime
        {
            get
            {
                return Stopwatch.GetTimestamp();
            }
        }

        public static long GetFrequency
        {
            get
            {
                return Stopwatch.Frequency;
            }
        }
    }
}
