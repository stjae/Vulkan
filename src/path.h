#define PROJECT_DIR_OSX "/"
#define PROJECT_DIR_WINDOWS "C:/Users/stjae/dev/vulkan/"

#if defined(__APPLE__)
    #define PROJECT_DIR PROJECT_DIR_OSX
#else
    #define PROJECT_DIR PROJECT_DIR_WINDOWS
#endif
