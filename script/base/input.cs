namespace vkApp
{
    public static class Input
    {
        public static bool IsKeyDown(Keycode keycode)
        {
            return InternalCall.IsKeyDown(keycode);
        }
        public static float GetMouseX()
        {
            return InternalCall.GetMouseX();
        }
        public static float GetMouseY()
        {
            return InternalCall.GetMouseY();
        }
    }
}
