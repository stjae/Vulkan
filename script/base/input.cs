namespace vkApp
{
    public static class Input
    {
        public static bool IsKeyDown(Keycode keycode)
        {
            return InternalCall.IsKeyDown(keycode);
        }
        public static bool IsMouseButtonDown(MouseButton mouseButton)
        {
            return InternalCall.IsMouseButtonDown(mouseButton);
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
