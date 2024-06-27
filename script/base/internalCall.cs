using System.Runtime.CompilerServices;
using System.Numerics;

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
        internal extern static void GetMatrix(ulong meshInstanceID, out Matrix4x4 matrix);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetCameraTranslation(ulong meshInstanceID, out Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetCameraTranslation(ulong meshInstanceID, ref Vector3 translation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetCameraRotation(ulong meshInstanceID, out Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetCameraRotation(ulong meshInstanceID, ref Vector3 rotation);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetCameraDirection(ulong meshInstanceID, out Vector3 direction);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetCameraDirection(ulong meshInstanceID, ref Vector3 direction);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsCameraControllable(ulong meshInstanceID);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsKeyDown(Keycode keycode);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static bool IsMouseButtonDown(MouseButton mouseButton);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GetMouseX();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static float GetMouseY();
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetGravity(ulong meshInstanceID, ref Vector3 acceleration);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetLinearFactor(ulong meshInstanceID, ref Vector3 factor);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetAngularFactor(ulong meshInstanceID, ref Vector3 factor);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void ApplyImpulse(ulong meshInstanceID, ref Vector3 impulse);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetVelocity(ulong meshInstanceID, out Vector3 velocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetVelocity(ulong meshInstanceID, ref Vector3 velocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetAngularVelocity(ulong meshInstanceID, ref Vector3 velocity);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void SetRigidBodyTransform(ulong meshInstanceID, ref Matrix4x4 transform);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void GetRayHitPosition(ref Vector3 rayFrom, ref Vector3 rayTo, out Vector3 hitPosition);
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
        internal extern static void DuplicateMeshInstance(ulong meshInstanceID);
    }
}
