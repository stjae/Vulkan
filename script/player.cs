using System;
using vkApp;

public class Player : Base
{
    Player()
    {
        Log("Player Constructed\n");
    }

    void OnCreate()
    {
        Console.WriteLine("Player.OnCreate renamed");
    }
    void OnUpdate(float dt)
    {
        Console.WriteLine($"Player.OnUpdate: {dt}");
    }
}
