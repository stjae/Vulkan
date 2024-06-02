using System.Runtime.CompilerServices;

namespace vkApp
{
    public static class InternalCall
    {
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetTranslation(ulong meshInstanceID, out Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetTranslation(ulong meshInstanceID, ref Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetRotation(ulong meshInstanceID, out Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetRotation(ulong meshInstanceID, ref Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetForward(ulong meshInstanceID, out Vector3 forward);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetRight(ulong meshInstanceID, out Vector3 right);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetMatrix(ulong meshInstanceID, out Matrix4 matrix);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetCameraRotation(ulong meshInstanceID, out Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetCameraRotation(ulong meshInstanceID, ref Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyDown(Keycode keycode);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GetMouseX();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GetMouseY();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ApplyImpulse(ulong meshInstanceID, ref Vector3 impulse);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetVelocity(ulong meshInstanceID, ref Vector3 velocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetAngularVelocity(ulong meshInstanceID, ref Vector3 velocity);
    }
}
