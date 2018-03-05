Android Surfacing example
=========================

This example shows how to initialize CPU rendering (`ANativeWindow_lock`) or
OpenGL (`eglCreateWindowSurface`) with `SurfaceTexture` from `TextureView`
and how to properly deinitialize it so that you can switch from OpenGL to
CPU rendering and vice versa on the same `SurfaceTexture` and in the same
`TextureView`.

## Rationale

If deinitialization is not properly done, you can spend several days or even
weeks debugging cryptic errors in Logcat, such as:

```
connect: already connected (cur=2, req=1)
```

Unfortunately, there is very little documentation, no examples and you may
get the impression that switching from CPU rendering to OpenGL is for some
reason not supported (eventhough it is). To spare you from the headache of
going through Buffer Queue sources, I prepared this simple and
easy-to-follow example of how to do it properly on any Android that
supports `TextureView` and OpenGL ES2.

## Threading

In the example here, all drawing is done on the main (UI) thread. This is
fine for the simple one-color drawing we do here but you may want to move
the drawing to another thread. Synchronization on surface events
(`onSurface…`) will than be necessary. Generally, you don't need to create
OpenGL context on the same thread that will use it but I strongly recommend
keeping one context on the same thread. But if you have multiple
`TextureView`s, feel free to use one thread for each.

