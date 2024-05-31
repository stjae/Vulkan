using System;
using System.Runtime.CompilerServices;
using vkApp;

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
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetTranslation(ulong meshInstanceID, out Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetTranslation(ulong meshInstanceID, ref Vector3 translation);
    }

    public class MeshInstance
    {
        protected MeshInstance()
        {
            ID = 0;
        }
        internal MeshInstance(ulong id)
        {
            ID = id;
        }
        public readonly ulong ID;

        public Vector3 Translation
        {
            get
            {
                InternalCalls.GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCalls.SetTranslation(ID, ref value);
            }
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
