#define PROJECT_DIR_OSX "@PROJECT_DIR_OSX@/"
#define PROJECT_DIR_WINDOWS "@PROJECT_DIR_WINDOWS@/"

#if defined(__APPLE__)
    #define PROJECT_DIR PROJECT_DIR_OSX
#else
    #define PROJECT_DIR PROJECT_DIR_WINDOWS
#endif
