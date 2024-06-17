using System;

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
        public static Vector3 operator +(Vector3 v0, Vector3 v1)
        {
            return new Vector3(v0.X + v1.X, v0.Y + v1.Y, v0.Z + v1.Z);
        }
        public static Vector3 operator -(Vector3 v0, Vector3 v1)
        {
            return new Vector3(v0.X - v1.X, v0.Y - v1.Y, v0.Z - v1.Z);
        }
        public static Vector3 operator +(Vector3 v, float f)
        {
            return new Vector3(v.X + f, v.Y + f, v.Z + f);
        }
        public static Vector3 operator -(Vector3 v, float f)
        {
            return new Vector3(v.X - f, v.Y - f, v.Z - f);
        }
        public static Vector3 operator *(Vector3 v, float f)
        {
            return new Vector3(v.X * f, v.Y * f, v.Z * f);
        }
        public void Print()
        {
            Console.WriteLine($"Vector3({X}, {Y}, {Z})");
        }
    }
    public struct Vector4
    {
        public float X, Y, Z, W;

        public Vector4(float x, float y, float z, float w)
        {
            X = x;
            Y = y;
            Z = z;
            W = w;
        }
        public static Vector4 operator +(Vector4 v0, Vector4 v1)
        {
            return new Vector4(v0.X + v1.X, v0.Y + v1.Y, v0.Z + v1.Z, v0.W + v1.W);
        }
        public static Vector4 operator +(Vector4 v, float f)
        {
            return new Vector4(v.X + f, v.Y + f, v.Z + f, v.W + f);
        }
        public static Vector4 operator -(Vector4 v, float f)
        {
            return new Vector4(v.X - f, v.Y - f, v.Z - f, v.W - f);
        }
        public static Vector4 operator *(Matrix4 m, Vector4 v)
        {
            float v0 = m.X0 * v.X + m.Y0 * v.X + m.Z0 * v.X + m.W0 * v.X;
            float v1 = m.X1 * v.Y + m.Y1 * v.Y + m.Z1 * v.Y + m.W1 * v.Y;
            float v2 = m.X2 * v.Z + m.Y2 * v.Z + m.Z2 * v.Z + m.W2 * v.Z;
            float v3 = m.X3 * v.W + m.Y3 * v.W + m.Z3 * v.W + m.W3 * v.W;

            return new Vector4(v0, v1, v2, v3);
        }
        public void Print()
        {
            Console.WriteLine($"Vector4({X}, {Y}, {Z}, {W})");
        }
    }
    public struct Matrix4
    {
        public float
            X0, Y0, Z0, W0,
            X1, Y1, Z1, W1,
            X2, Y2, Z2, W2,
            X3, Y3, Z3, W3;

        public Matrix4
            (float x0, float y0, float z0, float w0,
            float x1, float y1, float z1, float w1,
            float x2, float y2, float z2, float w2,
            float x3, float y3, float z3, float w3)
        {
            X0 = x0;
            Y0 = y0;
            Z0 = z0;
            W0 = w0;

            X1 = x1;
            Y1 = y1;
            Z1 = z1;
            W1 = w1;

            X2 = x2;
            Y2 = y2;
            Z2 = z2;
            W2 = w2;

            X3 = x3;
            Y3 = y3;
            Z3 = z3;
            W3 = w3;
        }

        public void Print()
        {
            Console.Write("Matrix4(" +
                $"({X0}, {Y0}, {Z0}, {W0})," +
                $"({X1}, {Y1}, {Z1}, {W1})," +
                $"({X2}, {Y2}, {Z2}, {W2})," +
                $"({X3}, {Y3}, {Z3}, {W3}))");
        }
    }
}
