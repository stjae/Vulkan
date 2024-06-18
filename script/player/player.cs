using System;
using vkApp;
using System.Numerics;

public class Player : MeshInstance
{
    private static float mouseX;
    private static float mouseY;
    private static float prevMouseX;
    private static float prevMouseY;
    void OnCreate()
    {
        Console.WriteLine($"Player.OnCreate: {ID}");
        prevMouseX = 0;
        prevMouseY = 0;
    }
    void OnUpdate(float dt)
    {
        mouseX = Input.GetMouseX();
        mouseY = Input.GetMouseY();
        float mouseDx = mouseX - prevMouseX;
        float mouseDy = mouseY - prevMouseY;

        float speed = 10.0f;
        Vector3 velocity = new Vector3();

        if (Input.IsKeyDown(Keycode.KEY_W))
            velocity += Forward * speed;
        if (Input.IsKeyDown(Keycode.KEY_S))
            velocity -= Forward * speed;
        if (Input.IsKeyDown(Keycode.KEY_A))
            velocity += Right * speed;
        if (Input.IsKeyDown(Keycode.KEY_D))
            velocity -= Right * speed;

        Velocity = velocity;

        Vector3 angularVelocty = new Vector3(0.0f, -mouseDx * 50.0f, 0.0f);
        AngularVelocity = angularVelocty;

        Vector3 cameraRotation = CameraRotation;
        cameraRotation.X -= mouseDy * 50.0f;
        CameraRotation = cameraRotation;

        CameraTranslation = Translation + new Vector3(0.0f, 2.0f, 0.0f);
        CameraDirection = Forward;

        prevMouseX = mouseX;
        prevMouseY = mouseY;
    }
    void OnDestroy()
    {
        Console.WriteLine($"Player.OnDestroy: {ID}");
    }
}
