#ifndef TIME_H
#define TIME_H

#include <glfw/glfw3.h>

class Time
{
    friend class Application;

    inline static int s_frameCount;
    inline static float s_dt, s_framerate;
    inline static double s_currentTime, s_lastTime;
    static void Update()
    {
        static float timePassed;
        static int count;

        s_currentTime = glfwGetTime();
        s_dt = (float)(s_currentTime - s_lastTime);
        s_lastTime = s_currentTime;

        timePassed += s_dt;

        if (timePassed > 1.0) {
            s_frameCount = count;
            s_framerate = 1000.0f / (float)s_frameCount;

            timePassed = 0;
            count = 0;
        }

        count++;
    }

public:
    static float GetDeltaTime() { return s_dt; }
    static int GetFrameCount() { return s_frameCount; }
    static float GetFramerate() { return s_framerate; }
};

#endif
