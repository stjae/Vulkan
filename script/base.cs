using System;
using System.Runtime.CompilerServices;

namespace vkApp
{
    public struct Vector3
    {
        public float X, Y, Z;

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }

    public static class InternalCalls
    {
    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    internal extern static void CppLog_string(string str);

    [MethodImplAttribute(MethodImplOptions.InternalCall)]
    internal extern static void CppLog_vec3(ref Vector3 inVec, out Vector3 outVec);
    }

    public class Base
    {
        protected Base()
        {
            Log("Base constructor called\n");
            Vector3 v = new Vector3(1, 2, 3);
            Vector3 result = Log(v);
            Console.WriteLine($"{ result.X},{ result.Y},{ result.Z}");
            //Console.WriteLine($"{ result.X},{ result.Y},{ result.Z}");
        }

        protected static void Print()
        {
            Console.WriteLine("Hello C#");
        }

        protected void Log(string msg)
        {
            InternalCalls.CppLog_string(msg);
        }
        protected Vector3 Log(Vector3 v)
        {
            InternalCalls.CppLog_vec3(ref v, out Vector3 result);
            return result;
        }

    }
}
