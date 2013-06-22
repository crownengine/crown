package crown.android;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL20;


/**
*	This class provides a GL context used by 
*	Crown for rendering
*/
class CrownView extends GLSurfaceView 
{
    private static String TAG = "CrownView";

    private static final boolean DEBUG = false;

	/**
	*	Constructor
	*/
    public CrownView(Context context) 
	{
        super(context);
        init(false, 0, 0);
    }

	/**
	*	Constructor
	*/
    public CrownView(Context context, boolean translucent, int depth, int stencil) 
	{
        super(context);
        init(translucent, depth, stencil);
    }

	/**
	*	
	*/
    private void init(boolean translucent, int depth, int stencil) 
	{

        /* By default, GLSurfaceView() creates a RGB_565 opaque surface.
         * If we want a translucent one, we should change the surface's
         * format here, using PixelFormat.TRANSLUCENT for GL Surfaces
         * is interpreted as any 32-bit surface with alpha by SurfaceFlinger.
         */
        if (translucent) 
		{
            this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
        }

        /* Setup the context factory for 2.0 rendering.
         * See ContextFactory class definition below
         */
        setEGLContextFactory(new ContextFactory());

        /* We need to choose an EGLConfig that matches the format of
         * our surface exactly. This is going to be done in our
         * custom config chooser. See ConfigChooser class definition
         * below.
         */
        setEGLConfigChooser( translucent ?
                             new ConfigChooser(8, 8, 8, 8, depth, stencil) :
                             new ConfigChooser(5, 6, 5, 0, depth, stencil) );

        /* Set the renderer responsible for frame rendering */
        setRenderer(new Renderer());
    }

	/**
	* 	ContextFactory generate a EGL context that will be used by Crown
	*/
    private static class ContextFactory implements GLSurfaceView.EGLContextFactory 
	{
        private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

        public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) 
		{
            Log.w(TAG, "creating EGL context");

            checkEglError("Before eglCreateContext", egl);

			// Create OpenGL|ES 1.1 context
            int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 1, EGL10.EGL_NONE };
			// Create OpenGL|ES 2 context
//          int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };

            EGLContext context = egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);

            checkEglError("After eglCreateContext", egl);

            return context;
        }

        public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) 
		{
            egl.eglDestroyContext(display, context);
        }
    }

	/**
	*	check EGL fails
	*/
    private static void checkEglError(String prompt, EGL10 egl) 
	{
        int error;
        while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) 
		{
            Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
        }
    }

	/**
	*	ConfigChooser is used to set EGL configuration
	*/
    private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser 
	{

        public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) 
		{
            mRedSize = r;
            mGreenSize = g;
            mBlueSize = b;
            mAlphaSize = a;
            mDepthSize = depth;
            mStencilSize = stencil;
        }

		// Choose OpenGl|ES 2 bit
        private static int EGL_OPENGL_ES_BIT = 4;
        private static int[] s_configAttribs2 =
        {
            EGL10.EGL_RED_SIZE, 4,
            EGL10.EGL_GREEN_SIZE, 4,
            EGL10.EGL_BLUE_SIZE, 4,
            EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
            EGL10.EGL_NONE
        };

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) 
		{
            /* Get the number of minimally matching EGL configurations
             */
            int[] num_config = new int[1];
            egl.eglChooseConfig(display, s_configAttribs2, null, 0, num_config);

            int numConfigs = num_config[0];

            if (numConfigs <= 0) 
			{
                throw new IllegalArgumentException("No configs match configSpec");
            }

            /* Allocate then read the array of minimally matching EGL configs
             */
            EGLConfig[] configs = new EGLConfig[numConfigs];
            egl.eglChooseConfig(display, s_configAttribs2, configs, numConfigs, num_config);


            /* Now return the "best" one
             */
            return chooseConfig(egl, display, configs);
        }

        public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs) 
		{
            for(EGLConfig config : configs) 
			{
                int d = findConfigAttrib(egl, display, config,
                        EGL10.EGL_DEPTH_SIZE, 0);
                int s = findConfigAttrib(egl, display, config,
                        EGL10.EGL_STENCIL_SIZE, 0);

                // We need at least mDepthSize and mStencilSize bits
                if (d < mDepthSize || s < mStencilSize)
                    continue;

                // We want an *exact* match for red/green/blue/alpha
                int r = findConfigAttrib(egl, display, config,
                        EGL10.EGL_RED_SIZE, 0);
                int g = findConfigAttrib(egl, display, config,
                            EGL10.EGL_GREEN_SIZE, 0);
                int b = findConfigAttrib(egl, display, config,
                            EGL10.EGL_BLUE_SIZE, 0);
                int a = findConfigAttrib(egl, display, config,
                        EGL10.EGL_ALPHA_SIZE, 0);

                if (r == mRedSize && g == mGreenSize && b == mBlueSize && a == mAlphaSize)
				{
                    return config;	
				}
            }
            return null;
        }

        private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue) 
		{
            if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) 
			{
                return mValue[0];
            }
            return defaultValue;
        }

        // Subclasses can adjust these values:
        protected int mRedSize;
        protected int mGreenSize;
        protected int mBlueSize;
        protected int mAlphaSize;
        protected int mDepthSize;
        protected int mStencilSize;
        private int[] mValue = new int[1];
    }
	
	/**
	*	Renderer wraps Crown rendering functions
	*/
    private static class Renderer implements GLSurfaceView.Renderer
	{
        public void onDrawFrame(GL10 gl)
		{
            if (CrownLib.isRunning())
			{
                CrownLib.frame();
            }
        }

        public void onSurfaceChanged(GL10 gl, int width, int height)
		{
            if (!CrownLib.isInit())
            {
		        CrownLib.init();
            }

            CrownLib.setDisplaySize(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config)
		{

        }
    }
}
