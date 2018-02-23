cmake_minimum_required(VERSION 3.4.1)

add_library( native

             SHARED

             cpp/guard.cpp
             cpp/jni.cpp
             cpp/log.cpp
             cpp/MainActivity.cpp )


# For LogCat
find_library( log
              log )

# For CPU rendering
find_library( android
              android )

# For OpenGL
find_library( EGL
              EGL )
find_library( GLESv2
              GLESv2 )


target_link_libraries( native
                       ${log} ${android} ${EGL} ${GLESv2} )
