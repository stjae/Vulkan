using System;
using System.Windows.Input;
using vkApp;

public class Player : MeshInstance
{
    void OnCreate()
    {
        Console.WriteLine($"Player.OnCreate: {ID}");
    }
    void OnUpdate(float dt)
    {
        Console.WriteLine($"Player.OnUpdate: {dt}");
    }
}
