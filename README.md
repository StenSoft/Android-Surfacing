Android Surfacing example
=========================

This example shows how to initialize CPU rendering (`ANativeWindow_lock`) or
OpenGL with `SurfaceTexture` from `TextureView` and how to properly
deinitialize it so that you can switch from OpenGL to CPU rendering or vice
versa on the same `SurfaceTexture`.
