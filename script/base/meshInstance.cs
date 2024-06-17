using vkApp;

namespace vkApp
{
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
                InternalCall.GetTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCall.SetTranslation(ID, ref value);
            }
        }
        public Vector3 Rotation
        {
            get
            {
                InternalCall.GetRotation(ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCall.SetRotation(ID, ref value);
            }
        }
        public Vector3 Forward
        {
            get
            {
                InternalCall.GetForward(ID, out Vector3 forward);
                return forward;
            }
        }
        public Vector3 Right
        {
            get
            {
                InternalCall.GetRight(ID, out Vector3 right);
                return right;
            }
        }
        public Matrix4 Matrix
        {
            get
            {
                InternalCall.GetMatrix(ID, out Matrix4 matrix);
                return matrix;
            }
        }
        public Vector3 CameraTranslation
        {
            get
            {
                InternalCall.GetCameraTranslation(ID, out Vector3 translation);
                return translation;
            }
            set
            {
                InternalCall.SetCameraTranslation(ID, ref value);
            }
        }
        public Vector3 CameraRotation
        {
            get
            {
                InternalCall.GetCameraRotation(ID, out Vector3 rotation);
                return rotation;
            }
            set
            {
                InternalCall.SetCameraRotation(ID, ref value);
            }
        }
        public Vector3 CameraDirection
        {
            get
            {
                InternalCall.GetCameraDirection(ID, out Vector3 direction);
                return direction;
            }
            set
            {
                InternalCall.SetCameraDirection(ID, ref value);
            }
        }
        public Vector3 Impulse
        {
            set
            {
                InternalCall.ApplyImpulse(ID, ref value);
            }
        }
        public Vector3 Velocity
        {
            set
            {
                InternalCall.SetVelocity(ID, ref value);
            }
        }
        public Vector3 AngularVelocity
        {
            set
            {
                InternalCall.SetAngularVelocity(ID, ref value);
            }
        }
    }
}
